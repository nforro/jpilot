/*
 * datebook.c
 * Copyright (C) 1999 by Judd Montgomery
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <stdio.h>
#include <pi-source.h>
#include <pi-socket.h>
#include <pi-datebook.h>
#include <pi-dlp.h>
#include <pi-file.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utime.h>

#include "datebook.h"
#include "utils.h"

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define DATEBOOK_EOF 7

int datebook_sort(AppointmentList **al)
{
   AppointmentList *temp_al, *prev_al, *next;
   int found_one;
   
   //printf("datebook_sort()\n");
   found_one=1;
   while (found_one) {
      found_one=0;
      for (prev_al=NULL, temp_al=*al; temp_al;
	   prev_al=temp_al, temp_al=temp_al->next) {
	 //printf("begin %d\n", temp_al->ma.a.begin.tm_hour);
	 if (temp_al->next) {
	    if (temp_al->ma.a.begin.tm_hour > temp_al->next->ma.a.begin.tm_hour) {
	       found_one=1;
	       next=temp_al->next;
	       if (prev_al) {
		  prev_al->next = next;
	       }
	       temp_al->next=next->next;
	       next->next = temp_al;
	       if (temp_al==*al) {
		  *al=next;
	       }
	       temp_al=next;
	    }
	 }
      }
   }
}

static int pc_datebook_read_next_rec(FILE *in, MyAppointment *ma)
{
   PCRecordHeader header;
   int rec_len, num;
   char *record;
   //DatebookRecType rt;
   
   if (feof(in)) {
      return DATEBOOK_EOF;
   }
//  if (ftell(in)==0) {
//     printf("Error: File header not read\n");
//      return DATEBOOK_EOF;
//   }
   num = fread(&header, sizeof(header), 1, in);
   if (feof(in)) {
      return DATEBOOK_EOF;
   }
   if (num != 1) {
      printf("error on fread\n");
      return DATEBOOK_EOF;
   }
   rec_len = header.rec_len;
   ma->rt = header.rt;
   ma->attrib = header.attrib;
   //printf("read attrib = %d\n", ma->attrib);
   ma->unique_id = header.unique_id;
   record = malloc(rec_len);
   fread(record, rec_len, 1, in);
   if (feof(in)) {
      free(record);
      return DATEBOOK_EOF;
   }
   unpack_Appointment(&(ma->a), record, rec_len);
   free(record);
   return 0;
}

int does_pc_appt_exist(int unique_id, PCRecType rt)
{
   FILE *pc_in;
   PCRecordHeader header;
   int num;
   
   //printf("looking for unique id = %d\n",unique_id);
   //printf("looking for rt = %d\n",rt);
   pc_in=open_file("DatebookDB.pc", "r");
   if (pc_in==NULL) {
      printf("Couldn't open PC records file\n");
      return 0;
   }
   while(!feof(pc_in)) {
      num = fread(&header, sizeof(header), 1, pc_in);
      if (feof(pc_in) || (!num)) {
	 break;
      }
      //printf("read unique id = %d\n",header.unique_id);
      //printf("read rt = %d\n",header.rt);
      if ((header.unique_id==unique_id)
	   && (header.rt==rt)) {
	 fclose(pc_in);
	 //printf("TRUE\n");
	 return 1;
      }
      if (fseek(pc_in, header.rec_len, SEEK_CUR)) {
	 printf("fseek failed\n");
      }
   }
   fclose(pc_in);
   return 0;
}

int pc_datebook_write(struct Appointment *a, PCRecType rt, unsigned char attrib)
{
   PCRecordHeader header;
   //PCFileHeader   file_header;
   FILE *out;
   char record[65536];
   int rec_len;
   unsigned int next_unique_id;

   get_next_unique_pc_id(&next_unique_id);
#ifdef JPILOT_DEBUG
   printf("next unique id = %d\n",next_unique_id);
#endif
   
   out = open_file("DatebookDB.pc", "a");
   if (!out) {
      printf("Error opening DatebookDB.pc\n");
      return -1;
   }
   rec_len = pack_Appointment(a, record, 65535);
   if (!rec_len) {
      PRINT_FILE_LINE;
      printf("pack_Appointment error\n");
      return -1;
   }
   header.rec_len=rec_len;
   header.rt=rt;
   header.attrib=attrib;
   header.unique_id=next_unique_id;
   fwrite(&header, sizeof(header), 1, out);
   fwrite(record, rec_len, 1, out);
   fflush(out);
   fclose(out);
}

void free_AppointmentList(AppointmentList **al)
{
   AppointmentList *temp_al, *temp_al_next;
   
   for (temp_al = *al; temp_al; temp_al=temp_al_next) {
      free_Appointment(&(temp_al->ma.a));
      temp_al_next = temp_al->next;
      free(temp_al);
   }
   *al = NULL;
}

int dateToDays(struct tm *tm1)
{
   time_t t1;

   t1 = mktime(tm1);
   return t1/86400;//There are 86400 secs in a day
}

//returns 0 if times equal
//returns 1 if time1 is greater (later)
//returns 2 if time2 is greater (later)
int compareTimesToSec(struct tm *tm1, struct tm *tm2)
{
   time_t t1, t2;

   t1 =  mktime(tm1);
   t2 =  mktime(tm2);
   if (t1 > t2 ) return 1;
   if (t1 < t2 ) return 2;
   return 0;
}

//returns 0 if times equal
//returns 1 if time1 is greater (later)
//returns 2 if time2 is greater (later)
int compareTimesToDay(struct tm *tm1, struct tm *tm2)
{
   unsigned int t1, t2;
   
   t1 = tm1->tm_year*366+tm1->tm_yday;
   t2 = tm2->tm_year*366+tm2->tm_yday;
   if (t1 > t2 ) return 1;
   if (t1 < t2 ) return 2;
   return 0;
}

unsigned int isApptOnDate(struct Appointment *a, struct tm *date)
{
   unsigned int ret;
   unsigned int r;
   int begin_days, days, week1, week2;
   int dow, ndim;
   int i;
   //days_in_month is adjusted for leap year with the date
   //structure
   int days_in_month[]={31,28,31,30,31,30,31,31,30,31,30,31
   };
   //time_t ltime;
   //struct tm *now;

   //time( &ltime );
   //now = localtime( &ltime );

   ret = FALSE;

   //Leap year
   if (date->tm_year%4==0) {
      days_in_month[1]++;
   }
   
   //See if the appointment starts after date
   r = compareTimesToDay(&(a->begin), date);
   if (r == 1) {
      return FALSE;
   }
   if (r == 0) {
      ret = TRUE;
   }
   //If the appointment has an end date, see that we are not past it
   if (!(a->repeatForever)) {
      r = compareTimesToDay(&(a->repeatEnd), date);
      if (r == 2) {
	 return FALSE;
      }
   }

   switch (a->repeatType) {
    case repeatNone:
      break;
    case repeatDaily:
      //See if this appt repeats on this day
      begin_days = dateToDays(&(a->begin));
      days = dateToDays(date);
      //g_print("days=%d begin_days=%d\n",days, begin_days);
      ret = (((days - begin_days)%(a->repeatFrequency))==0);
      break;
    case repeatWeekly:
      get_month_info(date->tm_mon, date->tm_mday, date->tm_year, &dow, &ndim);
      //See if the appointment repeats on this day
      if (!(a->repeatDays[dow])) {
	 ret = FALSE;
	 break;
      }
      //See if we are in a week that is repeated in
      begin_days = dateToDays(&(a->begin));
      days = dateToDays(date);
      ret = (((days - begin_days)/7)%(a->repeatFrequency)==0);
      break;
    case repeatMonthlyByDay:
      //See if we are in a month that is repeated in
      ret = (((date->tm_year - a->begin.tm_year)*12 +
       (date->tm_mon - a->begin.tm_mon))%(a->repeatFrequency)==0);
      if (!ret) {
	 break;
      }
      //If the days of the week match - good
      //e.g. Monday or Thur, etc.
      if (a->repeatDay%7 != date->tm_wday) {
	 ret = FALSE;
	 break;
      }
      //Are they both in the same week in the month
      //e.g. The 3rd Mon, or the 2nd Fri, etc.
      week1 = a->repeatDay/7;
      week2 = (date->tm_mday - 1)/7;
      if (week1 != week2) {
	 ret = FALSE;
      }
      //See if the appointment repeats on the last week of the month
      //and this is the 4th, and last.
      if (week1 > 3) {
	 if ((date->tm_mday + 7) > days_in_month[date->tm_mon]) {
	    ret = TRUE;
	 }
      }
      break;
    case repeatMonthlyByDate:
      //See if we are in a repeating month
      ret = (((date->tm_year - a->begin.tm_year)*12 +
       (date->tm_mon - a->begin.tm_mon))%(a->repeatFrequency) == 0);
      if (!ret) {
	 break;
      }
      //See if this is the date that the appt repeats on
      if (date->tm_mday == a->begin.tm_mday) {
	 ret = TRUE;
	 break;
      }
      //If appt occurs after the last day of the month and this date
      //is the last day of the month then it occurs today
      ret = ((a->begin.tm_mday > days_in_month[date->tm_mon]) &&
	     (date->tm_mday == days_in_month[date->tm_mon]));
      break;
    case repeatYearly:
      if ((date->tm_year - a->begin.tm_year)%(a->repeatFrequency) != 0) {
	 ret = FALSE;
	 break;
      }
      if ((date->tm_mday == a->begin.tm_mday) &&
	  (date->tm_mon == a->begin.tm_mon)) {
	 ret = TRUE;
	 break;
      }
      //Take care of Feb 29th (Leap Day)
      if ((a->begin.tm_mon == 1) && (a->begin.tm_mday == 29) &&
	(date->tm_mon == 1) && (date->tm_mday == 28)) {
	 ret = TRUE;
	 break;
      }   
      break;
    default:
      g_print("unknown repeatType (%d) found in DatebookDB\n",
	      a->repeatType);
      printf("repeatMonthlyByDate = %d\n", repeatMonthlyByDate);
      ret = FALSE;
   }//switch

   if (ret) {
      //Check for exceptions
      for (i=0; i<a->exceptions; i++) {
	 //printf("exception %d mon %d\n", i, a->exception[i].tm_mon);
	 //printf("exception %d day %d\n", i, a->exception[i].tm_mday);
	 //printf("exception %d year %d\n", i, a->exception[i].tm_year);
	 //printf("exception %d yday %d\n", i, a->exception[i].tm_yday);
	 //printf("today is yday %d\n", date->tm_yday);
	 begin_days = dateToDays(&(a->exception[i]));
	 days = dateToDays(date);
	 //printf("%d == %d\n", begin_days, days);
	 if (begin_days == days) {
	    ret = FALSE;
	 }
      }
   }
   
   return ret;
}

int get_days_appointments(AppointmentList **appointment_list, struct tm *now)
{
   FILE *in, *pc_in;
//   *appointment_list=NULL;
//   char db_name[34];
//   char filler[100];
   char *buf;
//   unsigned char char_num_records[4];
//   unsigned char char_ai_offset[4];//app info offset
   int num_records, i, num, r;
   unsigned int offset, next_offset, rec_size;
//   unsigned char c;
   long fpos;  //file position indicator
   unsigned char attrib;
   unsigned int unique_id;
   mem_rec_header *mem_rh, *temp_mem_rh;
   record_header rh;
   RawDBHeader rdbh;
   DBHeader dbh;
   struct Appointment a;
   //struct AppointmentAppInfo ai;
   AppointmentList *temp_appointment_list;
   MyAppointment ma;

   mem_rh = NULL;

   in = open_file("DatebookDB.pdb", "r");
   if (!in) {
      printf("Error opening DatebookDB.pdb\n");
      return -1;
   }
   //Read the database header
   fread(&rdbh, sizeof(RawDBHeader), 1, in);
   if (feof(in)) {
      printf("Error reading DatebookDB.pdb\n");
      fclose(in);
      return -1;
   }
   raw_header_to_header(&rdbh, &dbh);
   
   //printf("db_name = %s\n", dbh.db_name);
   //printf("num records = %d\n", dbh.number_of_records);
   //printf("app info offset = %d\n", dbh.app_info_offset);

   //fread(filler, 2, 1, in);

   //Read each record entry header
   num_records = dbh.number_of_records;
   //printf("sizeof(record_header)=%d\n",sizeof(record_header));
   for (i=1; i<num_records+1; i++) {
      fread(&rh, sizeof(record_header), 1, in);
      offset = ((rh.Offset[0]*256+rh.Offset[1])*256+rh.Offset[2])*256+rh.Offset[3];
      //printf("record header %u offset = %u\n",i, offset);
      //printf("       attrib 0x%x\n",rh.attrib);
      //printf("    unique_ID %d %d %d = ",rh.unique_ID[0],rh.unique_ID[1],rh.unique_ID[2]);
      //printf("%d\n",(rh.unique_ID[0]*256+rh.unique_ID[1])*256+rh.unique_ID[2]);
      temp_mem_rh = (mem_rec_header *)malloc(sizeof(mem_rec_header));
      temp_mem_rh->next = mem_rh;
      mem_rh = temp_mem_rh;
      mem_rh->rec_num = i;
      mem_rh->offset = offset;
      mem_rh->attrib = rh.attrib;
      mem_rh->unique_id = (rh.unique_ID[0]*256+rh.unique_ID[1])*256+rh.unique_ID[2];
   }

   find_next_offset(mem_rh, 0, &next_offset, &attrib, &unique_id);
   fseek(in, next_offset, SEEK_SET);
   
   while(!feof(in)) {
      fpos = ftell(in);
      find_next_offset(mem_rh, fpos, &next_offset, &attrib, &unique_id);
      //next_offset += 223;
      rec_size = next_offset - fpos;
      //printf("rec_size = %u\n",rec_size);
      //printf("fpos,next_offset = %u %u\n",fpos,next_offset);
      //printf("----------\n");
      if (feof(in)) break;
      buf = malloc(rec_size);
      if (!buf) {
	 break;
      }
      num = fread(buf, 1, rec_size, in);
      if (feof(in) || (!num)) break;

      unpack_Appointment(&a, buf, rec_size);
      free(buf);
      if (isApptOnDate(&a, now)
	  && (!does_pc_appt_exist(unique_id, PALM_REC))) {
	 temp_appointment_list = malloc(sizeof(AppointmentList));
	 memcpy(&(temp_appointment_list->ma.a), &a, sizeof(struct Appointment));
	 //temp_appointment_list->ma.a = temp_a;
	 temp_appointment_list->ma.rt = PALM_REC;
	 temp_appointment_list->ma.attrib = attrib;
	 temp_appointment_list->ma.unique_id = unique_id;
	 temp_appointment_list->next = *appointment_list;
	 *appointment_list = temp_appointment_list;
      }
   }
   fclose(in);
   free_mem_rec_header(&mem_rh);

   //
   //Get the appointments out of the PC database
   //
   pc_in = open_file("DatebookDB.pc", "r");
   if (pc_in==NULL) {
      return 0;
   }
   //r = pc_datebook_read_file_header(pc_in);
   while(!feof(pc_in)) {
      r = pc_datebook_read_next_rec(pc_in, &ma);
      if (r==DATEBOOK_EOF) break;
      if ((isApptOnDate(&(ma.a), now))
	  &&(ma.rt!=DELETED_PC_REC)
	  &&(ma.rt!=DELETED_PALM_REC)
	  &&(ma.rt!=DELETED_DELETED_PALM_REC)) {
	 temp_appointment_list = malloc(sizeof(AppointmentList));
	 memcpy(&(temp_appointment_list->ma), &ma, sizeof(MyAppointment));
	 temp_appointment_list->next = *appointment_list;
	 *appointment_list = temp_appointment_list;

	 //temp_appointment_list->ma.attrib=0;
      } else {
	 //this doesnt really free it, just the string pointers
	 free_Appointment(&(ma.a));
      }
      if (ma.rt==DELETED_PALM_REC) {
	 for (temp_appointment_list = *appointment_list; temp_appointment_list;
	      temp_appointment_list=temp_appointment_list->next) {
	    if (temp_appointment_list->ma.unique_id == ma.unique_id) {
	       temp_appointment_list->ma.rt = ma.rt;
	    }
	 }
      }
   }
   fclose(pc_in);

   datebook_sort(appointment_list);
}

/* prefs.h
 * A module of J-Pilot http://jpilot.org
 *
 * Copyright (C) 1999-2001 by Judd Montgomery
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
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
#ifndef __PREFS_H__
#define __PREFS_H__

#include "libplugin.h"

#define PREF_RCFILE 0
#define PREF_TIME 1
#define PREF_SHORTDATE 2
#define PREF_LONGDATE 3
#define PREF_FDOW 4 /*First Day Of the Week */
#define PREF_SHOW_DELETED 5
#define PREF_SHOW_MODIFIED 6
#define PREF_HIDE_COMPLETED 7
#define PREF_HIGHLIGHT 8
#define PREF_PORT 9
#define PREF_RATE 10
#define PREF_USER 11
#define PREF_USER_ID 12
#define PREF_PC_ID 13
#define PREF_NUM_BACKUPS 14
#define PREF_WINDOW_WIDTH 15
#define PREF_WINDOW_HEIGHT 16
#define PREF_DATEBOOK_PANE 17
#define PREF_ADDRESS_PANE 18
#define PREF_TODO_PANE 19
#define PREF_MEMO_PANE 20
#define PREF_USE_DB3 21
#define PREF_LAST_APP 22
#define PREF_PRINT_THIS_MANY 23
#define PREF_PRINT_ONE_PER_PAGE 24
#define PREF_NUM_BLANK_LINES 25
#define PREF_PRINT_COMMAND 26
#define PREF_CHAR_SET 27
#define PREF_SYNC_DATEBOOK 28
#define PREF_SYNC_ADDRESS 29
#define PREF_SYNC_TODO 30
#define PREF_SYNC_MEMO 31
#define PREF_SYNC_MEMO32 32
#define PREF_ADDRESS_NOTEBOOK_PAGE 33
#define PREF_OUTPUT_HEIGHT 34
#define PREF_OPEN_ALARM_WINDOWS 35
#define PREF_DO_ALARM_COMMAND 36
#define PREF_ALARM_COMMAND 37
#define PREF_REMIND_IN 38
#define PREF_REMIND_UNITS 39
#define PREF_PASSWORD 40
#define PREF_MEMO32_MODE 41
#define PREF_PAPER_SIZE 42
#define PREF_DATEBOOK_EXPORT_FILENAME 43
#define PREF_DATEBOOK_IMPORT_PATH 44
#define PREF_ADDRESS_EXPORT_FILENAME 45
#define PREF_ADDRESS_IMPORT_PATH 46
#define PREF_TODO_EXPORT_FILENAME 47
#define PREF_TODO_IMPORT_PATH 48
#define PREF_MEMO_EXPORT_FILENAME 49
#define PREF_MEMO_IMPORT_PATH 50
#define PREF_MANANA_MODE 51
#define PREF_SYNC_MANANA 52

#define NUM_PREFS 53

#define MAX_PREF_NUM_BACKUPS 99

#define PREF_MDY 0
#define PREF_DMY 1
#define PREF_YMD 2

#define CHAR_SET_LATIN1   0 /* English, European, Latin based languages */
#define CHAR_SET_JAPANESE 1
#define CHAR_SET_1250     2 /* Czech */
#define CHAR_SET_1251     3 /* Russian; palm koi8-r, host win1251 */
#define CHAR_SET_1251_B   4 /* Russian; palm win1251, host koi8-r */
#define CHAR_SET_TRADITIONAL_CHINESE  5 /* Taiwan Chinese */
#define CHAR_SET_KOREAN   6 /* Korean Hangul */
#define NUM_CHAR_SETS     7

#define MAX_PREF_VALUE 200

void pref_init();
int pref_read_rc_file();
int pref_write_rc_file();
int get_pref(int which, long *n, const char **ret);
int set_pref(int which, long n, const char *string, int save);

/* Specialized functions */
int set_pref_possibility(int which, long n, int save);
int get_pref_possibility(int which, int n, char *ret);
int get_pref_dmy_order();
int get_pref_time_no_secs(char *datef);
int get_pref_time_no_secs_no_ampm(char *datef);

/*
 * Get the preference value as long. If failed to do so, return the
 * specified default.
 */
long get_pref_int_default(int which, long defval);

int make_pref_menu(GtkWidget **pref_menu, int pref_num);

#endif
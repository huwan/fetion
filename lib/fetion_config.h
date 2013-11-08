/***************************************************************************
 *   Copyright (C) 2010 by lwp                                             *
 *   levin108@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.            *
 *                                                                         *
 *   OpenSSL linking exception                                             *
 *   --------------------------                                            *
 *   If you modify this Program, or any covered work, by linking or        *
 *   combining it with the OpenSSL project's "OpenSSL" library (or a       *
 *   modified version of that library), containing parts covered by        *
 *   the terms of OpenSSL/SSLeay license, the licensors of this            *
 *   Program grant you additional permission to convey the resulting       *
 *   work. Corresponding Source for a non-source form of such a            *
 *   combination shall include the source code for the parts of the        *
 *   OpenSSL library used as well as that of the covered work.             *
 ***************************************************************************/

#ifndef FETION_CONFIG_H
#define FETION_CONFIG_H

typedef struct {
	char content[256];
	int phraseid;
} Phrase;

extern Config* fetion_config_new();

extern int fetion_config_download_configuration(User* user);

extern int fetion_config_initialize(Config* config , const char* userid);

extern int fetion_config_load_size(Config *config);

extern int fetion_config_save_size(Config *config);

extern int fetion_config_get_use_status_icon(Config *config);

extern int fetion_config_set_use_status_icon(Config *config);

extern int fetion_config_load(User* user);

extern Proxy* fetion_config_load_proxy();

extern void fetion_config_save_proxy(Proxy *proxy);

extern int fetion_config_save(User* user);

extern char* fetion_config_get_city_name(const char* province , const char* city);

extern char* fetion_config_get_province_name(const char* province);

extern FxList* fetion_config_get_phrase(Config* config);

extern void fetion_phrase_free(Phrase* phrase);

extern void fetion_config_free(Config *config);

/*user list*/
#define foreach_userlist(head , ul) \
	for(ul = head ; (ul = ul->next) != head;)

extern struct userlist* fetion_user_list_new(const char *no,
		const char *password , const char *userid, const char *sid,
	   	int laststate , int islastuser);

extern void fetion_user_list_append(struct userlist *head , struct userlist *ul);

extern void fetion_user_list_save(Config* config , struct userlist* ul);

extern void fetion_user_list_set_lastuser_by_no(struct userlist *ul , const char* no);

extern int fetion_user_list_remove(Config *config, const char *no);

extern struct userlist* fetion_user_list_find_by_no(struct userlist* list , const char* no);

extern struct userlist* fetion_user_list_load(Config* config);

extern void fetion_user_list_update_userid(Config *config,
				const char *no, const char *userid);

extern void fetion_user_list_free(struct userlist *list);


extern xmlNodePtr xml_goto_node(xmlNodePtr node , const char* xml);

extern char* xml_convert(xmlChar* in);

void escape_sql(char *in);

void unescape_sql(char *in);
#endif 

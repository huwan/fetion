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

#include <openfetion.h>

int fetion_buddylist_create(User* user , const char* name)
{
	FetionSip* sip = user->sip;
	SipHeader* eheader;
	char *res , *body;
	int ret;
	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_CREATEBUDDYLIST);
	fetion_sip_add_header(sip , eheader);
	body = generate_create_buddylist_body(name);
	res = fetion_sip_to_string(sip , body);
	free(body);
	tcp_connection_send(sip->tcp , res , strlen(res));
	free(res) ; 
	res = fetion_sip_get_response(sip);
	ret = fetion_sip_get_code(res);
	if(ret == 200)
	{
		ret = parse_create_buddylist_response(user , res);
		free(res);
		debug_info("Create buddy list success");
		return ret;
	}
	else
	{
		free(res);
		debug_error("Create buddy list failed , errno :" , ret);
		return -1;
	}
}
int fetion_buddylist_delete(User* user , int id)
{
	FetionSip* sip = user->sip;
	SipHeader* eheader;
	char *res , *body;
	int ret;
	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_DELETEBUDDYLIST);
	fetion_sip_add_header(sip , eheader);
	body = generate_delete_buddylist_body(id);
	res = fetion_sip_to_string(sip , body);
	free(body);
	tcp_connection_send(sip->tcp , res , strlen(res));
	free(res);
	res = fetion_sip_get_response(sip);
	ret = fetion_sip_get_code(res);
	free(res);
	if(ret == 200)
	{
		fetion_group_remove(user->groupList , id);
		debug_info("Delete buddy list success");
		return 1;
	}
	else
	{
		debug_error("Delete buddy list failed , errno:%d" , ret);
		return -1;
	}
}
int fetion_buddylist_edit(User* user , int id , const char* name)
{
	FetionSip* sip = user->sip;
	SipHeader* eheader;
	char *res , *body;
	int ret;
	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_SETBUDDYLISTINFO);
	fetion_sip_add_header(sip , eheader);
	body = generate_edit_buddylist_body(id , name);
	res = fetion_sip_to_string(sip , body);
	free(body);
	tcp_connection_send(sip->tcp , res , strlen(res));
	free(res);
	res = fetion_sip_get_response(sip);
	ret = fetion_sip_get_code(res);
	free(res);
	if(ret == 200)
	{
		debug_info("Set buddy list name to %s success" , name);
		return 1;
	}
	else
	{
		debug_error("Set buddy list name to %s failed , errno:%d" , name , ret);
		return -1;
	}
}

char* generate_create_buddylist_body(const char* name)
{
	char args[] = "<args></args>";
	xmlChar *res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlParseMemory(args , strlen(args));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "contacts" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "buddy-lists" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "buddy-list" , NULL);
	xmlNewProp(node , BAD_CAST "name" , BAD_CAST name);
	xmlDocDumpMemory(doc , &res , NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);
}
char* generate_edit_buddylist_body(int id , const char* name)
{
	char args[] = "<args></args>";
	char ids[128];
	xmlChar *res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlParseMemory(args , strlen(args));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "contacts" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "buddy-lists" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "buddy-list" , NULL);
	xmlNewProp(node , BAD_CAST "name" , BAD_CAST name);
	memset(ids, 0, sizeof(ids));
	snprintf(ids, sizeof(ids) - 1 , "%d" , id);
	xmlNewProp(node , BAD_CAST "id" , BAD_CAST ids);
	xmlDocDumpMemory(doc , &res , NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);

}
char* generate_delete_buddylist_body(int id)
{
	char args[] = "<args></args>";
	char ida[4];
	memset(ida, 0, sizeof(ida));
	sprintf(ida , "%d" , id);
	xmlChar *res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlParseMemory(args , strlen(args));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "contacts" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "buddy-lists" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "buddy-list" , NULL);
	xmlNewProp(node , BAD_CAST "id" , BAD_CAST ida);
	xmlDocDumpMemory(doc , &res , NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);
}
int parse_create_buddylist_response(User* user , const char* sipmsg)
{
	char *pos;
	Group* group;
	xmlChar* res;
	xmlDocPtr doc;
	xmlNodePtr node;
	int groupid;
	pos = strstr(sipmsg , "\r\n\r\n") + 4;
	doc = xmlParseMemory(pos , strlen(pos));
	node = xmlDocGetRootElement(doc);
	node = node->xmlChildrenNode;
	res = xmlGetProp(node , BAD_CAST "version");
	strcpy(user->contactVersion , (char*)res);
	xmlFree(res);
	node = node->xmlChildrenNode->xmlChildrenNode;
	group = fetion_group_new();
	res = xmlGetProp(node , BAD_CAST "name");
	strcpy(group->groupname , (char*)res);
	xmlFree(res);
	res = xmlGetProp(node , BAD_CAST "id");
	group->groupid = atoi((char*)res);
	groupid = group->groupid;
	xmlFree(res);
	xmlFreeDoc(doc);
	fetion_group_list_append(user->groupList , group);
	return groupid;
}

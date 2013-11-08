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
/*private */

static char* generate_subscribe_body(const char* version);
static char* generate_contact_info_body(const char* userid);
static char* generate_contact_info_by_no_body(const char* no , NumberType nt);
static char* generate_set_mobileno_perssion(const char* userid , int show);
static char* generate_set_displayname_body(const char* userid , const char* name);
static char* generate_move_to_group_body(const char *userid, const char *groupids);
static char* generate_delete_buddy_body(const char* userid);
static char* generate_add_buddy_body(const char* no , NumberType notype
								, int buddylist , const char* localname
								, const char* desc , int phraseid);
static char* generate_handle_contact_request_body(const char* sipuri
								, const char* userid , const char* localname
								, int buddylist , int result );
static Contact* parse_handle_contact_request_response(const char* sipmsg);
static Contact* parse_add_buddy_response(const char* sipmsg , int* statuscode);
static int parse_set_mobileno_permission_response(User* user , const char* sipmsg);
static Contact* parse_contact_info_by_no_response(const char* sipmsg);
static int has_special_word(const char *in);
static char *generate_group_body(const char *userid, const char *buddylist);

Contact* fetion_contact_new()
{
	Contact* list = (Contact*)malloc(sizeof(Contact));
	if(list == NULL){
		return NULL;
	}
	memset(list , 0 , sizeof(Contact));
	list->imageChanged = IMAGE_NOT_INITIALIZED;
	list->state = P_HIDDEN;
	list->pre = list;
	list->next = list;
	return list;
}

void fetion_contact_list_append(Contact* cl , Contact* contact)
{
	cl->next->pre = contact;
	contact->next = cl->next;
	contact->pre = cl;
	cl->next = contact;
}

Contact* fetion_contact_list_find_by_userid(Contact* contactlist , const char* userid)
{
	Contact* cl_cur;
	foreach_contactlist(contactlist , cl_cur){
		if(strcmp(cl_cur->userId , userid) == 0)
			return cl_cur;
	}
	return NULL;
}

Contact* fetion_contact_list_find_by_sipuri(Contact* contactlist , const char* sipuri)
{
	Contact *cl_cur;
	char *sid , *sid1;
	foreach_contactlist(contactlist , cl_cur){
		sid = fetion_sip_get_sid_by_sipuri(cl_cur->sipuri);
		sid1 = fetion_sip_get_sid_by_sipuri(sipuri);
		if(strcmp(sid , sid1) == 0){
			free(sid);
			free(sid1);
			return cl_cur;
		}
		free(sid);
		free(sid1);
	}
	return NULL;
}

Contact *fetion_contact_list_find_by_mobileno(Contact *contactlist, const char *mobileno)
{
	Contact *cl_cur;
	foreach_contactlist(contactlist, cl_cur) {
		if(strcmp(cl_cur->mobileno, mobileno) == 0)
			return cl_cur;
	}
	return NULL;
}

void fetion_contact_list_remove_by_userid(Contact* contactlist , const char* userid)
{
	Contact *cl_cur;
	foreach_contactlist(contactlist , cl_cur){
		if(strcmp(cl_cur->userId , userid) == 0){
			cl_cur->pre->next = cl_cur->next;
			cl_cur->next->pre = cl_cur->pre;
			free(cl_cur);
			break;
		}
	}
}

void fetion_contact_list_remove(Contact *contact)
{
	contact->next->pre = contact->pre;
	contact->pre->next = contact->next;
}

void fetion_contact_list_free(Contact* contact)
{
	Contact *cl_cur , *del_cur;
	for(cl_cur = contact->next ; cl_cur != contact ;){
		cl_cur->pre->next = cl_cur->next;
		cl_cur->next->pre = cl_cur->pre;
		del_cur = cl_cur;
		cl_cur = cl_cur->next;
		free(del_cur);
	}
	free(contact);
}

int fetion_contact_subscribe_only(User* user)
{
	char *res, *body;
	FetionSip* sip;
	SipHeader* eheader;

	sip = user->sip;
	fetion_sip_set_type(sip , SIP_SUBSCRIPTION);
	eheader = fetion_sip_event_header_new(SIP_EVENT_PRESENCE);
	if(eheader == NULL){
		return -1;
	}
	fetion_sip_add_header(sip , eheader);
	body = generate_subscribe_body("0");
	if(body == NULL){
		free(eheader);
		return -1;
	}
	res = fetion_sip_to_string(sip , body);
	if(res == NULL){
		free(eheader);
		free(body);
		return -1;
	}
	free(body);
	debug_info("Start subscribe contact list");
	tcp_connection_send(sip->tcp , res , strlen(res));
	free(res);
	return 0;
}

Contact* fetion_contact_get_contact_info(User* user , const char* userid)
{
	FetionSip* sip = user->sip;
	char *res , *body , *pos;
	char *cur;
	Contact* contact;
	xmlChar* cs;
	xmlDocPtr doc;
	xmlNodePtr node;
	contact = fetion_contact_list_find_by_userid(user->contactList , userid);
	body = generate_contact_info_body(userid);
	if(body == NULL) return NULL;
	fetion_sip_set_type(sip , SIP_SERVICE);
	SipHeader* eheader = fetion_sip_event_header_new(SIP_EVENT_GETCONTACTINFO);
	if(eheader == NULL){
		free(body);
		return NULL;
	}
	fetion_sip_add_header(sip , eheader);
	res = fetion_sip_to_string(sip , body);
	free(body);
	if(res == NULL){	
		free(res);
		return NULL;
	}
	tcp_connection_send(sip->tcp , res , strlen(res));
	free(res);
	res = fetion_sip_get_response(sip);
	if(res == NULL){
		return NULL;
	}

	pos = strstr(res , "\r\n\r\n") + 4;
	doc = xmlParseMemory(pos , strlen(pos));
	if(!doc){
		return NULL;
	}
	node = xmlDocGetRootElement(doc);
	node = node->xmlChildrenNode;
	if(xmlHasProp(node , BAD_CAST "carrier-region")){
		cs = xmlGetProp(node , BAD_CAST "carrier-region");
		pos = (char*)cs;

		for(cur = contact->country;*pos && *pos != '.';*cur ++ = *pos ++);
		*cur = '\0'; pos ++;
		for(cur = contact->province;*pos && *pos != '.';*cur ++ = *pos ++);
		*cur = '\0'; pos ++;
		for(cur = contact->city;*pos && *pos != '.';*cur ++ = *pos ++);
		*cur = '\0';
		xmlFree(cs);
		free(res);
	}
	return contact;
}

int fetion_contact_has_ungrouped(Contact *contactlist)
{
	Contact *cur;

	foreach_contactlist(contactlist , cur){
		if(cur->groupid == BUDDY_LIST_NOT_GROUPED)
		    return 1;
	}
	return 0;

}

int fetion_contact_has_strangers(Contact *contactlist)
{
	Contact *cur;

	foreach_contactlist(contactlist , cur){
		if(cur->groupid == BUDDY_LIST_STRANGER)
		    return 1;
	}
	return 0;

}

Contact* fetion_contact_get_contact_info_by_no(User* user , const char* no , NumberType nt)
{
	FetionSip* sip = user->sip;
	SipHeader* eheader;
	char *res , *body;
	Contact* contact;
	int ret;
	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_GETCONTACTINFO);
	if(eheader == NULL){
		return NULL;
	}
	fetion_sip_add_header(sip , eheader);
	body = generate_contact_info_by_no_body(no , nt);
	if(body == NULL){
		return NULL;
	}
	res = fetion_sip_to_string(sip , body);
	free(body);
	if(res == NULL){
		return NULL;
	}
	ret = tcp_connection_send(sip->tcp , res , strlen(res));
	free(res); res = NULL; 
	if(ret < 0)
		return NULL;

	res = fetion_sip_get_response(sip);
	if(res == NULL){
		return NULL;
	}
	ret = fetion_sip_get_code(res);
	
	if(ret == 200){
 		contact = parse_contact_info_by_no_response(res);
		free(res);
		debug_info("Get user information by mobile number success");
		return contact;
	}else{
		free(res);
		debug_error("Get user information by mobile number failed , errno :" , ret);
		return NULL;
	}
}
int fetion_contact_set_mobileno_permission(User* user , const char* userid , int show)
{
	FetionSip* sip = user->sip;
	SipHeader* eheader;
	char *res , *body;
	int ret;
	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_SETCONTACTINFO);
	if(eheader == NULL)	return -1;
	fetion_sip_add_header(sip , eheader);
	body = generate_set_mobileno_perssion(userid , show);
	if(body == NULL) return -1;
	res = fetion_sip_to_string(sip , body);
	free(body);
	if(res == NULL)	return -1;
	ret = tcp_connection_send(sip->tcp , res , strlen(res));
	free(res) ; 
	if(ret < 0)	return -1;

	res = fetion_sip_get_response(sip);
	if(res == NULL)	return -1;
	ret = fetion_sip_get_code(res);
	if(ret == 200){
		parse_set_mobileno_permission_response(user , res);
		free(res);
		debug_info("Get user information by mobile number success");
		return 0;
	}else{
		free(res);
		debug_error("Get user information by mobile number failed , errno :" , ret);
		return -1;
 	}
}
int fetion_contact_set_displayname(User* user , const char* userid , const char* name)
{
	FetionSip* sip = user->sip;
	SipHeader* eheader;
	char *res , *body;
	int ret;
	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_SETCONTACTINFO);
	if(eheader == NULL){
		return -1;
	}
	fetion_sip_add_header(sip , eheader);
	body = generate_set_displayname_body(userid , name);
	if(body == NULL){
		return -1;
	}
	res = fetion_sip_to_string(sip , body);
	free(body);
	if(res == NULL){
		return -1;
	}
	ret = tcp_connection_send(sip->tcp , res , strlen(res));
	free(res);
	if(ret < 0)
		return -1;
	res = fetion_sip_get_response(sip);
	if(res == NULL){
		return -1;
	}
	ret = fetion_sip_get_code(res);
	free(res);

	if(ret == 200){
		debug_info("Set buddy(%s)`s localname to %s success" , userid , name);
		return 0;
	}else{
		debug_info("Set buddy(%s)`s localname to %s failed" , userid , name);
		return -1;
	}
}

int fetion_contact_move_to_group(User *user, const char *userid, int old_bl, int new_bl)
{
	FetionSip *sip = user->sip;
	SipHeader *eheader;
	Contact    *cnt;
	char        bls[1024] = { 0, };
	char *res, *body;
	int ret;

	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_SETCONTACTINFO);
	fetion_sip_add_header(sip , eheader);

	cnt = fetion_contact_list_find_by_userid(user->contactList, userid);
	foreach_groupids(cnt->groupids) {
		if(group_id == old_bl) continue;
		sprintf(bls + strlen(bls), "%d;", group_id);
	} end_groupids(cnt->groupids)
	sprintf(bls + strlen(bls), "%d", new_bl);
	sprintf(cnt->groupids, "%s", bls);

	body = generate_move_to_group_body(userid, bls);

	if(!body) return -1;
	res = fetion_sip_to_string(sip , body);
	free(body);
	if(!res) return -1;
	ret = tcp_connection_send(sip->tcp , res , strlen(res));
	free(res);

	if(ret < 0)	return -1;

	return 0;
}

int fetion_contact_copy_to_group(User *user, const char *userid, int buddylist)
{
	FetionSip *sip = user->sip;
	SipHeader *eheader;
	Contact   *cnt;
	char *res, *body, newbl[48] = { 0,};
	int ret;
	fetion_sip_set_type(sip , SIP_SERVICE);
	if (!(eheader = fetion_sip_event_header_new(SIP_EVENT_SETCONTACTINFO))) return -1;
	fetion_sip_add_header(sip , eheader);
	
	cnt = fetion_contact_list_find_by_userid(user->contactList, userid);
	
	foreach_groupids(cnt->groupids) {
		if(group_id == buddylist) return -1;
		sprintf(newbl + strlen(newbl), "%d;", group_id);
	} end_groupids(cnt->groupids)
	sprintf(newbl + strlen(newbl), "%d", buddylist);
	sprintf(cnt->groupids, "%s", newbl);
	 
	body = generate_group_body(userid, newbl);
	res = fetion_sip_to_string(sip, body);
	free(body);
	if(!res) return -1;

	ret = tcp_connection_send(sip->tcp , res , strlen(res));
	free(res);

	if(ret < 0)	return -1;

	res = fetion_sip_get_response(sip);
	if(!res) return -1;

	ret = fetion_sip_get_code(res);
	free(res);

	return 0;
}

int fetion_contact_remove_from_group(User *user, const char *userid, int buddylist)
{
	FetionSip *sip = user->sip;
	SipHeader *eheader;
	Contact   *cnt;
	char *res, *body, newbl[48] = { 0,};
	int ret;
	fetion_sip_set_type(sip , SIP_SERVICE);
	if (!(eheader = fetion_sip_event_header_new(SIP_EVENT_SETCONTACTINFO))) return -1;
	fetion_sip_add_header(sip , eheader);
	
	cnt = fetion_contact_list_find_by_userid(user->contactList, userid);
	
	foreach_groupids(cnt->groupids) {
		if(group_id == buddylist) continue;
		sprintf(newbl + strlen(newbl), "%d;", group_id);
	} end_groupids(cnt->groupids)
	if(newbl[strlen(newbl) - 1] == ';') newbl[strlen(newbl) - 1] = '\0';
	sprintf(cnt->groupids, "%s", newbl);
	 
	body = generate_group_body(userid, newbl);
	res = fetion_sip_to_string(sip, body);
	free(body);
	if(!res) return -1;

	ret = tcp_connection_send(sip->tcp , res , strlen(res));
	free(res);

	if(ret < 0)	return -1;

	res = fetion_sip_get_response(sip);
	if(!res) return -1;

	ret = fetion_sip_get_code(res);
	free(res);

	return 0;
}

int fetion_contact_delete_buddy(User* user , const char* userid)
{
	FetionSip* sip = user->sip;
	SipHeader* eheader;
	char *res , *body;
	int ret;
	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_DELETEBUDDY);
	fetion_sip_add_header(sip , eheader);
	body = generate_delete_buddy_body(userid);
	if(body == NULL) return -1;
	res = fetion_sip_to_string(sip , body);
	free(body);
	if(res == NULL)	return -1;
#if 0
	if(fetion_contact_del_localbuddy(user, userid) == -1)
		return -1;
#endif

	ret = tcp_connection_send(sip->tcp , res , strlen(res));
	free(res);

	if(ret < 0)	return -1;

	res = fetion_sip_get_response(sip);
	if(res == NULL)	return -1;
	ret = fetion_sip_get_code(res);
	free(res);

	return 0;
}
Contact* fetion_contact_add_buddy(User* user , const char* no
								, NumberType notype , int buddylist
								, const char* localname , const char* desc
								, int phraseid , int* statuscode)
{
	FetionSip* sip = user->sip;
	SipHeader* eheader = NULL;
	SipHeader* ackheader = NULL;
	char *res = NULL;
	char *body = NULL;
	int ret;
	Contact* contact;
	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_ADDBUDDY);
	if(eheader == NULL){
		return NULL;
	}
	fetion_sip_add_header(sip , eheader);
	if(user->verification != NULL && user->verification->algorithm != NULL)	
	{
		ackheader = fetion_sip_ack_header_new(user->verification->code
											, user->verification->algorithm
											, user->verification->type
											, user->verification->guid);
		if(ackheader == NULL){
			return NULL;
		}
		fetion_sip_add_header(sip , ackheader);
	}
	body = generate_add_buddy_body(no , notype , buddylist , localname , desc , phraseid);
	if(body == NULL){
		return NULL;
	}

	res = fetion_sip_to_string(sip , body);
	free(body);
	if(res == NULL){
		return NULL;
	}
	tcp_connection_send(sip->tcp , res , strlen(res));
	free(res);
	res = fetion_sip_get_response(sip);
	if(res == NULL){
		return NULL;
	}
	ret = fetion_sip_get_code(res);
	*statuscode = ret;
	int rtv;
	switch(ret)
	{
		case 200 :
			contact = parse_add_buddy_response(res , statuscode);
			fetion_verification_free(user->verification);
			user->verification = NULL;
			free(res);
			if(contact == NULL){
				debug_info("Add buddy(%s) failed" , no);
				return NULL;
			}
			fetion_contact_list_append(user->contactList , contact);
			debug_info("Add buddy(%s) success" , no);
			return contact;
		case 421 : 
		case 420 :
			rtv = parse_add_buddy_verification(user , res);
			free(res);
			if(rtv != 0){
				debug_info("Add buddy(%s) falied , need verification, but parse error" , no);
				return NULL;
			}
			debug_info("Add buddy(%s) falied , need verification" , no);
			return NULL;
		default:
			free(res);
			debug_info("Add buddy(%s) failed" , no);
			return NULL;
	}
}

Contact* fetion_contact_handle_contact_request(User* user
		, const char* sipuri , const char* userid
		, const char* localname , int buddylist , int result)
{
	FetionSip* sip = user->sip;
	SipHeader* eheader;
	char *res , *body;
	int ret;
	Contact* contact;
	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_HANDLECONTACTREQUEST);
	if(eheader == NULL){
		return NULL;
	}
	fetion_sip_add_header(sip , eheader);
	body = generate_handle_contact_request_body(sipuri , userid , localname , buddylist , result);
	if(body == NULL){
		return NULL;
	}
	res = fetion_sip_to_string(sip , body);
	free(body);
	if(res == NULL){
		return NULL;
	}
	tcp_connection_send(sip->tcp , res , strlen(res));
	free(res);
	res = fetion_sip_get_response(sip);
	if(res == NULL){
		return NULL;
	}
	ret = fetion_sip_get_code(res);
	switch(ret)
	{
		case 200 :
			contact = parse_handle_contact_request_response(res);
			free(res);
			if(contact == NULL){
				debug_info("handle contact request from (%s) failed" , userid);
				return NULL;
			}
			fetion_contact_list_append(user->contactList , contact);
			debug_info("handle contact request from (%s) success" , userid);
			return contact;
		default:
			free(res);
			debug_info("handle contact request from (%s) failed" , userid);
			return NULL;
	}
	return NULL;
}

char* generate_subscribe_body(const char* version)
{
	xmlChar *buf;
	xmlDocPtr doc;
	xmlNodePtr node;
	char body[] = "<args></args>";
	doc = xmlParseMemory(body , strlen(body));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "subscription" , NULL);
	xmlNewProp(node , BAD_CAST "self" , BAD_CAST "v4default;mail-count");
	xmlNewProp(node , BAD_CAST "buddy" , BAD_CAST "v4default");
	xmlNewProp(node , BAD_CAST "version" , BAD_CAST version);
	xmlDocDumpMemory(doc , &buf , NULL);
	xmlFreeDoc(doc);
	return xml_convert(buf);
}
char* generate_contact_info_body(const char* userid)
{
	xmlChar *buf;
	xmlDocPtr doc;
	xmlNodePtr node;
	char body[] = "<args></args>";
	doc = xmlParseMemory(body , strlen(body));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "contact" , NULL);
	xmlNewProp(node , BAD_CAST "user-id" , BAD_CAST userid);
	xmlDocDumpMemory(doc , &buf , NULL);
	xmlFreeDoc(doc);
	return xml_convert(buf);
	
}
char* generate_contact_info_by_no_body(const char* no , NumberType nt)
{
	xmlChar *buf;
	xmlDocPtr doc;
	xmlNodePtr node;
	char uri[32];
	char body[] = "<args></args>";
	if(nt == MOBILE_NO)
		sprintf(uri , "tel:%s" , no);
	else
		sprintf(uri , "sip:%s" , no);
	doc = xmlParseMemory(body , strlen(body));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "contact" , NULL);
	xmlNewProp(node , BAD_CAST "uri" , BAD_CAST uri);
	xmlDocDumpMemory(doc , &buf , NULL);
	xmlFreeDoc(doc);
	return xml_convert(buf);
}
char* generate_set_mobileno_perssion(const char* userid , int show)
{
	xmlChar *buf;
	xmlDocPtr doc;
	xmlNodePtr node;
	char permission[32];
	char body[] = "<args></args>";
	sprintf(permission , "identity=%d" , show);
	doc = xmlParseMemory(body , strlen(body));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "contacts" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "contact" , NULL);
	xmlNewProp(node , BAD_CAST "user-id" , BAD_CAST userid);
	xmlNewProp(node , BAD_CAST "permission" , BAD_CAST permission);
	xmlDocDumpMemory(doc , &buf , NULL);
	xmlFreeDoc(doc);
	return xml_convert(buf);
}
char* generate_handle_contact_request_body(const char* sipuri
		, const char* userid , const char* localname
		, int buddylist , int result )
{
	char args[] = "<args></args>";
	char result_s[4];
	char buddylist_s[4];
	xmlChar *res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlParseMemory(args , strlen(args));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "contacts" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "buddies" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "buddy" , NULL);
	xmlNewProp(node , BAD_CAST "user-id" , BAD_CAST userid);
	xmlNewProp(node , BAD_CAST "uri" , BAD_CAST sipuri);
	sprintf(result_s , "%d" , result);
	sprintf(buddylist_s , "%d" , buddylist);
	xmlNewProp(node , BAD_CAST "result" , BAD_CAST result_s);
	xmlNewProp(node , BAD_CAST "buddy-lists" , BAD_CAST buddylist_s);
	xmlNewProp(node , BAD_CAST "expose-mobile-no" , BAD_CAST "1");
	xmlNewProp(node , BAD_CAST "expose-name" , BAD_CAST "1");
	xmlNewProp(node , BAD_CAST "local-name" , BAD_CAST localname);
	xmlDocDumpMemory(doc , &res , NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);
}
char* generate_set_displayname_body(const char* userid , const char* name)
{
	char args[] = "<args></args>";
	xmlChar *res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlParseMemory(args , strlen(args));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "contacts" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "contact" , NULL);
	xmlNewProp(node , BAD_CAST "user-id" , BAD_CAST userid);
	xmlNewProp(node , BAD_CAST "local-name" , BAD_CAST name);
	xmlDocDumpMemory(doc , &res , NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);
}

static char *generate_move_to_group_body(const char *userid, const char *groupids)
{
	char args[] = "<args></args>";
	xmlChar *res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlParseMemory(args , strlen(args));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "contacts" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "contact" , NULL);
	xmlNewProp(node , BAD_CAST "user-id" , BAD_CAST userid);
	xmlNewProp(node , BAD_CAST "buddy-lists" , BAD_CAST groupids);
	xmlDocDumpMemory(doc , &res , NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);
}

char* generate_delete_buddy_body(const char* userid)
{
	char args[] = "<args></args>";
	xmlChar *res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlParseMemory(args , strlen(args));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "contacts" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "buddies" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "buddy" , NULL);
	xmlNewProp(node , BAD_CAST "user-id" , BAD_CAST userid);
	xmlDocDumpMemory(doc , &res , NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);
}

char* generate_add_buddy_body(const char* no 
		, NumberType notype , int buddylist 
		, const char* localname , const char* desc , int phraseid)
{
	char args[] = "<args></args>";
	char uri[48];
	char phrase[4];
	char groupid[4];
	xmlChar *res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlParseMemory(args , strlen(args));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "contacts" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "buddies" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "buddy" , NULL);

	if(notype == FETION_NO)
		sprintf(uri , "sip:%s" , no);
	else
		sprintf(uri , "tel:%s" , no);

	sprintf(phrase , "%d" , phraseid);
	sprintf(groupid , "%d" , buddylist);
	xmlNewProp(node , BAD_CAST "uri" , BAD_CAST uri);
	xmlNewProp(node , BAD_CAST "local-name" , BAD_CAST localname);
	xmlNewProp(node , BAD_CAST "buddy-lists" , BAD_CAST groupid);
	xmlNewProp(node , BAD_CAST "desc" , BAD_CAST desc);
	xmlNewProp(node , BAD_CAST "expose-mobile-no" , BAD_CAST "1");
	xmlNewProp(node , BAD_CAST "expose-name" , BAD_CAST "1");
	xmlNewProp(node , BAD_CAST "addbuddy-phrase-id" , BAD_CAST phrase);
	xmlDocDumpMemory(doc , &res , NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);
}

static char *generate_group_body(const char *userid, const char *buddylist)
{
	char args[] = "<args></args>";
	xmlChar *res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlParseMemory(args , strlen(args));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "contacts" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "contact" , NULL);

	xmlNewProp(node , BAD_CAST "user-id" , BAD_CAST userid);
	xmlNewProp(node , BAD_CAST "buddy-lists" , BAD_CAST buddylist);
	xmlDocDumpMemory(doc , &res , NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);
}

static int parse_set_mobileno_permission_response(User* user , const char* sipmsg)
{
	char *pos;
	xmlChar* res;
	xmlDocPtr doc;
	xmlNodePtr node;
	pos = strstr(sipmsg , "\r\n\r\n") + 4;
	doc = xmlParseMemory(pos , strlen(pos));
	node = xmlDocGetRootElement(doc);
	node = node->xmlChildrenNode;
	res = xmlGetProp(node , BAD_CAST "contact-list-version");
	memset(user->contactVersion, 0, sizeof(user->contactVersion));
	strcpy(user->contactVersion , (char*)res);
	xmlFree(res);
	xmlFreeDoc(doc);
	return 0;
}
static Contact* parse_contact_info_by_no_response(const char* sipmsg)
{
	char *pos;
	Contact* contact;
	xmlChar* res;
	xmlDocPtr doc;
	xmlNodePtr node;
	contact = fetion_contact_new();
	pos = strstr(sipmsg , "\r\n\r\n") + 4;
	doc = xmlParseMemory(pos , strlen(pos));
	node = xmlDocGetRootElement(doc);
	node = node->xmlChildrenNode;
	if(xmlHasProp(node , BAD_CAST "uri"))
	{
		res = xmlGetProp(node , BAD_CAST "uri");
		strcpy(contact->sipuri , (char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "user-id"))
	{
		res = xmlGetProp(node , BAD_CAST "user-id");
		strcpy(contact->userId , (char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "sid"))
	{
		res = xmlGetProp(node , BAD_CAST "sid");
		strcpy(contact->sId , (char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "nickname"))
	{
		res = xmlGetProp(node , BAD_CAST "nickname");
		strcpy(contact->nickname , (char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "gender"))
	{
		res = xmlGetProp(node , BAD_CAST "gender");
		contact->gender = atoi((char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "birth-date"))
	{
		res = xmlGetProp(node , BAD_CAST "birth-date");
		strcpy(contact->birthday , (char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "impresa"))
	{
		res = xmlGetProp(node , BAD_CAST "impresa");
		strcpy(contact->impression , (char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "mobile-no"))
	{
		res = xmlGetProp(node , BAD_CAST "mobile-no");
		strcpy(contact->mobileno , (char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "carrier-region"))
	{
		int n;
		res = xmlGetProp(node , BAD_CAST "carrier-region");
		pos = (char*)res;
                if (!(pos == NULL || strcmp(pos, "") == 0)) {
                    n = strlen(pos) - strlen(strstr(pos , "."));
                    strncpy(contact->country , pos , n);
                    pos = strstr(pos , ".") + 1;
                    n = strlen(pos) - strlen(strstr(pos , "."));
                    strncpy(contact->province , pos , n);
                    pos = strstr(pos , ".") + 1;
                    n = strlen(pos) - strlen(strstr(pos , "."));
                    strncpy(contact->city , pos , n);
                }
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "portrait-crc"))
	{
		res = xmlGetProp(node , BAD_CAST "portrait-crc");
		strcpy(contact->portraitCrc , (char*)res);
		xmlFree(res);
	}
	xmlFreeDoc(doc);
	return contact;
}
static Contact* parse_add_buddy_response(const char* sipmsg , int* statuscode)
{
	char *pos;
	Contact* contact;
	xmlChar* res;
	xmlDocPtr doc;
	xmlNodePtr node;
	contact = fetion_contact_new();
	pos = strstr(sipmsg , "\r\n\r\n") + 4;
	doc = xmlParseMemory(pos , strlen(pos));
	node = xmlDocGetRootElement(doc);
	node = xml_goto_node(node , "buddy");

	if(node == NULL)
	{
		*statuscode = 400;
		free(contact);
		return NULL;
	}
	if(xmlHasProp(node , BAD_CAST "uri"))
	{
		res = xmlGetProp(node , BAD_CAST "uri");
		strcpy(contact->sipuri , (char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "user-id"))
	{
		res = xmlGetProp(node , BAD_CAST "user-id");
		strcpy(contact->userId , (char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "mobile-no"))
	{
		res = xmlGetProp(node , BAD_CAST "mobile-no");
		strcpy(contact->mobileno , (char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "local-name"))
	{
		res = xmlGetProp(node , BAD_CAST "local-name");
		strcpy(contact->localname , (char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "buddy-lists"))
	{
		res = xmlGetProp(node , BAD_CAST "buddy-lists");
		contact->groupid = atoi((char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "status-code"))
	{
		res = xmlGetProp(node , BAD_CAST "status-code");
		*statuscode = atoi((char*)res);
		xmlFree(res);
	}
	else
	{
		*statuscode = 200;
	}
	if(xmlHasProp(node , BAD_CAST "basic-service-status"))
	{
		res = xmlGetProp(node , BAD_CAST "basic-service-status");
		contact->serviceStatus = atoi((char*)res);
		xmlFree(res);
	}
	contact->relationStatus = STATUS_NOT_AUTHENTICATED;
	xmlFreeDoc(doc);
	return contact;
}
Contact* parse_handle_contact_request_response(const char* sipmsg)
{
	char *pos = NULL;
	Contact* contact = NULL;
	xmlChar* res = NULL;
	xmlDocPtr doc;
	xmlNodePtr node;
	contact = fetion_contact_new();

	pos = strstr(sipmsg , "\r\n\r\n") + 4;
	doc = xmlParseMemory(pos , strlen(pos));
	node = xmlDocGetRootElement(doc);
	node = xml_goto_node(node , "buddy");
	if(xmlHasProp(node , BAD_CAST "uri"))
	{
		res = xmlGetProp(node , BAD_CAST "uri");
		strcpy(contact->sipuri , (char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "user-id"))
	{
		res = xmlGetProp(node , BAD_CAST "user-id");
		strcpy(contact->userId , (char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "local-name"))
	{
		res = xmlGetProp(node , BAD_CAST "local-name");
		strcpy(contact->localname , (char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "buddy-lists"))
	{
		res = xmlGetProp(node , BAD_CAST "buddy-lists");
		contact->groupid = atoi((char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "relation-status")){
		res = xmlGetProp(node , BAD_CAST "relation-status");
		contact->relationStatus = atoi((char*)res);
		xmlFree(res);
	}else{
		contact->relationStatus = RELATION_STATUS_AUTHENTICATED;
	}
	xmlFreeDoc(doc);
	return contact;
}
int parse_add_buddy_verification(User* user , const char* str)
{
	char* xml = NULL;
	char w[128];
	int n = 0;
	xmlDocPtr doc;
	xmlNodePtr node;
	xmlChar *res = NULL;
	Verification *ver = NULL;

	ver = (Verification*)malloc(sizeof(Verification));
	memset(ver , 0 , sizeof(sizeof(Verification)));

	memset(w, 0, sizeof(w));
	fetion_sip_get_attr(str , "W" , w);
	xml = strstr(w , "algorithm=") + 11;
	n = strlen(xml) - strlen(strstr(xml , "\""));
	ver->algorithm = (char*)malloc(n + 1);
	memset(ver->algorithm, 0, n + 1);
	strncpy(ver->algorithm , xml , n);
	xml = strstr(w , "type=") + 6;
	n = strlen(xml) - strlen(strstr(xml , "\""));
	ver->type = (char*)malloc(n + 1);
	memset(ver->type, 0, n +1);
	strncpy(ver->type , xml , n);

	xml = strstr(str , "\r\n\r\n");
	doc = xmlParseMemory(xml , strlen(xml));
	node = xmlDocGetRootElement(doc);
	node = node->xmlChildrenNode;
	res = xmlGetProp(node , BAD_CAST "text");
	n = xmlStrlen(res) + 1;
	ver->text = (char*)malloc(n);
	memset(ver->text, 0, n);
	strncpy(ver->text , (char*)res , n - 1);
	xmlFree(res);
	res = xmlGetProp(node , BAD_CAST "tips");
	n = xmlStrlen(res) + 1;
	ver->tips = (char*)malloc(n);
	memset(ver->tips, 0, n);
	strncpy(ver->tips , (char*)res , n - 1);
	xmlFree(res);
	user->verification = ver;
	return 0;
}

void fetion_contact_load(User *user, int *gcount, int *bcount)
{
	char path[256];
	char sql[4096];
	sqlite3 *db;
	char **sqlres;
	int ncols, nrows, i, j, start;
	Contact *pos;
	Group *gpos;
	Config *config = user->config;

	debug_info("Load contact list");

	*gcount = 0;
	*bcount = 0;

	sprintf(path , "%s/data.db" , config->userPath);
	if(sqlite3_open(path, &db)){
		debug_error("failed to load contact list");
		return;
	}

	sprintf(sql, "select * from groups order by groupid;");
	if(sqlite3_get_table(db, sql, &sqlres, &nrows, &ncols, NULL)){
		sqlite3_close(db);
		return;
	}

	*gcount = nrows;

	for(i = 0; i < nrows; i++){
		gpos = fetion_group_new();
		for(j = 0; j < ncols; j++){
			start = ncols + i * ncols;
			gpos->groupid = atoi(sqlres[start]);
			strcpy(gpos->groupname, sqlres[start+1]);
		}
		fetion_group_list_append(user->groupList, gpos);
	}
	sqlite3_free_table(sqlres);

	sprintf(sql, "select * from contacts_2_2_0;");
	if(sqlite3_get_table(db, sql, &sqlres, &nrows, &ncols, NULL)){
		sqlite3_close(db);
		return;
	}

	*bcount = nrows;
	
	for(i = 0; i < nrows; i++){
		pos = fetion_contact_new();
		for(j = 0; j < ncols; j++){
			start = ncols + i * ncols;
			strcpy(pos->userId, 	sqlres[start]);
			strcpy(pos->sId, 		sqlres[start+1]);
			strcpy(pos->sipuri, 	sqlres[start+2]);
			strcpy(pos->localname,  sqlres[start+3]);
			strcpy(pos->nickname, 	sqlres[start+4]);
			strcpy(pos->impression, sqlres[start+5]);
			strcpy(pos->mobileno, 	sqlres[start+6]);
			strcpy(pos->devicetype, sqlres[start+7]);
			strcpy(pos->portraitCrc,sqlres[start+8]);
			strcpy(pos->birthday, 	sqlres[start+9]);
			strcpy(pos->country, 	sqlres[start+10]);
			strcpy(pos->province, 	sqlres[start+11]);
			strcpy(pos->city, 		sqlres[start+12]);
			pos->identity = 		atoi(sqlres[start+13]);
			pos->scoreLevel = 		atoi(sqlres[start+14]);
			pos->serviceStatus = 	atoi(sqlres[start+15]);
			pos->carrierStatus = 	atoi(sqlres[start+16]);
			pos->relationStatus = 	atoi(sqlres[start+17]);
			strcpy(pos->carrier,	sqlres[start+18]);
			pos->groupid = 			atoi(sqlres[start+19]);
			pos->gender = 			atoi(sqlres[start+20]);
			strcpy(pos->groupids,   sqlres[start+21]);
		}
		fetion_contact_list_append(user->contactList, pos);
	}
	sqlite3_close(db);
	sqlite3_free_table(sqlres);
}

void fetion_contact_save(User *user)
{
	char path[256];
	char sql[4096];
	sqlite3 *db;
	Contact *pos;
	Group *gpos;
	Config *config = user->config;

	debug_info("Save contact list");

	sprintf(path , "%s/data.db" , config->userPath);
	if(sqlite3_open(path, &db)){
		debug_error("failed to save user list");
		return;
	}
	/* begin transaction */
	if(sqlite3_exec(db, "BEGIN TRANSACTION;", 0,0, NULL)){
		debug_error("begin transaction :%s", sqlite3_errmsg(db));
		sqlite3_close(db);
		return;
	}

	sprintf(sql, "delete from groups");
	if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
		sprintf(sql, "create table groups (groupid,groupname)");
		if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
			debug_error("create table groups:%s", sqlite3_errmsg(db));
			sqlite3_close(db);
			return;
		}
	}
	foreach_grouplist(user->groupList, gpos){
		snprintf(sql, sizeof(sql)-1, "insert into groups "
				"values (%d,'%s');", gpos->groupid,
				gpos->groupname);
		if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
			debug_error("insert group info:%s", sqlite3_errmsg(db));
			continue;
		}
	}
	
	sprintf(sql, "delete from contacts_2_2_0;");
	if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
		sprintf(sql, "create table contacts_2_2_0 (userId,"
						"sId,sipuri,localname,nickname,"
						"impression,mobileno,devicetype,"
						"portraitCrc,birthday,country,"
						"province,city,identity,scoreLevel,"
						"serviceStatus,carrierStatus,"
						"relationStatus,carrier,groupid,gender,groupids);");
		if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
			debug_error("create table contacts:%s", sqlite3_errmsg(db));
			sqlite3_close(db);
			return;
		}
	}
	foreach_contactlist(user->contactList, pos){
		snprintf(sql, sizeof(sql)-1, "insert into contacts_2_2_0 "
				"values ('%s','%s','%s','%s','%s','%s',"
				"'%s','%s','%s','%s','%s','%s','%s','%d',%d,"
				"%d,%d,%d,'%s',%d,%d,'%s');",
				pos->userId, pos->sId, pos->sipuri,
				has_special_word(pos->localname) ? "": pos->localname,
			   	has_special_word(pos->nickname)? "": pos->nickname,
				has_special_word(pos->impression)?"": pos->impression,
			   	pos->mobileno,
				pos->devicetype, pos->portraitCrc,
				pos->birthday, pos->country, pos->province,
				pos->city, pos->identity, pos->scoreLevel,
				pos->serviceStatus, pos->carrierStatus,
				pos->relationStatus, pos->carrier,
				pos->groupid, pos->gender, pos->groupids);
		if(sqlite3_exec(db, sql, NULL, NULL, NULL))
			debug_error("insert contact %s:%s\n%s",
					pos->userId, sqlite3_errmsg(db), sql);
	}
	/* begin transaction */
	if(sqlite3_exec(db, "END TRANSACTION;", 0,0, NULL)){
		debug_error("end transaction :%s", sqlite3_errmsg(db));
		sqlite3_close(db);
		return;
	}
	sqlite3_close(db);
	debug_info("Save contact list successfully");
}

void fetion_contact_update(User *user, Contact *contact)
{
	char path[256];
	char sql[4096];
	sqlite3 *db;
	Config *config = user->config;

	debug_info("Update contact information");

	sprintf(path , "%s/data.db" , config->userPath);
	if(sqlite3_open(path, &db)){
		debug_error("failed to load user list");
		return;
	}

	snprintf(sql, sizeof(sql)-1, "update contacts_2_2_0 set "
			"userId='%s',sId='%s',sipuri='%s',"
			"localname='%s',nickname='%s',"
			"impression='%s',mobileno='%s',"
			"devicetype='%s',portraitCrc='%s',"
			"birthday='%s',country='%s',"
			"province='%s',city='%s',"
			"identity=%d,scoreLevel=%d,"
			"serviceStatus=%d,carrierStatus=%d,"
			"relationStatus=%d,carrier='%s',"
			"groupid=%d,gender=%d,groupids='%s' where userId='%s'",
			contact->userId, contact->sId, contact->sipuri,
			contact->localname, contact->nickname,
			contact->impression, contact->mobileno,
			contact->devicetype, contact->portraitCrc,
			contact->birthday, contact->country, contact->province,
			contact->city, contact->identity, contact->scoreLevel,
			contact->serviceStatus, contact->carrierStatus,
			contact->relationStatus, contact->carrier,
			contact->groupid, contact->gender, contact->groupids, contact->userId);

	if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
		debug_error("update contact %s:%s", contact->userId, sqlite3_errmsg(db));
		sprintf(sql, "create table contacts_2_2_0 (userId,"
					"sId,sipuri,localname,nickname,"
					"impression,mobileno,devicetype,"
					"portraitCrc,birthday,country,"
					"province,city,identity,scoreLevel,"
					"serviceStatus,carrierStatus,"
					"relationStatus,carrier,groupid,gender,groupids);");
		if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
			debug_error("create table contacts:%s", sqlite3_errmsg(db));
			sqlite3_close(db);
			return;
		}

		snprintf(sql, sizeof(sql)-1, "insert into contacts_2_2_0 "
					"values ('%s','%s','%s','%s','%s','%s',"
					"'%s','%s','%s','%s','%s','%s','%s','%d',%d,"
					"%d,%d,%d,'%s',%d,%d,'%s');",
					contact->userId, contact->sId, contact->sipuri,
					contact->localname, contact->nickname,
					contact->impression, contact->mobileno,
					contact->devicetype, contact->portraitCrc,
					contact->birthday, contact->country, contact->province,
					contact->city, contact->identity, contact->scoreLevel,
					contact->serviceStatus, contact->carrierStatus,
					contact->relationStatus, contact->carrier,
					contact->groupid, contact->gender, contact->groupids);

		if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
			debug_error("insert contacts:%s", sqlite3_errmsg(db));
			sqlite3_close(db);
			return;
		}
	}
	sqlite3_close(db);
}

int fetion_contact_del_localbuddy(User *user, const char *userid)
{
	char path[256];
	char sql[4096];
	sqlite3 *db;
	Config *config = user->config;

	sprintf(path , "%s/data.db" , config->userPath);
	if(sqlite3_open(path, &db)){
		debug_error("failed to delete localbuddy");
		return -1;
	}

	sprintf(sql, "delete from contacts_2_2_0 where "
			"userid='%s';", userid);
	if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
		debug_error("failed to delete localbuddy:%s",sqlite3_errmsg(db));
		return -1;
	}
	return 0;
}

int fetion_contact_del_localgroup(User *user, const char *groupid)
{
	char path[256];
	char sql[4096];
	sqlite3 *db;
	Config *config = user->config;

	sprintf(path , "%s/data.db" , config->userPath);
	if(sqlite3_open(path, &db)){
		debug_error("failed to delete localgroup");
		return -1;
	}

	sprintf(sql, "delete from groups where "
			"id='%s';", groupid);
	if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
		debug_error("failed to delete localgroup:%s",sqlite3_errmsg(db));
		return -1;
	}
	return 0;
}

static int has_special_word(const char *in)
{
	int i = 0;
	int inlength=(int)strlen(in);
	for(;i< inlength; i++){
		if(in[i] == '\'')
			return 1;
	}
	return 0;
}

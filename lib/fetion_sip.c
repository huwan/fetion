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

#define _XOPEN_SOURCE
#include <openfetion.h>

int callid = 1;

FetionSip* fetion_sip_new(FetionConnection* tcp , const char* sid)
{
	FetionSip* sip = (FetionSip*)malloc(sizeof(FetionSip));
	memset(sip , 0 , sizeof(FetionSip));
	strcpy(sip->from , sid);
	sip->sequence = 2;
	sip->tcp = tcp;
	sip->header = NULL;
	return sip;
}

FetionSip* fetion_sip_clone(FetionSip* sip)
{
	FetionSip* res = (FetionSip*)malloc(sizeof(FetionSip));
	memset(res , 0 , sizeof(FetionSip));
	memcpy(res , sip , sizeof(FetionSip));
	sip->header = NULL;
	return res;
}

SipHeader* fetion_sip_header_new(const char* name , const char* value)
{
	SipHeader* header = (SipHeader*)malloc(sizeof(SipHeader));
	memset(header , 0 , sizeof(SipHeader));
	strcpy(header->name , name);
	header->value = (char*)malloc(strlen(value) + 2 );
	memset(header->value , 0 , strlen(value) + 2);
	strcpy(header->value , value);
	header->next = NULL;
	return header;
}

void fetion_sip_set_type(FetionSip *sip, SipType type)
{
	if(!sip) return;
	sip->type = type;
	sip->callid = callid;
}

void fetion_sip_set_callid(FetionSip* sip , int callid)
{
	sip->callid = callid;
}

SipHeader* fetion_sip_authentication_header_new(const char* response)
{
	int len;
	char* res;
	char start[] = "Digest response=\"";
	char end[]   = "\",algorithm=\"SHA1-sess-v4\"";
	SipHeader* header;
	
	len = strlen(start) + strlen(end) + strlen(response) + 10;
	res = (char*)malloc(len);
	memset(res, 0 , len);
	sprintf(res, "%s%s%s" , start , response , end);
	header = (SipHeader*)malloc(sizeof(SipHeader));
	memset(header , 0 , sizeof(SipHeader));
	strcpy(header->name , "A");
	header->value = res;
	return header;
}

SipHeader* fetion_sip_ack_header_new(const char* code , const char* algorithm , const char* type , const char* guid)
{
	char ack[512];
	sprintf(ack , "Verify response=\"%s\",algorithm=\"%s\",type=\"%s\",chid=\"%s\""
			 	, code , algorithm , type , guid);
	return fetion_sip_header_new("A" , ack);
}

SipHeader* fetion_sip_event_header_new(int eventType)
{
	char event[48];
	memset(event, 0, sizeof(event));
	switch(eventType)
	{
		case SIP_EVENT_PRESENCE :
			strcpy(event , "PresenceV4");
			break;
		case SIP_EVENT_SETPRESENCE :
			strcpy(event , "SetPresenceV4");
			break;
		case SIP_EVENT_CATMESSAGE :
			strcpy(event , "CatMsg");
			break;
		case SIP_EVENT_SENDCATMESSAGE :
			strcpy(event , "SendCatSMS");
			break;
		case SIP_EVENT_STARTCHAT :
			strcpy(event , "StartChat");
			break;
		case SIP_EVENT_GETCONTACTINFO :
			strcpy(event , "GetContactInfoV4");
			break;
		case SIP_EVENT_CONVERSATION :
			strcpy(event , "Conversation");
			break;
		case SIP_EVENT_INVITEBUDDY :
			strcpy(event , "InviteBuddy");
			break;
		case SIP_EVENT_CREATEBUDDYLIST :
			strcpy(event , "CreateBuddyList");
			break;
		case SIP_EVENT_DELETEBUDDYLIST :
			strcpy(event , "DeleteBuddyList");
			break;
		case SIP_EVENT_SETCONTACTINFO :
			strcpy(event , "SetContactInfoV4");
			break;
		case SIP_EVENT_SETUSERINFO :
			strcpy(event , "SetUserInfoV4");
			break;
		case SIP_EVENT_SETBUDDYLISTINFO :
			strcpy(event , "SetBuddyListInfo");
			break;
		case SIP_EVENT_DELETEBUDDY :
			strcpy(event , "DeleteBuddyV4");
			break;
		case SIP_EVENT_ADDBUDDY :
			strcpy(event , "AddBuddyV4");
			break;
		case SIP_EVENT_KEEPALIVE :
			strcpy(event , "KeepAlive");
			break;
		case SIP_EVENT_DIRECTSMS :
			strcpy(event , "DirectSMS");
			break;
		case SIP_EVENT_HANDLECONTACTREQUEST :
			strcpy(event , "HandleContactRequestV4");
			break;
		case SIP_EVENT_SENDDIRECTCATSMS :
			strcpy(event , "SendDirectCatSMS");
			break;
		case SIP_EVENT_PGGETGROUPLIST:
			strcpy(event , "PGGetGroupList");
			break;
		case SIP_EVENT_PGGETGROUPINFO:
			strcpy(event , "PGGetGroupInfo");
			break;
		case SIP_EVENT_PGPRESENCE:
			strcpy(event , "PGPresence");
			break;
		case SIP_EVENT_PGGETGROUPMEMBERS:
			strcpy(event , "PGGetGroupMembers");
			break;
		case SIP_EVENT_PGSENDCATSMS:
			strcpy(event , "PGSendCatSMS");
			break;
		default:
			break;
	}
	return fetion_sip_header_new("N" , event);
}

SipHeader* fetion_sip_credential_header_new(const char* credential)
{
	char value[64];
	memset(value , 0, sizeof(value));
	sprintf(value , "TICKS auth=\"%s\"" , credential);
	return fetion_sip_header_new("A" , value);
}

void fetion_sip_add_header(FetionSip* sip , SipHeader* header)
{
	SipHeader* pos = sip->header;
	if(pos == NULL)
	{
		sip->header = header;
		return;
	}
	while(pos != NULL)
	{
		if(pos->next == NULL)
		{
			pos->next = header;
			break;
		}
		pos = pos->next;
	}
}

char* fetion_sip_to_string(FetionSip* sip , const char* body)
{
	char *res , *head , buf[1024] , type[128];
	SipHeader *pos , *tmp;
	int len = 0;

	pos = sip->header;
	while(pos){
		len += (strlen(pos->value) + strlen(pos->name) + 5);
		pos = pos->next;
	}
	len += (body == NULL ? 100 : strlen(body) + 100 );
	res = (char*)malloc(len + 1);
	memset(res , 0 , len + 1);
	memset(type, 0 , sizeof(type));
	switch(sip->type){
		case SIP_REGISTER     : strcpy(type , "R");
			break;
		case SIP_SUBSCRIPTION :	strcpy(type , "SUB");
			break;
		case SIP_SERVICE 	  : strcpy(type , "S");
			break;
		case SIP_MESSAGE      : strcpy(type , "M");
			break;
		case SIP_INCOMING	  : strcpy(type , "IN");
			break;
		case SIP_OPTION 	  : strcpy(type , "O");
			break;
		case SIP_INVITATION	: strcpy(type , "I");
			break;
		case SIP_ACKNOWLEDGE	: strcpy(type , "A");
			break;
		default:
			break;
	};

	if(*type == '\0'){
		free(res);
		return NULL;
	}

	sprintf(buf, "%s fetion.com.cn SIP-C/4.0\r\n"
			"F: %s\r\n"
			"I: %d\r\n"
			"Q: 2 %s\r\n",
			type,
			sip->from,
			sip->callid,
			type);

	strcat(res , buf);

	pos = sip->header;
	while(pos){
		len = strlen(pos->value) + strlen(pos->name) + 5;
		head = (char*)malloc(len);
		sprintf(head , "%s: %s\r\n" , pos->name , pos->value);
		strcat(res , head);
		tmp = pos;
		pos = pos->next;
		free(head);
		free(tmp->value);
		free(tmp);
	}
	if(body){
		sprintf(buf , "L: %d\r\n\r\n" , strlen(body));
		strcat(res , buf);
		strcat(res , body);
	}else{
		strcat(res , "\r\n");
	}
	callid ++;
	sip->header = NULL;
	return res;
}
void fetion_sip_free(FetionSip* sip)
{
	debug_info("Free sip struct and close socket");
	if(sip != NULL)
		tcp_connection_free(sip->tcp);
	free(sip);
}

char* fetion_sip_get_sid_by_sipuri(const char* sipuri)
{
	char *res , *pos;
	int n;
	pos = strstr(sipuri , ":") + 1;
	n = strlen(pos) - (strstr(pos , "@") == 0 ? 0 : strlen(strstr(pos , "@"))) ;
	res = (char*)malloc(n + 1);
	memset(res , 0 , n + 1);
	strncpy(res , pos , n);
	return res;
}

char* fetion_sip_get_pgid_by_sipuri(const char *pgsipuri)
{
	char *res , *pos;
	int n;
	if(strstr(pgsipuri , "PG") == NULL)
	    return NULL;
	pos = strstr(pgsipuri , "PG") + 2;
	n = strlen(pos) - (strstr(pos , "@") == 0 ? 0 : strlen(strstr(pos , "@"))) ;
	res = (char*)malloc(n + 1);
	memset(res , 0 , n + 1);
	strncpy(res , pos , n);
	return res;
}

int fetion_sip_get_attr(const char* sip , const char* name , char* result)
{
	char m_name[16];
	char* pos;
	int n;

	sprintf(m_name , "%s: " , name);
	if(strstr(sip , m_name) == NULL)
		return -1;
	pos = strstr(sip , m_name) + strlen(m_name);
	if(strstr(pos , "\r\n") == NULL)
		n = strlen(pos);
	else
		n = strlen(pos) - strlen(strstr(pos , "\r\n"));
	strncpy(result , pos , n);
	return 1;
}

int fetion_sip_get_length(const char* sip)
{
	char res[6];
	char name[] = "L";
	memset(res, 0, sizeof(res));
	if(fetion_sip_get_attr(sip , name , res) == -1)
		return 0;
	return atoi(res);
}
int fetion_sip_get_code(const char* sip)
{
	char *pos , res[32];
	int n;

	memset(res, 0, sizeof(res));
	if(strstr(sip , "4.0 ") == NULL)
	    return 400;
	pos = strstr(sip , "4.0 ") + 4;
	if(strstr(pos , " ") == NULL)
	    return 400;
	n = strlen(pos) - strlen(strstr(pos , " "));
	strncpy(res , pos , n);
	return atoi(res);
}
int fetion_sip_get_type(const char* sip)
{
	char res[128];
	int n;

	if(!strstr(sip, " "))
		return SIP_UNKNOWN;

	n = strlen(sip) - strlen(strstr(sip , " "));
	memset(res, 0, sizeof(res));
	strncpy(res , sip , n);
	if(strcmp(res , "I") == 0 )
		return SIP_INVITATION;
	if(strcmp(res , "M") == 0 )
		return SIP_MESSAGE;
	if(strcmp(res , "BN") == 0)
		return SIP_NOTIFICATION;
	if(strcmp(res , "SIP-C/4.0") == 0
		|| strcmp(res , "SIP-C/2.0") == 0)
		return SIP_SIPC_4_0;
	if(strcmp(res , "IN") == 0)
		return SIP_INCOMING;
	if(strcmp(res , "O") == 0 )
		return SIP_OPTION;
	return SIP_UNKNOWN;
		
}
char* fetion_sip_get_response(FetionSip* sip)
{
	char *res;
	int len , n;
   	int c, c1;
	char buf[1024 * 20];

	memset(buf, 0, sizeof(buf));

	if((c = tcp_connection_recv(sip->tcp , buf , sizeof(buf) - 2)) == -1) return (char*)0;

	len = fetion_sip_get_length(buf);

	while(strstr(buf , "\r\n\r\n") == NULL && c < (int)sizeof(buf))
		c += tcp_connection_recv(sip->tcp, buf + c, sizeof(buf) - c - 1);
	

	n = strlen(buf) - strlen(strstr(buf , "\r\n\r\n") + 4);
	len += n;
	if(!(res = (char*)malloc(len + 10))) return (char*)0; 

	memset(res, 0, len + 10);
	strcpy(res, buf);
	if(c < len) {
	   	for(;;) {
			memset(buf, 0, sizeof(buf));
			if((c1 = tcp_connection_recv(sip->tcp , buf
					, len -c < (int)(sizeof(buf) - 1) ? len -c : (int)(sizeof(buf) - 1) ))
				== -1) {free(res); return (char*)0; }

			strncpy(res + c, buf, c1);
			c += c1;
			if(c >= len) {
				break;
			}
		}
	}
	return res;
}
void fetion_sip_set_connection(FetionSip* sip , FetionConnection* conn)
{
	sip->tcp = conn;
}

static SipMsg *sipmsg_new(void)
{
	SipMsg *msg = (SipMsg*)malloc(sizeof(SipMsg));
	memset(msg, 0, sizeof(SipMsg));
	msg->next = NULL;
	return msg;
}

static void sipmsg_set_msg(SipMsg *sipmsg, const char *msg, int n)
{
	sipmsg->message = (char*)malloc(n + 1);
	memset(sipmsg->message, 0, n + 1);
	strncpy(sipmsg->message, msg, n);
}

#define BUFFER_SIZE (1024 * 100)

/* the following code is hard to read,forgive me! */
SipMsg *fetion_sip_listen(FetionSip *sip, int *error)
{
	int     n;
	int     body_len;
	char    buffer[BUFFER_SIZE];
	char    holder[BUFFER_SIZE];
	char   *cur;
	char   *pos;
	SipMsg *list = NULL;
	SipMsg *msg;

	*error = 0;

	memset(buffer, 0, sizeof(buffer));
	n = tcp_connection_recv_dont_wait(sip->tcp,
				buffer, sizeof(buffer) - 1);
	if(n == 0){
		debug_info("fetion_sip_listen 0");
		*error = 1;
		return NULL;
	}
	if(n == -1){
		*error = 1;
		return NULL;
	}
	cur = buffer;
	for(;;){
		pos = strstr(cur, "\r\n\r\n");
		body_len = 0;
		if(pos){
			n = strlen(cur) - strlen(pos);
			memset(holder, 0, sizeof(holder));
			strncpy(holder, cur, n);
			body_len = fetion_sip_get_length(holder);
		}

		if(cur == NULL || *cur == '\0')
			return list;

		if(body_len == 0 && pos){
			msg = sipmsg_new();
			n = strlen(cur) - strlen(pos) + 4;
			sipmsg_set_msg(msg, cur, n);
			if(list)
				fetion_sip_message_append(list, msg);
			else
				list = msg;
			cur += n;
			continue;
		}

		if((body_len == 0 && !pos) ||
				(body_len != 0 && !pos)){
			memset(holder, 0 , sizeof(holder));
			strcpy(holder, cur);
			tcp_connection_recv(sip->tcp,
					holder + strlen(cur),
					BUFFER_SIZE - strlen(cur) - 1);
			memset(buffer, 0 , sizeof(buffer));
			strcpy(buffer, holder);
			cur = buffer;
			continue;
		}else{
			/* now body_len != 0 */
			pos += 4;
			memset(holder, 0 , sizeof(holder));
			if((int)strlen(pos) < body_len){
				strcpy(holder, cur);
				tcp_connection_recv(sip->tcp,
					holder + strlen(cur),
					BUFFER_SIZE - strlen(cur) - 1);
				memset(buffer, 0, sizeof(buffer));
				strcpy(buffer, holder);
				cur = buffer;
				continue;
			}else if((int)strlen(pos) == body_len){
				msg = sipmsg_new();
				sipmsg_set_msg(msg, cur, strlen(cur));
				if(list)
					fetion_sip_message_append(list, msg);
				else
					list = msg;
				return list;
			}else{
				msg = sipmsg_new();
				n = strlen(cur) - strlen(pos) + body_len;
				sipmsg_set_msg(msg, cur, n);
				if(list)
					fetion_sip_message_append(list, msg);
				else
					list = msg;

				memset(holder, 0 , sizeof(holder));
				strcpy(holder, cur + n);
				memset(buffer, 0, sizeof(buffer));
				strcpy(buffer, holder);
				cur = buffer;
				continue;
			}
		}
	}

}
int fetion_sip_keep_alive(FetionSip* sip)
{
	char *res = NULL;
	int ret;

	debug_info("Send a periodical chat keep alive request");

	fetion_sip_set_type(sip , SIP_REGISTER);
	res = fetion_sip_to_string(sip , NULL);
	ret = tcp_connection_send(sip->tcp , res , strlen(res));
	free(res);
	return ret;
}
void fetion_sip_message_free(SipMsg* msg)
{
	SipMsg* pos = msg;
	SipMsg* pot;
	while(pos != NULL)
	{
		pot = pos;
		pos = pos->next;
		if(pot != NULL ){
			free(pot->message);
		}
		free(pot);
		pot = NULL;
	}
}
void fetion_sip_message_append(SipMsg* msglist , SipMsg* msg)
{
	SipMsg* pos = msglist;
	while(pos != NULL)
	{
		if(pos->next == NULL)
		{
			pos->next = msg;
			break;
		}
		pos = pos->next;
	}
}
void fetion_sip_parse_notification(const char* sip , int* type , int* event , char** xml)
{
	char type1[16] , *pos;
	xmlChar *event1;
	xmlDocPtr doc;
	xmlNodePtr node;
	memset(type1, 0, sizeof(type1));
	fetion_sip_get_attr(sip , "N" , type1);
	if(strcmp(type1 , "PresenceV4") == 0)
		*type = NOTIFICATION_TYPE_PRESENCE;
	else if(strcmp(type1 , "Conversation") == 0)
		*type = NOTIFICATION_TYPE_CONVERSATION;
	else if(strcmp(type1 , "contact") == 0)
		*type = NOTIFICATION_TYPE_CONTACT;
	else if(strcmp(type1 , "registration") == 0)
		*type = NOTIFICATION_TYPE_REGISTRATION;
	else if(strcmp(type1 , "SyncUserInfoV4") == 0)
		*type = NOTIFICATION_TYPE_SYNCUSERINFO;
	else if(strcmp(type1 , "PGGroup") == 0)
	    	*type = NOTIFICATION_TYPE_PGGROUP;
	else
		*type = NOTIFICATION_TYPE_UNKNOWN;

	pos = strstr(sip , "\r\n\r\n") + 4;
	*xml = (char*)malloc(strlen(pos) + 1);
	memset(*xml , 0 , strlen(pos) + 1);
	strcpy(*xml , pos);
	doc = xmlReadMemory(*xml , strlen(*xml) , NULL , "UTF-8" , XML_PARSE_RECOVER);
	node = xmlDocGetRootElement(doc);
	node = xml_goto_node(node , "event");
	event1 = xmlGetProp(node ,  BAD_CAST "type");
	if(xmlStrcmp(event1 , BAD_CAST "PresenceChanged") == 0)
		*event = NOTIFICATION_EVENT_PRESENCECHANGED;
	else if(xmlStrcmp(event1 , BAD_CAST "UserLeft") == 0)
		*event = NOTIFICATION_EVENT_USERLEFT;
	else if(xmlStrcmp(event1 , BAD_CAST "deregistered") == 0)
		*event = NOTIFICATION_EVENT_DEREGISTRATION;
	else if(xmlStrcmp(event1 , BAD_CAST "SyncUserInfo") == 0)
		*event = NOTIFICATION_EVENT_SYNCUSERINFO;
	else if(xmlStrcmp(event1 , BAD_CAST "AddBuddyApplication") == 0)
		*event = NOTIFICATION_EVENT_ADDBUDDYAPPLICATION;
	else if(xmlStrcmp(event1 , BAD_CAST "PGGetGroupInfo") == 0)
	    	*event = NOTIFICATION_EVENT_PGGETGROUPINFO;
	else
		*event = NOTIFICATION_EVENT_UNKNOWN;
	xmlFree(event1);
	xmlFreeDoc(doc);
}
void fetion_sip_parse_message(FetionSip* sip , const char* sipmsg , Message** msg)
{
	char len[16] , callid[16] , sequence[16] ;
	char sendtime[32] , from[64] , rep[256];
	char memsipuri[64];

	char *pos = NULL;
	xmlDocPtr doc;
	xmlNodePtr node;
	memset(len , 0 , sizeof(len));
	memset(callid , 0 , sizeof(callid));
	memset(sequence , 0 , sizeof(sequence));
	memset(sendtime , 0 , sizeof(sendtime));
	memset(from , 0 , sizeof(from));
	fetion_sip_get_attr(sipmsg , "F" , from);
	fetion_sip_get_attr(sipmsg , "L" , len);
	fetion_sip_get_attr(sipmsg , "I" , callid);
	fetion_sip_get_attr(sipmsg , "Q" , sequence);
	fetion_sip_get_attr(sipmsg , "D" , sendtime);	

	*msg = fetion_message_new();

	(*msg)->sysback = 0;
	if(strstr(sipmsg, "SIP-C/3.0") &&
		!strstr(sipmsg, "SIP-C/4.0"))
		(*msg)->sysback = 1;

	/* group message */
	if(strstr(from , "PG") != NULL){
	    fetion_message_set_pguri(*msg , from);
	    memset(memsipuri , 0 , sizeof(memsipuri));
	    fetion_sip_get_attr(sipmsg , "SO" , memsipuri);
	    fetion_message_set_sipuri(*msg , memsipuri);
	}else{
	    fetion_message_set_sipuri(*msg , from);
	}

	pos = strstr(sipmsg , "\r\n\r\n") + 4;
	doc = xmlReadMemory(pos , strlen(pos) , "UTF-8" , NULL , XML_PARSE_NOERROR);
	fetion_message_set_time(*msg , convert_date(sendtime));

	if(doc != NULL){
		node = xmlDocGetRootElement(doc);
		pos = (char*)xmlNodeGetContent(node);
		fetion_message_set_message(*msg , pos);
		free(pos);
		xmlFreeDoc(doc);
	}else{
		fetion_message_set_message(*msg , pos);
	}

	memset(rep, 0, sizeof(rep));
	if(strstr(from , "PG") == NULL)
	    sprintf(rep ,"SIP-C/4.0 200 OK\r\nF: %s\r\nI: %s\r\nQ: %s\r\n\r\n"
				    , from , callid , sequence );
	else
	    sprintf(rep ,"SIP-C/4.0 200 OK\r\nI: %s\r\nQ: %s\r\nF: %s\r\n\r\n"
				    , callid , sequence , from);

	tcp_connection_send(sip->tcp , rep , strlen(rep));
}
void fetion_sip_parse_invitation(FetionSip* sip , Proxy *proxy , const char* sipmsg
		, FetionSip** conversionSip , char** sipuri)
{
	char from[50];
	char auth[128];
	char* ipaddress = NULL;
	char buf[1024];
	int port;
	char* credential = NULL;
	FetionConnection* conn = NULL;
	SipHeader* aheader = NULL;
	SipHeader* theader = NULL;
	SipHeader* mheader = NULL;
	SipHeader* nheader = NULL;
	char* sipres = NULL;

	memset(from, 0, sizeof(from));
	memset(auth, 0, sizeof(auth));
	memset(buf, 0, sizeof(buf));
	fetion_sip_get_attr(sipmsg , "F" , from);
	fetion_sip_get_attr(sipmsg , "A" , auth);
	fetion_sip_get_auth_attr(auth , &ipaddress , &port , &credential);
	conn = tcp_connection_new();

	if(proxy != NULL && proxy->proxyEnabled)
		tcp_connection_connect_with_proxy(conn , ipaddress , port , proxy);
	else {
		int ret = tcp_connection_connect(conn , ipaddress , port);
		if(ret == -1)
			ret = tcp_connection_connect(conn , ipaddress , 443);
		if(ret == -1) {
			debug_error("Connect to server failed: %s:%d/%s:%d", ipaddress, port, ipaddress, 443);
			return;
		}
	}

	*conversionSip = fetion_sip_clone(sip);
	fetion_sip_set_connection(*conversionSip , conn);
	debug_info("Received a conversation invitation");
	sprintf(buf , "SIP-C/4.0 200 OK\r\nF: %s\r\nI: -61\r\nQ: 200002 I\r\n\r\n"
				, from);
	*sipuri = (char*)malloc(48);
	memset(*sipuri , 0 , 48);
	strcpy(*sipuri , from);
	tcp_connection_send(sip->tcp , buf , strlen(buf));

	fetion_sip_set_type(sip , SIP_REGISTER);
	aheader = fetion_sip_credential_header_new(credential);
	theader = fetion_sip_header_new("K" , "text/html-fragment");
	mheader = fetion_sip_header_new("K" , "multiparty");
	nheader = fetion_sip_header_new("K" , "nudge");
	fetion_sip_add_header(sip , aheader);
	fetion_sip_add_header(sip , theader);
	fetion_sip_add_header(sip , mheader);
	fetion_sip_add_header(sip , nheader);
	sipres = fetion_sip_to_string(sip , NULL);
	debug_info("Register to conversation server %s:%d" , ipaddress , port);
	tcp_connection_send(conn , sipres , strlen(sipres));
	free(sipres);
	free(ipaddress);
	memset(buf , 0 , sizeof(buf));
	port = tcp_connection_recv(conn , buf , sizeof(buf));

	memset((*conversionSip)->sipuri, 0, sizeof((*conversionSip)->sipuri));
	strcpy((*conversionSip)->sipuri , *sipuri);

}
void fetion_sip_parse_addbuddyapplication(const char* sipmsg
		, char** sipuri , char** userid , char** desc , int* phrase)
{
	char *pos = NULL;
	xmlDocPtr doc;
	xmlNodePtr node;
	xmlChar *res = NULL;
	pos = strstr(sipmsg , "\r\n\r\n") + 4;
	doc = xmlReadMemory(pos , strlen(pos) , NULL , "UTF-8" , XML_PARSE_RECOVER);
	node = xmlDocGetRootElement(doc);
	node = xml_goto_node(node , "application");

	res = xmlGetProp(node , BAD_CAST "uri");
	*sipuri = (char*)malloc(strlen((char*)res) + 1);
	memset(*sipuri, 0, strlen((char*)res) + 1);
	strcpy(*sipuri , (char*)res);
	xmlFree(res);

	res = xmlGetProp(node , BAD_CAST "user-id");
	*userid = (char*)malloc(strlen((char*)res) + 1);
	memset(*userid, 0, strlen((char*)res) + 1);
	strcpy(*userid , (char*)res);
	xmlFree(res);

	res = xmlGetProp(node , BAD_CAST "desc");
	*desc = (char*)malloc(xmlStrlen(res) + 1);
	memset(*desc, 0, xmlStrlen(res) + 1);
	strcpy(*desc , (char*)res);
	xmlFree(res);

	res = xmlGetProp(node , BAD_CAST "addbuddy-phrase-id");
	*phrase = atoi((char*)res);
	xmlFree(res);

	xmlFreeDoc(doc);

}

void fetion_sip_parse_incoming(FetionSip* sip
		, const char* sipmsg , char** sipuri
		, IncomingType* type , IncomingActionType *action)
{
	char *pos = NULL;
	xmlDocPtr doc = NULL;
	xmlNodePtr node = NULL;
	xmlChar *res = NULL;
	char replyMsg[128];
	char callid[10];
	char seq[10];

	pos = strstr(sipmsg , "\r\n\r\n") + 4;
	doc = xmlParseMemory(pos , strlen(pos));
	node = xmlDocGetRootElement(doc);
	if(xmlStrcmp(node->name , BAD_CAST "share-content") == 0){
		debug_info("Received a share-content IN message");
		*sipuri = (char*)malloc(48);
		memset(*sipuri, 0, 48);
		fetion_sip_get_attr(sipmsg , "F" , *sipuri);
		*type = INCOMING_SHARE_CONTENT;
		if(! xmlHasProp(node , BAD_CAST "action")){
			*action = INCOMING_ACTION_UNKNOWN;
			xmlFreeDoc(doc);
			return;
		}
		res = xmlGetProp(node , BAD_CAST "action");
		if(xmlStrcmp(res , BAD_CAST "accept") == 0){
			*action = INCOMING_ACTION_ACCEPT;
		} else if( xmlStrcmp(res , BAD_CAST "cancel") == 0){
			*action = INCOMING_ACTION_CANCEL;
		} else {
			*action = INCOMING_ACTION_UNKNOWN;
		}
		xmlFree(res);
		xmlFreeDoc(doc);
		return;
	}
	if(xmlStrcmp(node->name , BAD_CAST "is-composing") != 0){
		debug_info("Received a unhandled sip message , thanks for sending it to the author");
		*type = INCOMING_UNKNOWN;
		xmlFreeDoc(doc);
		return;
	}
	node = node->xmlChildrenNode;
	res = xmlNodeGetContent(node);
	if(xmlStrcmp(res, BAD_CAST "nudge") == 0 ||
		xmlStrcmp(res, BAD_CAST "input") == 0) {
		*type = INCOMING_UNKNOWN;
		*sipuri = (char*)malloc(50);
		memset(replyMsg, 0, sizeof(replyMsg));
		memset(callid, 0, sizeof(callid));
		memset(seq, 0, sizeof(seq));
		memset(*sipuri, 0, 50);

		fetion_sip_get_attr(sipmsg , "I" , callid);
		fetion_sip_get_attr(sipmsg , "Q" , seq);
		fetion_sip_get_attr(sipmsg , "F" , *sipuri);
		sprintf(replyMsg , "SIP-C/4.0 200 OK\r\n"
						   "F: %s\r\n"
						   "I: %s \r\n"
						   "Q: %s\r\n\r\n"
						 , *sipuri , callid , seq);
		tcp_connection_send(sip->tcp , replyMsg , strlen(replyMsg));
		if(xmlStrcmp(res, BAD_CAST "nudge") == 0)
			*type = INCOMING_NUDGE;
		else
			*type = INCOMING_INPUT;
	}
	xmlFree(res);
	xmlFreeDoc(doc);
}

void fetion_sip_parse_userleft(const char* sipmsg , char** sipuri)
{
	char *pos = NULL;
	xmlDocPtr doc = NULL;
	xmlNodePtr node = NULL;
	xmlChar *res;

	pos = strstr(sipmsg , "\r\n\r\n") + 4;
	doc = xmlParseMemory(pos , strlen(pos));
	node = xmlDocGetRootElement(doc);
	node = xml_goto_node(node , "member");
	res = xmlGetProp(node , BAD_CAST "uri");
	*sipuri = (char*)malloc(xmlStrlen(res) + 1);
	memset(*sipuri, 0, xmlStrlen(res) + 1);
	strcpy(*sipuri , (char*)res);
	xmlFreeDoc(doc);
}

static char *generate_action_accept_body(Share *share)
{
	xmlChar *buf = NULL;
	xmlDocPtr doc;
	xmlNodePtr node , root;
	char body[] = "<share-content></share-content>";

	doc = xmlParseMemory(body , strlen(body));
	root = xmlDocGetRootElement(doc);
	node = xmlNewChild(root , NULL , BAD_CAST "client" , NULL);
	xmlNewProp(node , BAD_CAST "prefer-types" , BAD_CAST share->preferType);
	printf("%s\n" , hexip_to_dotip("3B408066"));
	xmlNewProp(node , BAD_CAST "inner-ip" , BAD_CAST "3B408066");
	xmlNewProp(node , BAD_CAST "net-type" , BAD_CAST "0");
	xmlNewProp(node , BAD_CAST "tcp-port" , BAD_CAST "443");
	xmlDocDumpMemory(doc , &buf , NULL);
	xmlFreeDoc(doc);
	return xml_convert(buf);
}

int fetion_sip_parse_shareaccept(FetionSip *sip 
		, const char* sipmsg , Share *share)
{
	xmlDocPtr doc;
	xmlNodePtr node;
	xmlChar *res;
	char callid[16];
	char from[48];
	char seq[16];
	char response[1024];
	char *pos;

	memset(callid, 0, sizeof(callid));
	memset(from, 0, sizeof(from));
	memset(seq, 0, sizeof(seq));
	fetion_sip_get_attr(sipmsg , "I" , callid);
	fetion_sip_get_attr(sipmsg , "F" , from);
	fetion_sip_get_attr(sipmsg , "Q" , seq);
	
	pos = strstr(sipmsg , "\r\n\r\n") + 4;
	doc = xmlReadMemory(pos , strlen(pos) , NULL , "UTF-8" , XML_PARSE_RECOVER );
	node = xmlDocGetRootElement(doc);

	node = node->xmlChildrenNode->next;
	if(xmlStrcmp(node->name , BAD_CAST "client") != 0)
		return -1;
	
	res = xmlGetProp(node , BAD_CAST "prefer-types");
	strcpy(share->preferType , (char*)res);
	xmlFree(res);
	
	res = xmlGetProp(node , BAD_CAST "inner-ip");
	pos = hexip_to_dotip((char*)res);
	xmlFree(res);
	strcpy(share->outerIp , pos);
	free(pos);
	pos = NULL;

	res = xmlGetProp(node , BAD_CAST "udp-inner-port");
	share->outerUdpPort = atoi((char*)res);
	xmlFree(res);

	res = xmlGetProp(node , BAD_CAST "tcp-port");
	share->outerTcpPort = atoi((char*)res);
	xmlFree(res);

	pos = generate_action_accept_body(share);
	memset(response, 0, sizeof(response));
	sprintf(response , "SIP-C/4.0 200 OK\r\n"
					   "F: %s\r\n"
					   "I: %s\r\n"
					   "Q: %s\r\n"
					   "L: %d\r\n\r\n%s"
					 , from , callid , seq , strlen(pos) , pos);
	free(pos);
	pos = NULL;

	printf("%s\n" , response);
	tcp_connection_send(sip->tcp , response , strlen(response) );
	
	return 1;
}

void fetion_sip_parse_sysmsg(const char* sipmsg , int *type
		, int* showonce , char **content , char **url){

	char *pos;
	xmlChar *res;
	xmlDocPtr doc;
	xmlNodePtr node;

	pos = strstr(sipmsg , "\r\n\r\n") + 4;
	doc = xmlReadMemory(pos , strlen(pos)
			, NULL , "UTF-8" , XML_PARSE_RECOVER);

	node = xmlDocGetRootElement(doc);
	res = xmlGetProp(node , BAD_CAST "type");
	*type = atoi((char*)type);
	xmlFree(res);
	res = xmlGetProp(node , BAD_CAST "show-once");
	*showonce = atoi((char*)res);
	xmlFree(res);
	node = node->xmlChildrenNode->next->next->next;
	res = xmlNodeGetContent(node);
	*content = (char*)malloc(xmlStrlen(res) + 1);
	memset(*content, 0, xmlStrlen(res) + 1);
	strcpy(*content , (char*)res);
	xmlFree(res);
	node = node->next->next;
	res = xmlNodeGetContent(node);
	*url = (char*)malloc(xmlStrlen(res) + 1);
	memset(*url, 0, xmlStrlen(res) + 1);
	strcpy(*url , (char*)res);
	xmlFree(res);

}

int fetion_sip_parse_sipc(const char *sipmsg , int *callid , char **xml)
{
	char callid_str[16];
	char *pos;
	int n;
	char code[5];

	pos = strstr(sipmsg , " ") + 1;
	n = strlen(pos) - strlen(strstr(pos , " "));
	strncpy(code , pos , n);
	
	fetion_sip_get_attr(sipmsg , "I" , callid_str);
	*callid = atoi(callid_str);
	
	pos = strstr(sipmsg , "\r\n\r\n");

	if(pos)
	    pos += 4;
	else{
	    *xml = NULL;
	    return -1;
	}
	
	*xml = (char*)malloc(strlen(pos) + 1);
	memset(*xml , 0 , strlen(pos) + 1);
	strcpy(*xml , pos);

	return atoi(code);

}

struct tm convert_date(const char* date)
{
	char* pos = strstr(date , ",") + 2;
	struct tm dstr;

	strptime(pos , "%d %b %Y %T %Z" , &dstr);

	dstr.tm_hour += 8;
	if(dstr.tm_hour > 23)
		dstr.tm_hour -= 24;
	return dstr;
	
}
void fetion_sip_get_auth_attr(const char* auth , char** ipaddress , int* port , char** credential)
{
	char* pos = strstr(auth , "address=\"") + 9;
	int n = strlen(pos) - strlen(strstr(pos , ":"));
	char port_str[6] = { 0 };
	*credential = (char*)malloc(256);
	memset(*credential , 0 , 256);
	*ipaddress = (char*)malloc(256);
	memset(*ipaddress , 0 , 256);
	strncpy(*ipaddress , pos , n);
	pos = strstr(pos , ":") + 1;
	n = strlen(pos) - strlen(strstr(pos , ";"));
	strncpy(port_str , pos , n);
	*port = atoi(port_str);
	pos = strstr(pos , "credential=\"") + 12;
	strncpy(*credential , pos , strlen(pos) - 1);
}

inline void 
fetion_sip_set_conn(FetionSip *sip, FetionConnection *conn)
{
	sip->tcp = conn;
}

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

extern char* generate_invite_friend_body(const char* sipuri);
extern char* generate_send_nudge_body();
extern void fetion_conversation_parse_send_sms(const char* xml , int* daycount , int* mountcount);

extern struct unacked_list *unackedlist;

Conversation* fetion_conversation_new(User* user,
			  const char* sipuri , FetionSip* sip)
{
	Conversation* conversation = (Conversation*)malloc(sizeof(Conversation));
	memset(conversation , 0 , sizeof(Conversation));
	conversation->currentUser = user;
	if(sipuri != NULL)
		conversation->currentContact = 
				fetion_contact_list_find_by_sipuri(user->contactList , sipuri);
	else
		conversation->currentContact = NULL;
	if(sipuri != NULL && conversation->currentContact == NULL){
		free(conversation);
		return NULL;
	}
	conversation->currentSip = sip;
	return conversation;
}

int fetion_conversation_send_sms(Conversation* conversation , const char* msg)
{
	FetionSip* sip = conversation->currentSip == NULL ?
		   	conversation->currentUser->sip : conversation->currentSip;
	SipHeader *toheader , *cheader , *kheader , *nheader;
	Message *message;
	struct unacked_list *unacked;
	char* res;
	struct tm *now;
	struct tm now_copy;

	fetion_sip_set_type(sip , SIP_MESSAGE);
	nheader  = fetion_sip_event_header_new(SIP_EVENT_CATMESSAGE);
	toheader = fetion_sip_header_new("T" , conversation->currentContact->sipuri);
	cheader  = fetion_sip_header_new("C" , "text/plain");
	kheader  = fetion_sip_header_new("K" , "SaveHistory");
	fetion_sip_add_header(sip , toheader);
	fetion_sip_add_header(sip , cheader);
	fetion_sip_add_header(sip , kheader);
	fetion_sip_add_header(sip , nheader);
	/* add message to list */
	now = get_currenttime();
	now_copy = *now;
	message = fetion_message_new();
	fetion_message_set_sipuri(message , conversation->currentContact->sipuri);
	fetion_message_set_time(message , now_copy);
	fetion_message_set_message(message , msg);
	fetion_message_set_callid(message , sip->callid);
	unacked = unacked_list_new(message);
	unacked_list_append(unackedlist , unacked);

	res = fetion_sip_to_string(sip , msg);
	debug_info("Sent a message to %s" , conversation->currentContact->sipuri);
	if(tcp_connection_send(sip->tcp , res , strlen(res)) == -1){
		free(res);
		return -1;
	}
	free(res);
	return 1;
}

int fetion_conversation_send_sms_with_reply(Conversation *conv, const char *msg)
{
	char       rep[1024];

	FetionSip* sip = conv->currentSip == NULL ?
		   	conv->currentUser->sip : conv->currentSip;
	SipHeader *toheader , *cheader , *kheader , *nheader;
	Message *message;
	char* res;
	struct tm *now;
	struct tm now_copy;

	fetion_sip_set_type(sip , SIP_MESSAGE);
	nheader  = fetion_sip_event_header_new(SIP_EVENT_CATMESSAGE);
	toheader = fetion_sip_header_new("T" , conv->currentContact->sipuri);
	cheader  = fetion_sip_header_new("C" , "text/plain");
	kheader  = fetion_sip_header_new("K" , "SaveHistory");
	fetion_sip_add_header(sip , toheader);
	fetion_sip_add_header(sip , cheader);
	fetion_sip_add_header(sip , kheader);
	fetion_sip_add_header(sip , nheader);
	/* add message to list */
	now = get_currenttime();
	now_copy = *now;
	message = fetion_message_new();
	fetion_message_set_sipuri(message , conv->currentContact->sipuri);
	fetion_message_set_time(message , now_copy);
	fetion_message_set_message(message , msg);
	fetion_message_set_callid(message , sip->callid);

	res = fetion_sip_to_string(sip , msg);
	debug_info("Sent a message to %s" , conv->currentContact->sipuri);
	tcp_connection_send(sip->tcp , res , strlen(res));
	free(res);

	memset(rep , 0 , sizeof(rep));
	tcp_connection_recv(sip->tcp , rep , sizeof(rep));

	if(fetion_sip_get_code(rep) == 280 || fetion_sip_get_code(rep) == 200){
		return 1;
	}else{
		return -1;
	}
}

int fetion_conversation_send_sms_to_myself(Conversation* conversation,
			   	const char* message)
{
	SipHeader *toheader = NULL;
	SipHeader *eheader = NULL;
	char* res = NULL;
	FetionSip* sip = conversation->currentUser->sip;

	fetion_sip_set_type(sip , SIP_MESSAGE);
	toheader = fetion_sip_header_new("T" , conversation->currentUser->sipuri);
	eheader  = fetion_sip_event_header_new(SIP_EVENT_SENDCATMESSAGE);
	fetion_sip_add_header(sip , toheader);
	fetion_sip_add_header(sip , eheader);
	res = fetion_sip_to_string(sip , message);
	debug_info("Sent a message to myself" , conversation->currentContact->sipuri);
	if(tcp_connection_send(sip->tcp , res , strlen(res)) == -1) {
		free(res);
		return -1;
	}
	free(res);
	res = fetion_sip_get_response(sip);
	free(res);
	return 1;
}

int fetion_conversation_send_sms_to_myself_with_reply(Conversation* conversation,
			   	const char* message)
{
	SipHeader *toheader = NULL;
	SipHeader *eheader = NULL;
	char       *res = NULL;
	char        rep[1024];
	int         code;
	FetionSip  *sip = conversation->currentUser->sip;

	fetion_sip_set_type(sip , SIP_MESSAGE);
	toheader = fetion_sip_header_new("T" , conversation->currentUser->sipuri);
	eheader  = fetion_sip_event_header_new(SIP_EVENT_SENDCATMESSAGE);
	fetion_sip_add_header(sip , toheader);
	fetion_sip_add_header(sip , eheader);
	res = fetion_sip_to_string(sip , message);
	debug_info("Sent a message to myself" , conversation->currentContact->sipuri);
	tcp_connection_send(sip->tcp , res , strlen(res));
	free(res);
	memset(rep, 0, sizeof(rep));
	tcp_connection_recv(sip->tcp , rep , sizeof(rep));
	code = fetion_sip_get_code(rep);
	if(code == 200 || code == 280){
		return 1;
	}else{
		return -1;
	}
}

int fetion_conversation_send_sms_to_phone(Conversation* conversation,
			   	const char* message)
{
	
	SipHeader *toheader = NULL;
	SipHeader *eheader = NULL;
	SipHeader *aheader = NULL;
	User *user = conversation->currentUser;
	char* res = NULL;
	FetionSip* sip = user->sip;
	char* sipuri = conversation->currentContact->sipuri;
	char astr[256] , rep[1024];
	int code;

	fetion_sip_set_type(sip , SIP_MESSAGE);
	toheader = fetion_sip_header_new("T" , sipuri);
	eheader  = fetion_sip_event_header_new(SIP_EVENT_SENDCATMESSAGE);
	fetion_sip_add_header(sip , toheader);
	if(user->verification != NULL){
		memset(astr, 0, sizeof(astr));
		sprintf(astr , "Verify algorithm=\"picc\",chid=\"%s\",response=\"%s\""
				, user->verification->guid
				, user->verification->code);
		aheader = fetion_sip_header_new("A" , astr);
		fetion_sip_add_header(sip , aheader);
	}
	fetion_sip_add_header(sip , eheader);
	res = fetion_sip_to_string(sip , message);
	debug_info("Sent a message to (%s)`s mobile phone" , sipuri);
	tcp_connection_send(sip->tcp , res , strlen(res));
	free(res);
	memset(rep, 0, sizeof(rep));
	tcp_connection_recv(sip->tcp , rep , sizeof(rep));
	code = fetion_sip_get_code(rep);
	if(code == 420 || code == 421){
		return -1;
	}else{
		return 1;
	}
}
int fetion_conversation_send_sms_to_phone_with_reply(Conversation* conversation
		, const char* message , int* daycount , int* monthcount)
{
	
	SipHeader *toheader , *eheader , *aheader;
	char* res;
	char* xml;
	User *user = conversation->currentUser;
	FetionSip* sip = user->sip;
	char astr[256] , rep[1024];
	char* sipuri = conversation->currentContact->sipuri;

	fetion_sip_set_type(sip , SIP_MESSAGE);
	toheader = fetion_sip_header_new("T" , sipuri);
	eheader  = fetion_sip_event_header_new(SIP_EVENT_SENDCATMESSAGE);
	fetion_sip_add_header(sip , toheader);
	if(user->verification != NULL){
		sprintf(astr , "Verify algorithm=\"picc\",chid=\"%s\",response=\"%s\""
				, user->verification->guid
				, user->verification->code);
		aheader = fetion_sip_header_new("A" , astr);
		fetion_sip_add_header(sip , aheader);
	}
	fetion_sip_add_header(sip , eheader);
	res = fetion_sip_to_string(sip , message);
	debug_info("Sent a message to (%s)`s mobile phone" , sipuri);
	tcp_connection_send(sip->tcp , res , strlen(res));
	free(res);

	memset(rep , 0 , sizeof(rep));
	tcp_connection_recv(sip->tcp , rep , sizeof(rep));

	if(fetion_sip_get_code(rep) == 280){
		xml = strstr(rep , "\r\n\r\n") + 4;
		fetion_conversation_parse_send_sms(xml , daycount , monthcount);
		return 1;
	}else{
		debug_error("Send a message to (%s)`s mobile phone failed",
				sipuri);
		return -1;
	}
}
int fetion_conversation_invite_friend(Conversation* conversation)
{
	FetionSip* sip = conversation->currentUser->sip;
	char *res , *ip , *credential , auth[256] , *body;
	int port , ret;
	FetionConnection* conn;
	Proxy *proxy = conversation->currentUser->config->proxy;
	SipHeader *eheader , *theader , *mheader , *nheader , *aheader;


	/*start chat*/
	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_STARTCHAT);
	fetion_sip_add_header(sip , eheader);
	res = fetion_sip_to_string(sip , NULL);
	tcp_connection_send(sip->tcp , res , strlen(res));
	free(res); res = NULL;
	res = fetion_sip_get_response(sip);
	if(!res)
		return -1;

	memset(auth , 0 , sizeof(auth));
	fetion_sip_get_attr(res , "A" , auth);
	if(auth==NULL)
		return -1;

	fetion_sip_get_auth_attr(auth , &ip , &port , &credential);
	free(res); res = NULL;
	conn = tcp_connection_new();

	if(proxy != NULL && proxy->proxyEnabled)
		ret = tcp_connection_connect_with_proxy(conn, ip, port, proxy);
	else {
		ret = tcp_connection_connect(conn, ip, port);
		if(ret == -1)
			ret = tcp_connection_connect(conn, ip, 443);
	}

	if(ret == -1)
		return -1;

	/*clone sip*/
	conversation->currentSip = fetion_sip_clone(conversation->currentUser->sip);
	memset(conversation->currentSip->sipuri, 0 , sizeof(conversation->currentSip->sipuri));
	strcpy(conversation->currentSip->sipuri , conversation->currentContact->sipuri);
	fetion_sip_set_connection(conversation->currentSip , conn);
	free(ip); ip = NULL;
	/*register*/
	sip = conversation->currentSip;
	fetion_sip_set_type(sip , SIP_REGISTER);
	aheader = fetion_sip_credential_header_new(credential);
	theader = fetion_sip_header_new("K" , "text/html-fragment");
	mheader = fetion_sip_header_new("K" , "multiparty");
	nheader = fetion_sip_header_new("K" , "nudge");
	fetion_sip_add_header(sip , aheader);
	fetion_sip_add_header(sip , theader);
	fetion_sip_add_header(sip , mheader);
	fetion_sip_add_header(sip , nheader);
	res = fetion_sip_to_string(sip , NULL);
	tcp_connection_send(conn , res , strlen(res));
	free(res);res = NULL;
	free(credential); credential = NULL;
	res = fetion_sip_get_response(sip);
	free(res); res = NULL;
	/*invite buddy*/
	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_INVITEBUDDY);
	fetion_sip_add_header(sip , eheader);
	body = generate_invite_friend_body(conversation->currentContact->sipuri);
	res = fetion_sip_to_string(sip , body);	
	free(body); body = NULL;
	tcp_connection_send(sip->tcp , res , strlen(res));
	free(res); res = NULL;
	res = fetion_sip_get_response(sip);

	if(fetion_sip_get_code(res) == 200)	{
		free(res);
		char lastbuf[2048];
		tcp_connection_recv(sip->tcp, lastbuf, sizeof(lastbuf));
		return 1;
	}else{
		free(res);
		return -1;
	}
}
int fetion_conversation_send_nudge(Conversation* conversation)
{
	SipHeader *toheader = NULL;
	char* res = NULL;
	char* body = NULL;
	FetionSip* sip = conversation->currentSip;
	if(sip == NULL)
	{
		debug_error("Did not start a chat chanel , can not send a nudge");
		return -1;
	}
	char* sipuri = conversation->currentContact->sipuri;
	fetion_sip_set_type(sip , SIP_INCOMING);
	toheader = fetion_sip_header_new("T" , sipuri);
	fetion_sip_add_header(sip , toheader);
	body = generate_send_nudge_body();
	res = fetion_sip_to_string(sip , body);
	free(body);
	debug_info("Sent a nudge to (%s)" , sipuri);
	tcp_connection_send(sip->tcp , res , strlen(res));
	free(res);
/*	res = fetion_sip_get_response(sip);
	if(fetion_sip_get_code(res) == 280)
	{
		free(res);
		return 1;
	}
	else
	{
		printf("%s\n" , res);
		free(res);
		debug_error("Send nuge failed");
		return -1;
	}*/
	return 1;

}
char* generate_invite_friend_body(const char* sipuri)
{
	xmlChar *buf;
	xmlDocPtr doc;
	xmlNodePtr node;
	char body[] = "<args></args>";
	doc = xmlParseMemory(body , strlen(body));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "contacts" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "contact" , NULL);
	xmlNewProp(node , BAD_CAST "uri" , BAD_CAST sipuri);
	xmlDocDumpMemory(doc , &buf , NULL);
	xmlFreeDoc(doc);
	return xml_convert(buf);
}
char* generate_send_nudge_body()
{
	xmlChar *buf;
	xmlDocPtr doc;
	xmlNodePtr node;
	char body[] = "<is-composing></is-composing>";
	doc = xmlParseMemory(body , strlen(body));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "state" , NULL);
	xmlNodeSetContent(node , BAD_CAST "nudge");
	xmlDocDumpMemory(doc , &buf , NULL);
	xmlFreeDoc(doc);
	return xml_convert(buf);
}
void fetion_conversation_parse_send_sms(const char* xml , int* daycount , int* mountcount)
{
	xmlDocPtr doc;
	xmlNodePtr node;
	xmlChar* res;
	doc = xmlParseMemory(xml , strlen(xml));
	node = xmlDocGetRootElement(doc);
	node = xml_goto_node(node , "frequency");
	res = xmlGetProp(node , BAD_CAST "day-count");
	*daycount = atoi((char*)res);
	xmlFree(res);
	res = xmlGetProp(node , BAD_CAST "month-count");
	*mountcount = atoi((char*)res);
	xmlFree(res);
	xmlFreeDoc(doc);
}

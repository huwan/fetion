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

static void 
parse_option_verification(User *user , const char *in)
{
	int n;
	char *pos , w[256];
	xmlDocPtr doc;
	xmlNodePtr node;
	xmlChar *res;

	user->verification = fetion_verification_new();
	memset(w, 0, sizeof(w));
	fetion_sip_get_attr(in , "W" , w);
	pos = strstr(w , "thm=\"") + 5;
	n = strlen(pos) - strlen(strstr(pos , "\""));
	user->verification->algorithm = (char*)malloc(n + 1);
	memset(user->verification->algorithm, 0, n + 1);
	strncpy(user->verification->algorithm , pos , n);
	pos = strstr(pos , "type=\"") + 6;
	n = strlen(pos) - strlen(strstr(pos , "\""));
	user->verification->type = (char*)malloc(n + 1);
	memset(user->verification->type, 0, n + 1);
	strncpy(user->verification->type , pos , n);

	pos = strstr(in , "\r\n\r\n") + 4;
	doc = xmlReadMemory(pos , strlen(pos)
			, NULL , "UTF-8" , XML_PARSE_RECOVER);
	node = xmlDocGetRootElement(doc);
	node = node->xmlChildrenNode;
	if(xmlHasProp(node , BAD_CAST "text")){
		res = xmlGetProp(node , BAD_CAST "text");
		user->verification->text = (char*)malloc(xmlStrlen(res) + 1);
		memset(user->verification->text, 0, xmlStrlen(res) + 1);
		strcpy(user->verification->text , (char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "tips")){
		res = xmlGetProp(node , BAD_CAST "tips");
		if(strstr((char*)res , "，<a")){
			n = xmlStrlen(res) - strlen(strstr((char*)res , "，<a"));
			user->verification->tips = (char*)malloc(n + 1);
			memset(user->verification->tips, 0, n + 1);
			strncpy(user->verification->tips , (char*)res , n);
		}
		xmlFree(res);
	}
}

int fetion_directsms_send_option(User *user , const char *response)
{
	FetionSip *sip = user->sip;
	SipHeader *eheader , *aheader;
	int code;
	char *res , atext[1024];
	
	fetion_sip_set_type(sip , SIP_OPTION);
	eheader = fetion_sip_event_header_new(SIP_EVENT_DIRECTSMS);	
	fetion_sip_add_header(sip , eheader);
	if(user->verification != NULL && response != NULL){
		memset(atext, 0, sizeof(atext));
		sprintf(atext , "Verify algorithm=\"%s\","
				"type=\"%s\",response=\"%s\",chid=\"%s\""
				, user->verification->algorithm
				, user->verification->type
				, response
				, user->verification->guid);
		aheader = fetion_sip_header_new("A" , atext);
		fetion_sip_add_header(sip , aheader);
	}
	res = fetion_sip_to_string(sip , NULL);
	tcp_connection_send(sip->tcp , res , strlen(res));
	free(res);
	res = fetion_sip_get_response(sip);
	code = fetion_sip_get_code(res);
	if(code == 200){
		return DSMS_OPTION_SUCCESS;
	}else{
		parse_option_verification(user , res);
		return DSMS_OPTION_FAILED;
	}
	free(res);
}

static int 
parse_subscribe_response(const char *in , char **error)
{
	char *pos , c[4];
	int n;
	xmlDocPtr doc;
	xmlNodePtr node;
	xmlChar *res;

	pos = strstr(in , " ") + 1;
	n = strlen(pos) - strlen(strstr(pos , " "));
	memset(c, 0, sizeof(c));
	strncpy(c , pos , n);
	if(strcmp(c , "200") == 0){
		*error = NULL;
		return PIC_SUCCESS;
	}
	pos = strstr(in , "\r\n\r\n") + 4;
	doc = xmlReadMemory(pos , strlen(pos)
			, NULL , "UTF-8" , XML_PARSE_RECOVER );
	node = xmlDocGetRootElement(doc);
	node = node->xmlChildrenNode;
	if(xmlStrcmp(node->name , BAD_CAST "error") == 0){
		if(xmlHasProp(node , BAD_CAST "user-msg")){
			res = xmlGetProp(node , BAD_CAST "user-msg");
			*error = (char*)malloc(xmlStrlen(res) + 1);
			strcpy(*error , (char*)res);
			xmlFree(res);
			return PIC_ERROR;
		}else{
			*error = NULL;
			return UNKNOW_ERROR;
		}
	}else{
		*error = NULL;
		return UNKNOW_ERROR;
	}
}

int fetion_directsms_send_subscribe(User *user , const char *code , char **error)
{
	char body[256];
	char http[2048];
	char *ip;
	FetionConnection *tcp;

	ip = get_ip_by_name(NAVIGATION_URI);
	memset(body, 0, sizeof(body));
	sprintf(body , "PicCertSessionId=%s&PicCertCode=%s&MobileNo=%s"
			, user->verification->guid , code , user->mobileno);

	memset(http, 0, sizeof(http));
	sprintf(http , "POST /nav/ApplySubscribe.aspx HTTP/1.1\r\n"
				   "Cookie: ssic=%s\r\n"
				   "Accept: */*\r\n"
				   "Host: %s\r\n"
				   "Content-Length: %d\r\n"
				   "Content-Type: application/x-www-form-urlencoded;"
				   "charset=utf-8\r\n"
				   "User-Agent: IIC2.0/PC "PROTO_VERSION"\r\n"
				   "Connection: Keep-Alive\r\n"
				   "Cache-Control: no-cache\r\n\r\n%s"
				 , user->ssic , NAVIGATION_URI , strlen(body) , body);

	printf("%s\n" , http);
	tcp = tcp_connection_new();
	tcp_connection_connect(tcp , ip , 80);
	tcp_connection_send(tcp , http , strlen(http));
	memset(http, 0, sizeof(http));
	tcp_connection_recv(tcp , http , sizeof(http));
	printf("%s\n" , http);
	return parse_subscribe_response(http , error);

}

int fetion_directsms_send_sms(User *user
		, const char *to , const char *msg)
{
	FetionSip *sip = user->sip;
	SipHeader *svheader , *eheader , *theader;
	char tostr[24] , *res , rep[1024];
	int code;
	
	fetion_sip_set_type(sip , SIP_MESSAGE);
	memset(tostr, 0, sizeof(tostr));
	sprintf(tostr , "tel:%s" , to);
	theader = fetion_sip_header_new("T" , tostr);
	fetion_sip_add_header(sip , theader);
	svheader = fetion_sip_header_new("SV" , "1");
	fetion_sip_add_header(sip , svheader);
	eheader = fetion_sip_event_header_new(SIP_EVENT_SENDDIRECTCATSMS);
	fetion_sip_add_header(sip , eheader);
	res = fetion_sip_to_string(sip , msg);
	tcp_connection_send(sip->tcp , res , strlen(res));
	printf("%s\n" , res);
	memset(rep, 0, sizeof(rep));
	int ret = tcp_connection_recv(sip->tcp , rep , sizeof(rep));
	printf("%d\n" , ret);
	printf("%s\n" , rep);
	code = fetion_sip_get_code(rep);
	if(code == 280){
		return SEND_SMS_SUCCESS;
	}else{
		if(code == 420 || code == 421){
			parse_option_verification(user , rep);
			return SEND_SMS_NEED_AUTHENTICATION;
		}else{
			return SEND_SMS_OTHER_ERROR;
		}
	}
}

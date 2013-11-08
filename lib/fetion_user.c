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
#include <signal.h>

struct unacked_list *unackedlist;

static int fetion_user_download_portrait_again(const char* filepath , const char* buf , Proxy *proxy);
static char* generate_set_state_body(StateType state);
static char* generate_set_moodphrase_body(const char* customConfigVersion
				, const char* customConfig , const char* personalVersion ,  const char* moodphrase);
static char* generate_update_information_body(User* user);
static char* generate_keep_alive_body();
static void parse_set_moodphrase_response(User* user , const char* sipmsg);
static char* generate_set_sms_status_body(int days);
static void parse_set_sms_status_response(User *user , const char *sipmsg);

User* fetion_user_new(const char* no , const char* password)
{
	User* user = (User*)malloc(sizeof(User));

 	struct sigaction sa;
 	sa.sa_handler = SIG_IGN;
 	sigaction(SIGPIPE, &sa, 0 );

	memset(user , 0 , sizeof(User));
	if(strlen(no) == 11){
		strcpy(user->mobileno , no);
		user->loginType = LOGIN_TYPE_MOBILENO;
	}else{
		strcpy(user->sId , no);
		user->loginType = LOGIN_TYPE_FETIONNO;
	}
	strcpy(user->password , password);
	user->contactList = fetion_contact_new();
	user->groupList = fetion_group_new();
	user->pggroup = NULL;
	user->sip = NULL;
	user->verification = NULL;
	user->customConfig = NULL;
	user->ssic = NULL;
	user->config = NULL;

	unackedlist = unacked_list_new(NULL);

	return user;
}

void fetion_user_set_userid(User* user, const char* userid1)
{
	strcpy(user->userId, userid1);
}

void fetion_user_set_sid(User* user, const char* sId1)
{
	strcpy(user->sId, sId1);
}

void fetion_user_set_mobileno(User* user , const char* mobileno1)
{
	strcpy(user->mobileno , mobileno1);
}

void fetion_user_set_password(User *user, const char *password)
{
	strcpy(user->password, password);
	user->password[strlen(password)] = '\0';
}

void fetion_user_set_sip(User* user , FetionSip* sip1)
{
	debug_info("Set a initialized Sip Struct to User");
	user->sip = sip1;
}

void fetion_user_set_config(User* user , Config* config1)
{
	debug_info("Set a initialized Config Struct to User");
	user->config = config1;
}

void fetion_user_set_verification_code(User* user , const char* code)
{
	user->verification->code = (char*)malloc(strlen(code) + 1);
	memset(user->verification->code , 0 , strlen(code) + 1);
	strcpy(user->verification->code , code);
}

int fetion_user_init_config(User *user)
{
	assert(user != NULL);
	assert(user->config != NULL);
	assert(*(user->userId) != '\0');
	return fetion_config_initialize(user->config, user->userId);
}

void fetion_user_free(User* user)
{
	if(user->ssic != NULL)
		free(user->ssic);
	if(user->customConfig != NULL)
		free(user->customConfig);
	if(user->verification != NULL)
		fetion_verification_free(user->verification);
	if(user->sip != NULL) 
		fetion_sip_free(user->sip);
	if(user->config != NULL)
		fetion_config_free(user->config);
	free(user);
}
int fetion_user_set_state(User* user , StateType state)
{
	SipHeader* eheader;
	FetionSip* sip = user->sip;
	char* body;
	char* res;

	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_SETPRESENCE);
	fetion_sip_add_header(sip , eheader);
	body = generate_set_state_body(state);
	res = fetion_sip_to_string(sip , body);
	tcp_connection_send(sip->tcp , res , strlen(res));
	user->state = state;
	free(body);
	free(res);
	debug_info("User state changed to %d" , state);
	return 1;
}
int fetion_user_set_moodphrase(User* user , const char* moodphrase)
{
	FetionSip* sip = user->sip;
	SipHeader* eheader;
	char *res , *body;
	int ret;
	fetion_sip_set_type(sip , SIP_SERVICE);
	debug_info("Start seting moodphrase");
	eheader = fetion_sip_event_header_new(SIP_EVENT_SETUSERINFO);
	fetion_sip_add_header(sip , eheader);
	body = generate_set_moodphrase_body(user->customConfigVersion
									  , user->customConfig
									  , user->personalVersion
									  , moodphrase);
	res = fetion_sip_to_string(sip , body);
	free(body);
	tcp_connection_send(sip->tcp , res , strlen(res));
	free(res) ; 
	res = fetion_sip_get_response(sip);
	ret = fetion_sip_get_code(res);
	if(ret == 200){
		parse_set_moodphrase_response(user , res);
		free(res);
		debug_info("Set moodphrase success");
		return 1;
	}else{
		free(res);
		debug_error("Set moodphrase failed , errno :" , ret);
		return -1;
	}

}
int fetion_user_update_info(User* user)
{
	FetionSip* sip = user->sip;
	SipHeader* eheader = NULL;
	char *res , *body;
	int ret;
	fetion_sip_set_type(sip , SIP_SERVICE);
	debug_info("Start Updating User Information");
	eheader = fetion_sip_event_header_new(SIP_EVENT_SETUSERINFO);
	fetion_sip_add_header(sip , eheader);
	body = generate_update_information_body(user);
	res = fetion_sip_to_string(sip , body);
	free(body);
	tcp_connection_send(sip->tcp , res , strlen(res));
	free(res) ; 
	res = fetion_sip_get_response(sip);
	ret = fetion_sip_get_code(res);

	if(ret == 200){
		free(res);
		debug_info("Update information success");
		return 1;
	}else{
		free(res);
		debug_error("Update information failed , errno :" , ret);
		return -1;
	}
}
int fetion_user_keep_alive(User* user)
{
	FetionSip* sip = user->sip;
	SipHeader* eheader = NULL;
	int ret;
	char *res = NULL , *body = NULL;
	fetion_sip_set_type(sip , SIP_REGISTER);
	debug_info("send a keep alive request");
	eheader = fetion_sip_event_header_new(SIP_EVENT_KEEPALIVE);
	fetion_sip_add_header(sip , eheader);
	body = generate_keep_alive_body();
	res = fetion_sip_to_string(sip , body);
	free(body);
	ret = tcp_connection_send(sip->tcp , res , strlen(res));
	free(res); 
	return ret;
}
Group* fetion_group_new()
{
	Group* list = (Group*)malloc(sizeof(Group));
	memset(list , 0 , sizeof(Group));
	list->pre = list;
	list->next = list;
	return list;
}
void fetion_group_list_append(Group* head , Group* group)
{
	head->next->pre = group;
	group->next = head->next;
	group->pre = head;
	head->next = group;
}

void fetion_group_list_prepend(Group* head , Group* group)
{
	head->pre->next = group;
	group->next = head;
	group->pre = head->pre;
	head->pre = group;
}

void fetion_group_list_remove(Group *group)
{
	group->next->pre = group->pre;
	group->pre->next = group->next;
}

void fetion_group_remove(Group* head , int groupid)
{
	Group *gl_cur;
	foreach_grouplist(head , gl_cur){
		if(gl_cur->groupid == groupid){
			gl_cur->pre->next = gl_cur->next;
			gl_cur->next->pre = gl_cur->pre;
			free(gl_cur);
			break;
		}
	}
}
Group* fetion_group_list_find_by_id(Group* head , int id)
{
	Group *gl_cur;
	foreach_grouplist(head , gl_cur){
		if(gl_cur->groupid == id){
			return gl_cur;
		}
	}
	return NULL;
}
Verification* fetion_verification_new()
{
	Verification* ver = (Verification*)malloc(sizeof(Verification));
	memset(ver , 0 , sizeof(Verification));
	ver->algorithm = NULL;
	ver->type = NULL;
	ver->text = NULL;
	ver->tips = NULL;
	return ver;
}
void fetion_verification_free(Verification* ver)
{
	if(ver != NULL){
		free(ver->algorithm);
		free(ver->type);
		free(ver->text);
		free(ver->tips);
		free(ver->guid);
		free(ver->code);
	}
	free(ver);
}

int fetion_user_upload_portrait(User* user , const char* filename)
{
	char http[1024];
	unsigned char buf[1024];
	char res[1024];
	char code[4];
	char* ip = NULL;
	FILE* f = NULL;
	Config *config = user->config;
	char* server = config->portraitServerName;
	char* portraitPath = config->portraitServerPath;
	Proxy *proxy = config->proxy;
	long filelength;
	int n;
	FetionConnection* tcp;

	ip = get_ip_by_name(server);
	if(ip == NULL){
		debug_error("Parse server ip address failed , %s" , server);
		return -1;
	}

	f = fopen(filename , "r");
	fseek(f , 0 , SEEK_END);
	filelength = ftell(f);
	rewind(f);
	debug_info("uploading portrait....");
	sprintf(http , "POST /%s/setportrait.aspx HTTP/1.1\r\n"
		    	   "Cookie: ssic=%s\r\n"
				   "Accept: */*\r\n"
		    	   "Host: %s\r\n"
		    	   "Content-Length: %ld\r\n"
		    	   "Content-Type: image/jpeg\r\n"
		    	   "User-Agent: IIC2.0/PC 4.0.0000\r\n"
				   "Connection: Keep-Alive\r\n"
				   "Cache-Control: no-cache\r\n\r\n"
		  		  , portraitPath , user->ssic , server , filelength);

	tcp = tcp_connection_new();
	if(proxy != NULL && proxy->proxyEnabled)
		tcp_connection_connect_with_proxy(tcp , ip , 80 , proxy);
	else
		tcp_connection_connect(tcp , ip , 80);

	tcp_connection_send(tcp , http , strlen(http));

	memset(buf , 0 , sizeof(buf));
	int ret;
	while((n = fread(buf , 1 , sizeof(buf) , f))){
		ret = tcp_connection_send(tcp , buf , n) ;
		if(ret == -1){
			fclose(f);
			return -1;
		}
		memset(buf , 0 , sizeof(buf));
	}
	fclose(f);

	memset(res, 0, sizeof(res));
	tcp_connection_recv(tcp , res , sizeof(res));
	memset(code, 0, sizeof(code));
	strncpy(code , res + 9 , 3);
	if(strcmp(code , "200") == 0){
		debug_info("Upload portrait success");
		return 1;
	}else{
		debug_error("Upload portrait failed");
		return -1;
	}
}

int fetion_user_set_sms_status(User *user , int days)
{
	FetionSip *sip = user->sip;
	SipHeader *eheader;
	char buffer[2048];
	char code[16];
	char *body;
	char *res;

	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_SETUSERINFO);
	fetion_sip_add_header(sip , eheader);
	body = generate_set_sms_status_body(days);
	res = fetion_sip_to_string(sip , body);
	tcp_connection_send(sip->tcp , res , strlen(res));
	free(res);
	memset(buffer , 0 , sizeof(buffer));
	tcp_connection_recv(sip->tcp , buffer , sizeof(buffer));

	res = strstr(buffer , " ") + 1;
	memset(code , 0 , sizeof(code));
	strncpy(code , res , 3);
	if(strcmp(code , "200") == 0){
		parse_set_sms_status_response(user , buffer);
		debug_info("set sms online status to %d days[%s]"
					   	, days , user->smsOnLineStatus);
		return 1;
	}else{
		debug_error("failed to set sms online status");
		return -1;
	}

	return 1;
}

int fetion_user_download_portrait(User* user , const char* sipuri)
{
    char uri[256];
	char *server = user->config->portraitServerName;
	char *portraitPath = user->config->portraitServerPath;
	sprintf(uri , "/%s/getportrait.aspx" , portraitPath);

	return fetion_user_download_portrait_with_uri(user , sipuri , server , uri);
}

int fetion_user_download_portrait_with_uri(User *user , const char *sipuri
       	, const char *server , const char *portraitpath)
{
	char buf[2048] , *ip , *pos = NULL;
	FILE *f = NULL;
	char filename[256];
	char *encodedSipuri , *encodedSsic , replyCode[4] = { 0 };
	char *friendSid = NULL;
	Config *config = user->config;
	FetionConnection* tcp = NULL;
	int i = 0 , isFirstChunk = 0 , chunkLength = 0 , imageLength = 0 , receivedLength = 0;
	int ret;

	ip = get_ip_by_name(server);
	if(ip == NULL)
	{
		debug_error("Parse server ip address failed , %s" , server);
		return -1;
	}
	if(! sipuri || *sipuri == '\0')
		return -1;
	friendSid = fetion_sip_get_sid_by_sipuri(sipuri);
	if(friendSid == NULL)
		return -1;
/*	open a file to write ,if not exist then create one*/
	sprintf(filename , "%s/%s.jpg" , config->iconPath ,  friendSid);
	free(friendSid);
	encodedSipuri = http_connection_encode_url(sipuri);
	encodedSsic = http_connection_encode_url(user->ssic);
	sprintf(buf , "GET %s?Uri=%s"
			  "&Size=120&c=%s HTTP/1.1\r\n"
			  "User-Agent: IIC2.0/PC "PROTO_VERSION"\r\n"
			  "Accept: image/pjpeg;image/jpeg;image/bmp;"
			  "image/x-windows-bmp;image/png;image/gif\r\n"
			  "Host: %s\r\nConnection: Keep-Alive\r\n\r\n"
			  , portraitpath , encodedSipuri , encodedSsic , server);

	tcp = tcp_connection_new();
	if(config->proxy != NULL && config->proxy->proxyEnabled)
		ret = tcp_connection_connect_with_proxy(tcp , ip , 80 , config->proxy);
	else
		ret = tcp_connection_connect(tcp, ip, 80);
	if(ret < 0){
		debug_error("connect to portrait server:%s",
				strerror(errno));
		return -1;
	}
	free(ip);
	ret = tcp_connection_send(tcp , buf , strlen(buf));	
	if(ret < 0)
		return -1;

	//read reply

	/* 200 OK begin to download protrait ,
	 * 302 need to redirect ,404 not found */
	for(;;){
		memset(buf, 0, sizeof(buf));
		chunkLength = tcp_connection_recv(tcp , buf , sizeof(buf) -1);
		if(chunkLength < 0)
			break;
		if(isFirstChunk == 0)
		{	
			/* check the code num for the first segment*/
			memcpy(replyCode , buf + 9 , 3);
			switch(atoi(replyCode))
			{   
				/*	no protrait for current user found
				 * ,just return a error */
				case 404:
					goto end;
					break;
				/*write the image bytes of the first segment into file*/
				case 200:
					f = fopen(filename , "wb+");
					if( f == NULL )
					{
						debug_error("Write user portrait to local disk failed");
						return -1;
					}
					pos = (char*)buf;
					imageLength = http_connection_get_body_length(pos);
					receivedLength = chunkLength - http_connection_get_head_length(pos) -4;
					for(i = 0 ; i < chunkLength ; i++ )
						if( buf[i] == '\r' && buf[i+1] == '\n'
							&&buf[i+2] == '\r' && buf[i+3] == '\n' )
						{
							fwrite(buf + i + 4 , chunkLength - i -4 , 1 ,f);
							fflush(f);
							break;
						}
					if(receivedLength == imageLength)
						goto end;
					break;
				default:
					goto redirect;
					break;
			};
			isFirstChunk ++;
		}
		else
		{
			if(strcmp(replyCode , "200") == 0){
				fwrite(buf , chunkLength , 1 , f);
				fflush(f);
			}
			receivedLength += chunkLength;
			if(receivedLength == imageLength)
				break;
		}
	}
	if(strcmp(replyCode , "200") == 0)
	{
		fclose(f);
		f = NULL;
		tcp_connection_free(tcp);
		tcp = NULL;
		return 0;
	}
redirect:
	if(strcmp(replyCode , "302") == 0)
		ret = fetion_user_download_portrait_again(filename , buf , config->proxy);
end:
	if(f != NULL)
		fclose(f);
	tcp_connection_free(tcp);
	tcp = NULL;
	if(ret < 0)
		return -1;
	return 1;
}

static int fetion_user_download_portrait_again(const char* filepath , const char* buf , Proxy* proxy)
{
	char location[1024] = { 0 };
	char httpHost[50] = { 0 };
	char httpPath[512] = { 0 };
	char http[1024] = { 0 };
	char replyCode[5] = { 0 };
	FILE* f = NULL;
	FetionConnection* tcp = NULL;
	char* ip = NULL;
	char* pos = strstr(buf , "Location: ") ;
	int chunkLength = 0 , imageLength = 0 , receivedLength = 0;
	int i , n = 0;
	int ret;
	
	int isFirstChunk = 0;
	unsigned char img[2048] = { 0 };

	if(pos == NULL)
		return -1;
	pos += 10;
	n = strlen(pos) - strlen(strstr(pos , "\r\n"));
	strncpy(location , pos , n );
	pos = location + 7;
	n = strlen(pos) - strlen(strstr(pos , "/"));
	strncpy(httpHost , pos , n);
	pos += n;
	strcpy(httpPath , pos);
	sprintf(http , "GET %s HTTP/1.1\r\n"
				   "User-Agent: IIC2.0/PC 3.3.0370\r\n"
			 	   "Accept: image/pjpeg;image/jpeg;image/bmp;"
				   "image/x-windows-bmp;image/png;image/gif\r\n"
				   "Cache-Control: private\r\n"
				   "Host: %s\r\n"
				   "Connection: Keep-Alive\r\n\r\n" , httpPath , httpHost);
	ip = get_ip_by_name(httpHost);
	if(ip == NULL){
		debug_error("Parse portrait server ip address failed , %s" , httpHost);
		return -1;
	}
	tcp = tcp_connection_new();

	if(proxy != NULL && proxy->proxyEnabled)
		ret = tcp_connection_connect_with_proxy(tcp , ip , 80 , proxy);
	else
		ret = tcp_connection_connect(tcp , ip , 80);

	if(ret < 0)
		return -1;

	free(ip);
	ret = tcp_connection_send(tcp , http , strlen(http));
	if(ret < 0)
		return -1;
	//read portrait data
	f = fopen(filepath , "wb+");
	for(;;){
		memset(img, 0 , sizeof(img));
		chunkLength = tcp_connection_recv(tcp , img , sizeof(img)-1);
		if(chunkLength <= 0)
			break;
		if(isFirstChunk ++ == 0)
		{
			char* pos = (char*)(img);
			strncpy(replyCode , pos + 9 , 3 );
			if(strcmp(replyCode , "404") == 0){
				fclose(f);
				f = NULL;
				goto end;
			}
			imageLength = http_connection_get_body_length(pos);
			receivedLength = chunkLength - http_connection_get_head_length(pos) - 4;
			for(i = 0 ; i < chunkLength ; i ++)
				if( img[i] == '\r' && img[i+1] == '\n'
					&&img[i+2] == '\r' && img[i+3] == '\n' )
				{
					fwrite(img + i +4 , chunkLength - i - 4 , 1 ,f);
					break;
				}
			if(receivedLength == imageLength)
			{
				fclose(f);
				f = NULL;
				goto end;
			}
		}
		else
		{
			fwrite(img , chunkLength , 1 , f);
			receivedLength += chunkLength;
			if(receivedLength == imageLength)
				break;
		}
		memset(img , 0 , sizeof(img));
	}
	if(f != NULL)
		fclose(f);
end:
	tcp_connection_free(tcp);
	tcp = NULL;
	return 0;
}
Contact* fetion_user_parse_presence_body(const char* body , User* user)
{
	xmlDocPtr doc;
	xmlNodePtr node , cnode;
	xmlChar* pos;
	Contact* contact;
	Contact* contactres;
	Contact* contactlist = user->contactList;
	Contact* currentContact;

	contactres = fetion_contact_new();

	doc = xmlParseMemory(body , strlen(body));
	node = xmlDocGetRootElement(doc);
	node = xml_goto_node(node , "c");
	while(node != NULL)
	{
		pos = xmlGetProp(node , BAD_CAST "id");
		currentContact = fetion_contact_list_find_by_userid(contactlist , (char*)pos);
		if(currentContact == NULL)
		{
			/*not a valid information*/
			/*debug_error("User %s is not a valid user" , (char*)pos);*/
			node = node->next;
			continue;
		}
		cnode = node->xmlChildrenNode;
		if(xmlHasProp(cnode , BAD_CAST "sid"))
		{
			pos = xmlGetProp(cnode , BAD_CAST "sid");
			strcpy(currentContact->sId ,  (char*)pos);
			xmlFree(pos);
		}
		if(xmlHasProp(cnode , BAD_CAST "m"))
		{
			pos = xmlGetProp(cnode , BAD_CAST "m");
			strcpy(currentContact->mobileno ,  (char*)pos);
			xmlFree(pos);
		}
		if(xmlHasProp(cnode , BAD_CAST "l"))
		{
			pos = xmlGetProp(cnode , BAD_CAST "l");
			currentContact->scoreLevel = atoi((char*)pos);
			xmlFree(pos);
		}
		if(xmlHasProp(cnode , BAD_CAST "n"))
		{
			pos = xmlGetProp(cnode , BAD_CAST "n");
			strcpy(currentContact->nickname ,  (char*)pos);
			xmlFree(pos);
		}
		if(xmlHasProp(cnode , BAD_CAST "i"))
		{
			pos = xmlGetProp(cnode , BAD_CAST "i");
			strcpy(currentContact->impression ,  (char*)pos);
			xmlFree(pos);
		}
		if(xmlHasProp(cnode , BAD_CAST "p"))
		{
			pos = xmlGetProp(cnode , BAD_CAST "p");
			if(strcmp(currentContact->portraitCrc, (char*)pos) == 0
					|| strcmp((char*)pos, "0") == 0)
				currentContact->imageChanged = 0;
			else
				currentContact->imageChanged = 1;
			strcpy(currentContact->portraitCrc ,  (char*)pos);
			xmlFree(pos);
		}else{
			currentContact->imageChanged = 0;
		}

		if(xmlHasProp(cnode , BAD_CAST "c"))
		{
			pos = xmlGetProp(cnode , BAD_CAST "c");
			strcpy(currentContact->carrier , (char*)pos);
			xmlFree(pos);
		}
		if(xmlHasProp(cnode , BAD_CAST "cs"))
		{
			pos = xmlGetProp(cnode , BAD_CAST "cs");
			currentContact->carrierStatus = atoi((char*)pos);
			xmlFree(pos);
		}
		if(xmlHasProp(cnode , BAD_CAST "s"))
		{
			pos = xmlGetProp(cnode , BAD_CAST "s");
			currentContact->serviceStatus = atoi((char*)pos);
			xmlFree(pos);
		}
#if 0
		if(xmlHasProp(cnode , BAD_CAST "sms")){
			pos = xmlGetProp(cnode , BAD_CAST "sms");
			xmlFree(pos);
		}
#endif
		cnode = xml_goto_node(node , "pr");
		if(xmlHasProp(cnode , BAD_CAST "dt"))
		{
			pos = xmlGetProp(cnode , BAD_CAST "dt");
			strcpy(currentContact->devicetype ,  *((char*)pos) == '\0' ? "PC" : (char*)pos);
			xmlFree(pos);
		}
		if(xmlHasProp(cnode , BAD_CAST "b"))
		{
			pos = xmlGetProp(cnode , BAD_CAST "b");
			currentContact->state = atoi((char*)pos);
			xmlFree(pos);
		}
		contact = fetion_contact_new();
		memset(contact , 0 , sizeof(contact));
		memcpy(contact , currentContact , sizeof(Contact));
		fetion_contact_list_append(contactres , contact);
		node = node->next;
	}
	xmlFreeDoc(doc);
	return contactres;
}
Contact* fetion_user_parse_syncuserinfo_body(const char* body , User* user)
{
	xmlDocPtr doc;
	xmlNodePtr node;
	xmlChar* pos;
	Contact* contactlist = user->contactList;
	Contact* currentContact = NULL;

	doc = xmlParseMemory(body , strlen(body));
	node = xmlDocGetRootElement(doc);
	node = xml_goto_node(node , "buddy");
	if(node == NULL)
		return NULL;
	while(node){
		if(xmlHasProp(node , BAD_CAST "action")){
			pos = xmlGetProp(node , BAD_CAST "action");
			if(xmlStrcmp(pos , BAD_CAST "add") != 0){
				xmlFree(pos);
				node = node->next;
				continue;
			}
			xmlFree(pos);
		}
		
		pos = xmlGetProp(node , BAD_CAST "user-id");
		currentContact = fetion_contact_list_find_by_userid(contactlist , (char*)pos);
		//currentContact = fetion_contact_new();
		debug_info("synchronize user information");
		if(currentContact == NULL)
		{
			/*not a valid information*/
			debug_error("User %s is not a valid user" , (char*)pos);
			return NULL;
		}
		if(xmlHasProp(node , BAD_CAST "uri"))
		{
			pos = xmlGetProp(node , BAD_CAST "uri");
			strcpy(currentContact->sipuri ,  (char*)pos);
			xmlFree(pos);
		}
		if(xmlHasProp(node , BAD_CAST "relation-status"))
		{
			pos = xmlGetProp(node , BAD_CAST "relation-status");
			currentContact->relationStatus = atoi((char*)pos);
			if(atoi((char*)pos) == 1){
				debug_info("User %s accepted your request" , currentContact->userId);
			}else{
				debug_info("User %s refused your request" , currentContact->userId);
			}
			xmlFree(pos);
		}
		xmlFreeDoc(doc);
		return currentContact;
		node = node->next;
	}
	xmlFreeDoc(doc);
	return currentContact;
}

void fetion_user_save(User *user)
{
	char path[256];
	char sql[4096];
	char password[4096];
	char impression[4096];
	sqlite3 *db;
	Config *config = user->config;

	sprintf(path, "%s/data.db",
				   	config->userPath);

	debug_info("Save user information");
	if(sqlite3_open(path, &db)){
		debug_error("open data.db:%s",
					sqlite3_errmsg(db));
		return;
	}

	sprintf(sql, "delete from user;");
	if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
		sprintf(sql, "create table user ("
					"sId,userId,mobileno,password,sipuri,"
					"publicIp,lastLoginIp,lastLoginTime,"
					"personalVersion, contactVersion,"
					"nickname,impression,country,province,"
					"city,gender,smsOnLineStatus,"
					"customConfigVersion, customConfig,"
					"boundToMobile);");
		if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
			debug_error("create table user:%s", sqlite3_errmsg(db));
			sqlite3_close(db);
			return;
		}
	}
	sprintf(password, "%s", user->password);
	sprintf(impression, "%s", user->impression);
	escape_sql(password);
	escape_sql(impression);
	snprintf(sql, sizeof(sql)-1, "insert into user "
				"values ('%s','%s','%s','%s','%s',"
				"'%s','%s','%s','%s','%s',"
				"'%s','%s','%s','%s','%s',%d,'%s',"
				" '%s', '%s',%d);",
				user->sId, user->userId, user->mobileno,
				password, user->sipuri, user->publicIp,
				user->lastLoginIp, user->lastLoginTime,
				user->personalVersion, user->contactVersion,
				user->nickname, impression, user->country,
				user->province, user->city, user->gender,
				user->smsOnLineStatus, user->customConfigVersion,
				user->customConfig, user->boundToMobile);
	if(sqlite3_exec(db, sql, NULL, NULL, NULL))
		debug_error("update user:%s\n%s", sqlite3_errmsg(db), sql);

	sqlite3_close(db);
}

void fetion_user_load(User *user)
{
	char path[256];
	char sql[4096];
	char **sqlres;
	sqlite3 *db;
	int ncols, nrows;
	Config *config = user->config;

	sprintf(path, "%s/data.db",config->userPath);

	debug_info("Load user information");
	if(sqlite3_open(path, &db)){
		debug_error("open data.db:%s", sqlite3_errmsg(db));
		return;
	}

	sprintf(sql, "select * from user;");
	if(sqlite3_get_table(db, sql, &sqlres, &nrows, &ncols, NULL)){
		sqlite3_close(db);
		return;
	}

	if(nrows == 0 || ncols == 0){
		sqlite3_close(db);
		return;
	}

	strcpy(user->sId , 				sqlres[ncols]);
	strcpy(user->userId , 			sqlres[ncols+1]);
	strcpy(user->mobileno , 		sqlres[ncols+2]);
	strcpy(user->password , 		sqlres[ncols+3]);
	strcpy(user->sipuri , 			sqlres[ncols+4]);
	strcpy(user->publicIp , 		sqlres[ncols+5]);
	strcpy(user->lastLoginIp , 		sqlres[ncols+6]);
	strcpy(user->lastLoginTime , 	sqlres[ncols+7]);
	strcpy(user->personalVersion , 	sqlres[ncols+8]);
	strcpy(user->contactVersion , 	sqlres[ncols+9]);
	strcpy(user->nickname , 		sqlres[ncols+10]);
	strcpy(user->impression , 		sqlres[ncols+11]);
	strcpy(user->country , 			sqlres[ncols+12]);
	strcpy(user->province , 		sqlres[ncols+13]);
	strcpy(user->city , 			sqlres[ncols+14]);
	user->gender = 					atoi(sqlres[ncols+15]);
	strcpy(user->smsOnLineStatus ,  sqlres[ncols+16]);
	strcpy(user->customConfigVersion ,  sqlres[ncols+17]);
	user->customConfig = (char*)malloc(strlen(sqlres[ncols+18])+1);
	memset(user->customConfig, 0, strlen(sqlres[ncols+18])+1);
	strcpy(user->customConfig, sqlres[ncols+18]);
	user->boundToMobile = atoi(sqlres[ncols+19]);

	unescape_sql(user->password);
	unescape_sql(user->impression);
	
	sqlite3_free_table(sqlres);
	sqlite3_close(db);
}

static char* generate_set_state_body(StateType state)	
{
	char s[16];
	char data[] = "<args></args>";
	xmlChar* res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlParseMemory(data , strlen(data));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "presence" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "basic" , NULL);
	snprintf(s, sizeof(s) - 1 , "%d" , state);
	xmlNewProp(node , BAD_CAST "value" , BAD_CAST s);
	xmlDocDumpMemory(doc , &res , NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);
}
static char* generate_set_moodphrase_body(const char* customConfigVersion
		, const char* customConfig , const char* personalVersion
		,  const char* moodphrase)
{
	char args[] = "<args></args>";
	xmlChar *res;
	xmlDocPtr doc;
	xmlNodePtr node , cnode;
	doc = xmlParseMemory(args , strlen(args));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "userinfo" , NULL);
	cnode = xmlNewChild(node , NULL , BAD_CAST "personal" , NULL);
	xmlNewProp(cnode , BAD_CAST "impresa" , BAD_CAST moodphrase);
	xmlNewProp(cnode , BAD_CAST "version" , BAD_CAST personalVersion);
	cnode = xmlNewChild(node , NULL , BAD_CAST "custom-config" , BAD_CAST customConfig);
	xmlNewProp(cnode , BAD_CAST "type" , BAD_CAST "PC");
	xmlNewProp(cnode , BAD_CAST "version" , BAD_CAST customConfigVersion);
	xmlDocDumpMemory(doc , &res , NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);
}
static char* generate_update_information_body(User* user)
{
	char args[] = "<args></args>";
	char gender[5];
	xmlChar *res;
	xmlDocPtr doc;
	xmlNodePtr node , cnode;
	doc = xmlParseMemory(args , strlen(args));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "userinfo" , NULL);
	cnode = xmlNewChild(node , NULL , BAD_CAST "personal" , NULL);
	xmlNewProp(cnode , BAD_CAST "impresa" , BAD_CAST user->impression);
	xmlNewProp(cnode , BAD_CAST "nickname" , BAD_CAST user->nickname);
	sprintf(gender , "%d" , user->gender);
	xmlNewProp(cnode , BAD_CAST "gender" , BAD_CAST gender);
	xmlNewProp(cnode , BAD_CAST "version" , BAD_CAST "0");
	cnode = xmlNewChild(node , NULL , BAD_CAST "custom-config" , BAD_CAST user->customConfig);
	xmlNewProp(cnode , BAD_CAST "type" , BAD_CAST "PC");
	xmlNewProp(cnode , BAD_CAST "version" , BAD_CAST user->customConfigVersion);
	xmlDocDumpMemory(doc , &res , NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);
}
static char* generate_keep_alive_body()
{
	char args[] = "<args></args>";
	xmlChar *res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlParseMemory(args , strlen(args));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "credentials" , NULL);
	xmlNewProp(node , BAD_CAST "domains" , BAD_CAST "fetion.com.cn");
	xmlDocDumpMemory(doc , &res , NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);
}
static void parse_set_moodphrase_response(User* user , const char* sipmsg)
{
	char *pos;
	xmlChar* res;
	xmlDocPtr doc;
	xmlNodePtr node;
	pos = strstr(sipmsg , "\r\n\r\n") + 4;
	doc = xmlParseMemory(pos , strlen(pos));
	node = xmlDocGetRootElement(doc);
	node = node->xmlChildrenNode->xmlChildrenNode;
	res = xmlGetProp(node , BAD_CAST "version");
	memset(user->personalVersion, 0, sizeof(user->personalVersion));
	strcpy(user->personalVersion , (char*)res);
	xmlFree(res);
	res = xmlGetProp(node , BAD_CAST "impresa");
	memset(user->impression, 0, sizeof(user->impression));
	strcpy(user->impression , (char*)res);
	xmlFree(res);
	node = node->next;
	res = xmlGetProp(node , BAD_CAST "version");
	memset(user->customConfigVersion, 0, sizeof(user->customConfigVersion));
	strcpy(user->customConfigVersion , (char*)res);	
	xmlFree(res);
	res = xmlNodeGetContent(node);
	free(user->customConfig);
	user->customConfig = (char*)malloc(strlen((char*)res) + 1);
	memset(user->customConfig, 0, strlen((char*)res) + 1);
	strcpy(user->customConfig , (char*)res);
	xmlFree(res);
	xmlFreeDoc(doc);
}

static char* generate_set_sms_status_body(int days)
{
	char args[] = "<args></args>";
	xmlChar *res;
	xmlDocPtr doc;
	xmlNodePtr node;
	char status[16];

	doc = xmlParseMemory(args , strlen(args));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "userinfo" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "personal" , NULL);
	sprintf(status , "%d.00:00:00" , days);
	xmlNewProp(node , BAD_CAST "sms-online-status" , BAD_CAST status);
	xmlDocDumpMemory(doc , &res , NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);
}

static void parse_set_sms_status_response(User *user , const char *sipmsg)
{
	char *pos;
	xmlChar* res;
	xmlDocPtr doc;
	xmlNodePtr node;
	pos = strstr(sipmsg , "\r\n\r\n") + 4;
	doc = xmlParseMemory(pos , strlen(pos));
	node = xmlDocGetRootElement(doc);
	node = xml_goto_node(node , "personal");
	if(!node)
		return;
	if(!xmlHasProp(node , BAD_CAST "sms-online-status"))
		return;
	res = xmlGetProp(node , BAD_CAST "sms-online-status");
	strcpy(user->smsOnLineStatus , (char*)res);
	xmlFree(res);
	xmlFreeDoc(doc);
}

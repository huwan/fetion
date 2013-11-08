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
#include <openssl/rsa.h>
#include <openssl/sha.h>

/*private method*/
static char* generate_auth_body(User *user);
static void parse_personal_info(xmlNodePtr node, User *user);
static void parse_contact_list(xmlNodePtr node, User *user, int *group_count, int *buddy_count);
static void parse_stranger_list(xmlNodePtr node, User *user);
static void parse_ssi_auth_success(xmlNodePtr node, User *user);
static void parse_ssi_auth_failed(xmlNodePtr node, User *user);
static unsigned char *strtohex(const char *in , int *len) ;
static char *hextostr(const unsigned char *in , int len) ;
static char *hash_password_v1(const unsigned char* b0 , int b0len , const unsigned char* password , int psdlen);
static char *hash_password_v2(const char *userid, const char *passwordhex);
static char *hash_password_v4(const char *userid, const char *password);
static char *hash_password_v5(const char *userid, const char *hashed_password);
static char *generate_cnouce() ;
static unsigned char *decode_base64(const char* in , int* len);

char* generate_response(const char *nouce, const char *userid,
		const char *password, const char *publickey, const char *key)
{
	char* psdhex = hash_password_v4(userid , password);
	char modulus[257];
	char exponent[7];
	int ret, flen;
	BIGNUM *bnn, *bne;
	unsigned char *out;
	unsigned char *nonce , *aeskey , *psd , *res;
	int nonce_len , aeskey_len , psd_len;
	RSA *r = RSA_new();

	key = NULL;

	memset(modulus, 0, sizeof(modulus));
	memset(exponent, 0, sizeof(exponent));

	memcpy(modulus , publickey , 256);
	memcpy(exponent , publickey + 256 , 6);
	nonce = (unsigned char*)malloc(strlen(nouce) + 1);
	memset(nonce , 0 , strlen(nouce) + 1);
	memcpy(nonce , (unsigned char*)nouce , strlen(nouce));
	nonce_len = strlen(nouce);
	psd = strtohex(psdhex , &psd_len);
	aeskey = strtohex(generate_aes_key() , &aeskey_len);
	res = (unsigned char*)malloc(nonce_len + aeskey_len + psd_len + 1);
	memset(res , 0 , nonce_len + aeskey_len + psd_len + 1);
	memcpy(res , nonce , nonce_len);
	memcpy(res + nonce_len , psd , psd_len );
	memcpy(res + nonce_len + psd_len , aeskey , aeskey_len);

	bnn = BN_new();
	bne = BN_new();
	BN_hex2bn(&bnn, modulus);
	BN_hex2bn(&bne, exponent);
	r->n = bnn;	r->e = bne;	r->d = NULL;
//	RSA_print_fp(stdout, r, 5);
	flen = RSA_size(r);
	out =  (unsigned char*)malloc(flen);
	memset(out , 0 , flen);
	debug_info("Start encrypting response");
	ret = RSA_public_encrypt(nonce_len + aeskey_len + psd_len,
			res , out, r, RSA_PKCS1_PADDING);
	if (ret < 0){
		debug_info("Encrypt response failed!");
		free(res); 
		free(aeskey);
		free(psd);
		free(nonce);
		return NULL;
	}
	RSA_free(r);
	debug_info("Encrypting reponse success");
	free(res); 
	free(aeskey);
	free(psd);
	free(nonce);
	return hextostr(out , ret);
}

void generate_pic_code(User *user)
{
	char buf[1024] , *res , *code;
	char codePath[128];
	char cookie[1024];
	char* ip;

	FILE* picfd;
	int piclen = 0;
	unsigned char* pic;
	int n;
	Config *config = user->config;

	xmlDocPtr doc;
	xmlNodePtr node;

	memset(buf , 0 , sizeof(buf));
	ip = get_ip_by_name(NAVIGATION_URI);
	FetionConnection* con = tcp_connection_new();

	if(config->proxy != NULL && config->proxy->proxyEnabled)
		tcp_connection_connect_with_proxy(con , ip,
				80 , config->proxy);
	else
		tcp_connection_connect(con , ip , 80);

	memset(cookie , 0 , sizeof(cookie));
	if(user->ssic){
		sprintf(cookie , "Cookie: ssic=%s\r\n" , user->ssic);
	}
	sprintf(buf , "GET /nav/GetPicCodeV4.aspx?algorithm=%s HTTP/1.1\r\n"
				  "%sHost: %s\r\n"
				  "User-Agent: IIC2.0/PC "PROTO_VERSION"\r\n"
				  "Connection: close\r\n\r\n"
				, user->verification->algorithm == NULL ? "" : user->verification->algorithm
				, user->ssic == NULL ? "" : cookie , NAVIGATION_URI);
	tcp_connection_send(con , buf , strlen(buf));
	res = http_connection_get_response(con);
	tcp_connection_free(con);
	doc = xmlParseMemory(res , strlen(res));
	node = xmlDocGetRootElement(doc);
	node = node->xmlChildrenNode;
	user->verification->guid = (char*)xmlGetProp(node , BAD_CAST "id");
	code = (char*)xmlGetProp(node , BAD_CAST "pic");
	xmlFreeDoc(doc);
	debug_info("Generating verification code picture");
	pic = decode_base64(code , &piclen);
	free(code);
	memset(codePath, 0, sizeof(codePath));
	sprintf(codePath , "%s/code.gif" , user->config->globalPath);
	picfd = fopen(codePath , "wb+");
	n = 0;
	for(; n != piclen ;){
		n += fwrite(pic + n , 1 , piclen - n , picfd);
	}
	fclose(picfd);
	free(res);
}

char *ssi_auth_action(User *user)
{
	char sslbuf[2048] = { 0 };
	const char ssiName[] = "uid.fetion.com.cn";
	char noUri[128];
	char verifyUri[256];
	char *password , *ssi_ip , *res;
	int passwordType;
	
	debug_info("Initialize ssi authentication action");

	if(strlen(user->password) == 40) /* must be a hashed password */
		password = hash_password_v5(user->userId, user->password);
	else 
		password = hash_password_v4(user->userId, user->password);

	memset(noUri, 0, sizeof(noUri));
	if(user->loginType == LOGIN_TYPE_MOBILENO)
		sprintf(noUri , "mobileno=%s" , user->mobileno);
	else
		sprintf(noUri , "sid=%s" , user->sId);

	memset(verifyUri, 0, sizeof(verifyUri));
	if(user->verification != NULL && user->verification->code != NULL) {
		sprintf(verifyUri , "&pid=%s&pic=%s&algorithm=%s"
						  , user->verification->guid
						  , user->verification->code
						  , user->verification->algorithm);
	}

	passwordType = (strlen(user->userId) == 0 ? 1 : 2);
	sprintf(sslbuf, "GET /ssiportal/SSIAppSignInV4.aspx?%s"
				    "&domains=fetion.com.cn%s&v4digest-type=%d&v4digest=%s\r\n"
				    "User-Agent: IIC2.0/pc "PROTO_VERSION"\r\n"
					"Host: %s\r\n"
				    "Cache-Control: private\r\n"
				    "Connection: Keep-Alive\r\n\r\n",
				    noUri , verifyUri , passwordType , password , "uid.fetion.com.cn");
	ssi_ip = get_ip_by_name(ssiName);
	FetionConnection* ssl;
	ssl = tcp_connection_new();

	if(user->config->proxy != NULL && user->config->proxy->proxyEnabled) {
		int ret;
		ret = tcp_connection_connect_with_proxy(ssl , ssi_ip , 443 , user->config->proxy);
		if(ret < 0)
			return NULL;
	} else {
		if(tcp_connection_connect(ssl , ssi_ip , 443) < 0)
			return NULL;
	}

	debug_info("Start ssi login with %s password , user number %s"
			, passwordType == 1 ? "v3Temp" : "v4"
			, user->loginType == LOGIN_TYPE_MOBILENO ? user->mobileno : user->sId);

	if(ssl_connection_start(ssl) == -1) {
		debug_error("Initialize ssl failed,please retry or check your system`s configuration");
		return NULL;
	}
	res = ssl_connection_get(ssl , sslbuf);
	tcp_connection_free(ssl);
	free(password);
	free(ssi_ip);
	return res;
}

char *sipc_reg_action(User *user)
{
	char* sipmsg;
	char* cnouce = generate_cnouce();

	FetionSip* sip = user->sip;

	debug_info("Initialize sipc registeration action");

	fetion_sip_set_type(sip , SIP_REGISTER);
	SipHeader* cheader = fetion_sip_header_new("CN" , cnouce);
	SipHeader* client = fetion_sip_header_new("CL" , "type=\"pc\" ,version=\""PROTO_VERSION"\"");
	fetion_sip_add_header(sip , cheader);
	fetion_sip_add_header(sip , client);
	free(cnouce);
	sipmsg = fetion_sip_to_string(sip , NULL);
	debug_info("Start registering to sip server(%s:%d)"
			 , sip->tcp->remote_ipaddress , sip->tcp->remote_port);
	tcp_connection_send(sip->tcp , sipmsg , strlen(sipmsg));
	free(sipmsg);
#if 0
	sipmsg = (char*)malloc(1024);
	memset(sipmsg, 0 , 1024);
	if(tcp_connection_recv(sip->tcp , sipmsg , 1023) <= 0){
		debug_info("Network error occured here");
		return NULL;
	}
#endif
	sipmsg = fetion_sip_get_response(sip);

	return sipmsg;
}

char* sipc_aut_action(User* user , const char* response)
{
	char* sipmsg;
	char* xml;
	char* res;
	SipHeader* aheader = NULL;
	SipHeader* akheader = NULL;
	SipHeader* ackheader = NULL;
	FetionSip* sip = user->sip;

	debug_info("Initialize sipc authencation action");

	xml = generate_auth_body(user);
	fetion_sip_set_type(sip , SIP_REGISTER);
	aheader = fetion_sip_authentication_header_new(response);
	akheader = fetion_sip_header_new("AK" , "ak-value");
	fetion_sip_add_header(sip , aheader);
	fetion_sip_add_header(sip , akheader);
	if(user->verification != NULL && user->verification->algorithm != NULL)	{
		ackheader = fetion_sip_ack_header_new(user->verification->code
											, user->verification->algorithm
											, user->verification->type
											, user->verification->guid);
		fetion_sip_add_header(sip , ackheader);
	}
	sipmsg = fetion_sip_to_string(sip , xml);
	debug_info("Start sipc authentication , with ak-value");

	tcp_connection_send(sip->tcp , sipmsg , strlen(sipmsg));
	res = fetion_sip_get_response(sip);
	debug_info("Got sipc response");
	//free(sipmsg);
	return res;
}

void parse_ssi_auth_response(const char* ssi_response , User* user)
{
	xmlDocPtr doc;
	xmlNodePtr node;
	char* pos;
	char* xml = strstr(ssi_response , "\r\n\r\n") + 4;

	if(strstr(ssi_response , "ssic=")){
		int n;
		pos = strstr(ssi_response , "ssic=") + 5;
		n = strlen(pos) - strlen(strstr(pos , ";"));
		user->ssic = (char*)malloc(n + 1);
		memset(user->ssic, 0, n + 1);
		strncpy(user->ssic , pos , n);
	}

	doc = xmlReadMemory(xml , strlen(xml) , NULL , "UTF-8" , XML_PARSE_RECOVER);
	node = xmlDocGetRootElement(doc);
	pos = (char*)xmlGetProp(node , BAD_CAST "status-code");
	user->loginStatus = atoi(pos);
	node = node->xmlChildrenNode;
	if(atoi(pos) == 200)
	{
		debug_info("SSI login success");
		parse_ssi_auth_success(node , user);
	}
	else
	{
		debug_info("SSI login failed , status-code :%s" , pos);
		parse_ssi_auth_failed(node , user);
	}
	free(pos);
	xmlFreeDoc(doc);
}

void parse_sipc_reg_response(const char* reg_response , char** nouce , char** key)
{
	char digest[2048] = { 0 };
	char* pos;
	int n;

	fetion_sip_get_attr(reg_response , "W" , digest);

	pos = strstr(digest , "nonce") + 7;
	n = strlen(pos) - strlen(strstr(pos , "\","));
	*nouce = (char*)malloc(n + 1);
	strncpy(*nouce , pos , n);
	(*nouce)[n] = '\0';

	pos = strstr(pos , "key") + 5;
	n = strlen(pos) - strlen(strstr(pos , "\","));
	*key = (char*)malloc(n + 1);
	strncpy(*key , pos , n);
	(*key)[n] = '\0';
	debug_info("Register to sip server success");
	debug_info("nonce:%s" , *nouce);
}

static void parse_sms_frequency(xmlNodePtr node , User *user)
{
	xmlChar *res;

	node = node->xmlChildrenNode;
	if(xmlHasProp(node , BAD_CAST "day-limit")){
		res = xmlGetProp(node , BAD_CAST "day-limit");
		user->smsDayLimit = atoi((char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "day-count")){
		res = xmlGetProp(node , BAD_CAST "day-count");
		user->smsDayCount = atoi((char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "month-limit")){
		res = xmlGetProp(node , BAD_CAST "month-limit");
		user->smsMonthLimit = atoi((char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "month-count")){
		res = xmlGetProp(node , BAD_CAST "month-count");
		user->smsMonthCount = atoi((char*)res);
		xmlFree(res);
	}
}

int parse_sipc_auth_response(const char* auth_response , User* user, int *group_count, int *buddy_count)
{
	char *pos;
	xmlChar* buf = NULL;
	xmlDocPtr doc = NULL;
	xmlNodePtr rootnode = NULL;
	xmlNodePtr node = NULL;
	xmlNodePtr node1 = NULL;
	int code;

	code = fetion_sip_get_code(auth_response);
	user->loginStatus = code;

	if(code == 200){
		fetion_verification_free(user->verification);
		user->verification = NULL;
		debug_info("Sipc authentication success");
	}else if(code == 421 || code == 420){
		parse_add_buddy_verification(user , auth_response);
		return 2;
	}else{
		debug_error("Sipc authentication failed\n");
		printf("%s\n", auth_response);
		return -1;
	}
	if(!strstr(auth_response, "\r\n\r\n"))
		return -1;

	pos = strstr(auth_response , "\r\n\r\n") + 4;
	if(!pos)
		return -1;
	doc = xmlParseMemory(pos , strlen(pos));
	if(!doc)
		return -1;
	rootnode = xmlDocGetRootElement(doc); 
	node = rootnode->xmlChildrenNode;
	buf = xmlGetProp(node , BAD_CAST "public-ip");
	strcpy(user->publicIp , (char*)buf);
	xmlFree(buf);
	buf = xmlGetProp(node , BAD_CAST "last-login-ip");
	strcpy(user->lastLoginIp , (char*)buf);
	xmlFree(buf);
	buf = xmlGetProp(node , BAD_CAST "last-login-time");
	strcpy(user->lastLoginTime , (char*)buf);
	xmlFree(buf);
	node = node->next;
	node1 = node->xmlChildrenNode;
	parse_personal_info(node1 , user);
	node1 = xml_goto_node(node , "custom-config");
	buf = xmlGetProp(node1 , BAD_CAST "version");
	strcpy(user->customConfigVersion , (char*)buf);
	xmlFree(buf);
	buf = xmlNodeGetContent(node1);
	if(xmlStrlen(buf) > 0){
		user->customConfig = malloc(strlen((char*)buf) + 1);
		memset(user->customConfig , 0 , strlen((char*)buf) + 1);
		strcpy(user->customConfig , (char*)buf);
	}
	xmlFree(buf);
	node1 = xml_goto_node(node , "contact-list");
	parse_contact_list(node1 , user, group_count, buddy_count);
	node1 = xml_goto_node(node , "chat-friends");
	if(node1)
		parse_stranger_list(node1 , user);
	
	node1 = xml_goto_node(node , "quota-frequency");
	if(node1)
		parse_sms_frequency(node1 , user);
	
	xmlFreeDoc(doc);
	return 1;
}

char* generate_aes_key()
{
	char* key = (char*)malloc(65);
	if(key == NULL){
		return NULL;
	}
	memset( key , 0 , 65 );
	FILE* rand_fd = fopen("/dev/urandom", "r");
	if(rand_fd == NULL){
		free(key);
		return NULL;
	}
	int ret = fread(key, 64, 1, rand_fd);
	if(ret != 1){
		free(key);
		fclose(rand_fd);
		return NULL;
	}
	fclose(rand_fd);
	return key;
}

static char* generate_auth_body(User* user)
{
	char basexml[] = "<args></args>";
	char state[5];
	xmlChar* buf = NULL;
	xmlDocPtr doc = NULL;
	xmlNodePtr rootnode = NULL;
	xmlNodePtr node = NULL;
	xmlNodePtr node1 = NULL;

	doc = xmlParseMemory( basexml , strlen(basexml));
	rootnode = xmlDocGetRootElement(doc); 
	node = xmlNewChild(rootnode , NULL , BAD_CAST "device" , NULL);
	xmlNewProp(node , BAD_CAST "machine-code" , BAD_CAST "001676C0E351");
	node = xmlNewChild(rootnode , NULL , BAD_CAST "caps" , NULL);
	xmlNewProp(node , BAD_CAST "value" , BAD_CAST "1ff");
	node = xmlNewChild(rootnode , NULL , BAD_CAST "events" , NULL);
	xmlNewProp(node , BAD_CAST "value" , BAD_CAST "7f");
	node = xmlNewChild(rootnode , NULL , BAD_CAST "user-info" , NULL);
	xmlNewProp(node , BAD_CAST "mobile-no" , BAD_CAST user->mobileno);
	xmlNewProp(node , BAD_CAST "user-id" , BAD_CAST user->userId);
	node1 = xmlNewChild(node , NULL , BAD_CAST "personal" , NULL);
	xmlNewProp(node1 , BAD_CAST "version" , BAD_CAST "0"/*user->personalVersion*/);
	xmlNewProp(node1 , BAD_CAST "attributes" , BAD_CAST "v4default");
	node1 = xmlNewChild(node , NULL , BAD_CAST "custom-config" , NULL);
	xmlNewProp(node1 , BAD_CAST "version" , BAD_CAST "0"/*user->customConfigVersion*/);
	node1 = xmlNewChild(node , NULL , BAD_CAST "contact-list" , NULL);
	xmlNewProp(node1 , BAD_CAST "version" , BAD_CAST "0"/*user->contactVersion*/);
	xmlNewProp(node1 , BAD_CAST "buddy-attributes" , BAD_CAST "v4default");
	node = xmlNewChild(rootnode , NULL , BAD_CAST "credentials" , NULL);
	xmlNewProp(node , BAD_CAST "domains" , BAD_CAST "fetion.com.cn");
	node = xmlNewChild(rootnode , NULL , BAD_CAST "presence" , NULL);
	node1 = xmlNewChild(node , NULL , BAD_CAST "basic" , NULL);
	sprintf(state , "%d" , user->state);
	xmlNewProp(node1 , BAD_CAST "value" , BAD_CAST state);
	xmlNewProp(node1 , BAD_CAST "desc" , BAD_CAST "");
	xmlDocDumpMemory(doc , &buf , NULL);
	xmlFreeDoc(doc);
	return xml_convert(buf);
}

static void parse_personal_info(xmlNodePtr node , User* user)
{
	xmlChar *buf;
	char *pos;
	
	buf = xmlGetProp(node , BAD_CAST "version");
	strcpy(user->personalVersion , (char*)buf);
	xmlFree(buf);
	if(xmlHasProp(node , BAD_CAST "sid"))
	{
		buf = xmlGetProp(node , BAD_CAST "sid");
		strcpy(user->sId , (char*)buf);
		xmlFree(buf);
	}
	if(xmlHasProp(node , BAD_CAST "mobile-no"))
	{
		buf = xmlGetProp(node , BAD_CAST "mobile-no");
		if(xmlStrlen(buf)){
			user->boundToMobile = BOUND_MOBILE_ENABLE;
		}else{
			user->boundToMobile = BOUND_MOBILE_DISABLE;
		}
		strcpy(user->mobileno , (char*)buf);
		xmlFree(buf);
	}
	if(xmlHasProp(node , BAD_CAST "carrier-status"))
	{
		buf = xmlGetProp(node , BAD_CAST "carrier-status");
		user->carrierStatus = atoi((char*)buf);
		xmlFree(buf);
	}
	if(xmlHasProp(node , BAD_CAST "nickname"))
	{
		buf = xmlGetProp(node , BAD_CAST "nickname");
		strcpy(user->nickname , (char*)buf);
		xmlFree(buf);
	}
	if(xmlHasProp(node , BAD_CAST "gender"))
	{
		buf = xmlGetProp(node , BAD_CAST "gender");
		user->gender = atoi((char*)buf);
		xmlFree(buf);
	}
	if(xmlHasProp(node , BAD_CAST "sms-online-status"))
	{
		buf = xmlGetProp(node , BAD_CAST "sms-online-status");
		strcpy(user->smsOnLineStatus , (char*)buf);
		xmlFree(buf);
	}
	if(xmlHasProp(node , BAD_CAST "impresa"))
	{
		buf = xmlGetProp(node , BAD_CAST "impresa");
		strcpy(user->impression , (char*)buf);
		xmlFree(buf);
	}
	if(xmlHasProp(node , BAD_CAST "carrier-region"))
	{
		int n;
		buf = xmlGetProp(node , BAD_CAST "carrier-region");
		pos = (char*)buf;
		n = strlen(pos) - strlen(strstr(pos , "."));
		strncpy(user->country , pos , n);
		pos = strstr(pos , ".") + 1;
		n = strlen(pos) - strlen(strstr(pos , "."));
		strncpy(user->province , pos , n);
		pos = strstr(pos , ".") + 1;
		n = strlen(pos) - strlen(strstr(pos , "."));
		strncpy(user->city , pos , n);
		xmlFree(buf);
	}
}

static void parse_contact_list(xmlNodePtr node, User* user,
				int *group_count, int *buddy_count)
{
	xmlChar* buf = NULL;
	xmlNodePtr node1 , node2;
	Group* group = NULL;
	Contact* contact = NULL;
	int hasGroup = 1 , hasBuddy = 1;
	int nr = 0;

	*group_count = 0;
	*buddy_count = 0;

	buf = xmlGetProp(node , BAD_CAST "version");
	debug_info("Start reading contact list ");
	if(strcmp(user->contactVersion , (char*) buf) == 0){
		debug_info("Contact list is the same as that stored in the local disk!");
		return ;
	}
	strcpy(user->contactVersion , (char*)buf);
	xmlFree(buf);
	node1 = xml_goto_node(node , "buddy-lists");
	node2 = node1->xmlChildrenNode;
	user->groupCount = 0;

	while(node2 != NULL){
		hasGroup = 1;

		buf = xmlGetProp(node2 , BAD_CAST "id");
		group = fetion_group_list_find_by_id(user->groupList , atoi((char*)buf));
		if(group == NULL){
			hasGroup = 0;
			group = fetion_group_new();
		}
		group->groupid = atoi((char*)buf);
		xmlFree(buf);
		buf = xmlGetProp(node2 , BAD_CAST "name");
		strcpy(group->groupname , (char*)buf);
		xmlFree(buf);

		nr ++;
		group->dirty = 1;
		user->groupCount ++;
		
		if(hasGroup == 0){
			fetion_group_list_append(user->groupList , group);
			hasGroup = 1;
		}
		node2 = node2->next;
	}

	*group_count = nr;
	nr = 0;

	node1 = xml_goto_node(node , "buddies");
	node1 = node1->xmlChildrenNode;
	user->contactCount = 0;

	while(node1 != NULL){
		hasBuddy = 1;

		if(! xmlHasProp(node1 , BAD_CAST "i")){
			node1 = node1->next;
			continue;
		}
		buf = xmlGetProp(node1 , BAD_CAST "i");
		contact = fetion_contact_list_find_by_userid(user->contactList , (char*)buf);
		if(contact == NULL){
			hasBuddy = 0;
			contact = fetion_contact_new();
		}
		strcpy(contact->userId , (char*)buf);
		xmlFree(buf);

		/* maybe a buddy belongs to two groups */
		if(contact->dirty == 1){
			node = node->next;
			continue;
		}

		user->contactCount ++;
		/* set the dirty flags */
		contact->dirty = 1;
		nr ++;

		if(xmlHasProp(node1 , BAD_CAST "n")){
			buf = xmlGetProp(node1 , BAD_CAST "n");
			strcpy(contact->localname , (char*)buf);
			xmlFree(buf);
		}
		if(xmlHasProp(node1 , BAD_CAST "l")){
			buf = xmlGetProp(node1 , BAD_CAST "l");
			strncpy(contact->groupids, (char*)buf, sizeof(contact->groupids) - 1);
			if(strlen(contact->groupids) == 0)
				strcpy(contact->groupids, "0");
			contact->groupid = atoi((char*)buf);
			if(xmlStrstr(buf , BAD_CAST ";") != NULL
					|| contact->groupid < 0)
					contact->groupid = 0;
			xmlFree(buf);
		}
		if(xmlHasProp(node1 , BAD_CAST "p")){
			buf = xmlGetProp(node1 , BAD_CAST "p");
			if(strstr((char*)buf , "identity=1") != NULL)
				contact->identity = 1;
			else
				contact->identity = 0;
			xmlFree(buf);
		}
		if(xmlHasProp(node1 , BAD_CAST "r")){
			buf = xmlGetProp(node1 , BAD_CAST "r");
			contact->relationStatus = atoi((char*)buf);
			xmlFree(buf);
		}

		if(xmlHasProp(node1 , BAD_CAST "u")){
			buf = xmlGetProp(node1 , BAD_CAST "u");
			strcpy(contact->sipuri , (char*)buf);
			//if(strstr((char*)buf , "tel") != NULL)
			//	contact->serviceStatus = STATUS_SMS_ONLINE;
			xmlFree(buf);
		}

		strcpy(contact->portraitCrc , "0");

		if(hasBuddy == 0){
			fetion_contact_list_append(user->contactList , contact);
			hasBuddy = 1;
		}
		node1 = node1->next;
	}

	*buddy_count = nr;
	debug_info("Read contact list complete");
}

static void parse_stranger_list(xmlNodePtr node , User* user)
{
	xmlNodePtr node1 = node->xmlChildrenNode;
	xmlChar *buf = NULL;
	Contact *contact = NULL;
	int hasBuddy;
	while(node1 != NULL)
	{
		hasBuddy = 1;
		user->contactCount ++;
		buf = xmlGetProp(node1 , BAD_CAST "i");
		contact = fetion_contact_list_find_by_userid(user->contactList , (char*)buf);
		if(contact == NULL){
			hasBuddy = 0;
			contact = fetion_contact_new();
		}
		strcpy(contact->userId , (char*)buf);
		xmlFree(buf);
		buf = xmlGetProp(node1 , BAD_CAST "u");
		strcpy(contact->sipuri , (char*)buf);
		contact->groupid = BUDDY_LIST_STRANGER;
		contact->dirty = 1;

		if(hasBuddy == 0)
			fetion_contact_list_append(user->contactList , contact);
		
		node1 = node1->next;
	}
}

static void parse_ssi_auth_success(xmlNodePtr node , User* user)
{
	char* pos;
	pos = (char*)xmlGetProp(node , BAD_CAST "uri");
	strcpy(user->sipuri , pos);
	free(pos);
	pos = fetion_sip_get_sid_by_sipuri(user->sipuri);
	strcpy(user->sId , pos);
	free(pos);
	pos = (char*)xmlGetProp(node , BAD_CAST "mobile-no");
	strcpy(user->mobileno , pos);
	free(pos);
	pos = (char*)xmlGetProp(node , BAD_CAST "user-id");
	strcpy(user->userId , pos);
	free(pos);
}

static void parse_ssi_auth_failed(xmlNodePtr node , User* user)
{
	Verification* ver = fetion_verification_new();
	ver->algorithm = (char*)xmlGetProp(node , BAD_CAST "algorithm");
	ver->type      = (char*)xmlGetProp(node , BAD_CAST "type");
	ver->text      = (char*)xmlGetProp(node , BAD_CAST "text");
	ver->tips	   = (char*)xmlGetProp(node , BAD_CAST "tips");
	user->verification = ver;
}

static unsigned char* strtohex(const char* in , int* len) 
{
	unsigned char* out = (unsigned char*)malloc(strlen(in)/2 );
	int i = 0 , j = 0 , k = 0 ,length = 0;
	char tmp[3] = { 0 };
	memset(out , 0 , strlen(in) / 2);
	int inlength;
	inlength=(int)strlen(in);
	while(i < inlength)
	{
		tmp[k++] = in[i++];
		tmp[k] = '\0';
		if(k == 2)
		{
			out[j++] = (unsigned char)strtol(tmp , (char**)NULL , 16);
			k = 0;
			length ++;
		}
	}
	if(len != NULL )
		*len = length;
	return out;
}

static char* hextostr(const unsigned char* in , int len) 
{
	char* res = (char*)malloc(len * 2 + 1);
	int i = 0;
	memset(res , 0 , len * 2 + 1);
	while(i < len)
	{
		sprintf(res + i * 2 , "%02x" , in[i]);
		i ++;
	};
	i = 0;
	int reslength;
	reslength=(int)strlen(res);
	while(i < reslength)
	{
		res[i] = toupper(res[i]);
		i ++;
	};
	return res;
}

static char* hash_password_v1(const unsigned char* b0 , int b0len , const unsigned char* password , int psdlen) 
{
	unsigned char* dst = (unsigned char*)malloc(b0len + psdlen + 1);
	unsigned char tmp[20];
	char* res;
	memset(tmp , 0 , sizeof(tmp));
	memset(dst , 0 , b0len + psdlen + 1);
	memcpy(dst , b0 , b0len);
	memcpy(dst + b0len , password , psdlen);
	SHA_CTX ctx;
	SHA1_Init(&ctx);
	SHA1_Update(&ctx , dst , b0len + psdlen );
	SHA1_Final(tmp , &ctx);
	free(dst);
	res = hextostr(tmp , 20);
	return res;
}

static char* hash_password_v2(const char* userid , const char* passwordhex) 
{
	int id = atoi(userid);
	char* res;
	unsigned char* bid = (unsigned char*)(&id);
	unsigned char ubid[4];
	int bpsd_len;
	unsigned char* bpsd = strtohex(passwordhex , &bpsd_len);
	memcpy(ubid , bid , 4);
	res = hash_password_v1(ubid , sizeof(id) , bpsd , bpsd_len);
	free(bpsd);
	return res;
}

static char* hash_password_v4(const char* userid , const char* password)
{
	const char* domain = "fetion.com.cn:";
	char *res , *dst;
	unsigned char* udomain = (unsigned char*)malloc(strlen(domain));
	unsigned char* upassword = (unsigned char*)malloc(strlen(password));
	memset(udomain , 0 , strlen(domain));
	memcpy(udomain , (unsigned char*)domain , strlen(domain));
	memset(upassword , 0 , strlen(password));
	memcpy(upassword , (unsigned char*)password , strlen(password));
	res = hash_password_v1(udomain , strlen(domain) , upassword , strlen(password));
	free(udomain);
	free(upassword);
	if(userid == NULL || *userid == '\0') return res;
	dst = hash_password_v2(userid , res);
	free(res);
	return dst;
}

char* hash_password(const char* password)
{
	const char* domain = "fetion.com.cn:";
	char *res;
	unsigned char* udomain = (unsigned char*)malloc(strlen(domain));
	unsigned char* upassword = (unsigned char*)malloc(strlen(password));
	memset(udomain , 0 , strlen(domain));
	memcpy(udomain , (unsigned char*)domain , strlen(domain));
	memset(upassword , 0 , strlen(password));
	memcpy(upassword , (unsigned char*)password , strlen(password));
	res = hash_password_v1(udomain , strlen(domain) , upassword , strlen(password));
	free(udomain);
	free(upassword);
	return res;
}

/* login with the hashed password stored locally */
static char *hash_password_v5(const char *userid, const char *hashed_password)
{
	char *res, *dst;
	int len = 0;

	if(userid == NULL || *userid == '\0') {
		len = strlen(hashed_password);
		res = (char*)malloc(len + 1);
		strncpy(res, hashed_password, len);
		res[len] = '\0';
		return res;
	}

	dst = hash_password_v2(userid, hashed_password);
	return dst;
}

static char* generate_cnouce()
{
	char* cnouce = (char*)malloc(33);
	memset( cnouce , 0 , 33 );
	sprintf( cnouce , "%04X%04X%04X%04X%04X%04X%04X%04X" , 
			rand() & 0xFFFF , rand() & 0xFFFF , 
			rand() & 0xFFFF , rand() & 0xFFFF ,
			rand() & 0xFFFF , rand() & 0xFFFF,
			rand() & 0xFFFF , rand() & 0xFFFF );
	return cnouce;
}

static unsigned char* decode_base64(const char* in , int* len)
{
 	unsigned int n , t = 0 , c = 0;
	unsigned char* res;
	unsigned char out[3];
	unsigned char inp[4];

	n = strlen(in);
	if(n % 4 != 0) {
		debug_error("Try to decode a string which is not a base64 string(decode_base64)");
		return NULL;
	}
	n = n / 4 * 3;
	if(len != NULL)
		*len = n;
	res = (unsigned char*)malloc(n);
	memset(res , 0 , n);
	while(1)
	{
		memset(inp , 0 , 4);
		memset(out , 0 , 3);
		memcpy(inp , in + c , 4);
		c += 4;
		n = EVP_DecodeBlock(out , inp , 4 );
		memcpy(res + t , out , n);
		t += n;
		if(c >= strlen(in))
			break;
	}
	return res;
}

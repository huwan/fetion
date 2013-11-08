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
#include <openssl/md5.h>
#include <libgen.h>

#define GUID "9741dc51-43d3-448b-bfc2-dbf4661a27f7"
#define SESSIONID "xz4BBcV9741dc5143d3448bbfc2dbf4661a27f7"

Share *fetion_share_new(const char *sipuri)
{
	Share *share = (Share*)malloc(sizeof(Share));

	memset(share , 0 , sizeof(Share));

	strcpy(share->sipuri , sipuri);

	return share;
}


Share *fetion_share_new_with_path(const char *sipuri , const char *absolutePath)
{
	Share *share = (Share*)malloc(sizeof(Share));
	char *name = NULL;
	char *md5 = NULL;

	memset(share , 0 , sizeof(Share));

	strcpy(share->guid , GUID);
	strcpy(share->sessionid , SESSIONID);	
	strcpy(share->sipuri , sipuri);
	strcpy(share->absolutePath , absolutePath);

	name = basename((char*)absolutePath);
	strcpy(share->filename , name);
	share->filesize = fetion_share_get_filesize(absolutePath);
	md5 = fetion_share_compute_md5(absolutePath);
	strcpy(share->md5 , md5);
	free(md5);
	return share;
}

static char* generate_share_request_body(Share *share)
{
	xmlChar *buf = NULL;
	xmlDocPtr doc;
	xmlNodePtr node , fnode , root;
	char size[16];
	char body[] = "<share-content></share-content>";
	doc = xmlParseMemory(body , strlen(body));
	root = xmlDocGetRootElement(doc);
	xmlNewProp(root , BAD_CAST "id" , BAD_CAST share->guid);
	node = xmlNewChild(root , NULL , BAD_CAST "caps" , NULL);
	xmlNewProp(node , BAD_CAST "modes" , BAD_CAST "block;relay;p2p;p2pV2;relayV2;p2pV3;scV2");
	xmlNewProp(node , BAD_CAST "max-size" , BAD_CAST "2097151");
	node = xmlNewChild(root , NULL , BAD_CAST "client" , NULL);
	xmlNewProp(node , BAD_CAST "outer-ip" , BAD_CAST "");
	xmlNewProp(node , BAD_CAST "inner-ip" , BAD_CAST "59.64.128.102:1429;");
	xmlNewProp(node , BAD_CAST "port" , BAD_CAST "443");
	node = xmlNewChild(root , NULL , BAD_CAST "fileinfo" , NULL);
	fnode = xmlNewChild(node , NULL , BAD_CAST "transmit" , NULL);
	xmlNewProp(fnode , BAD_CAST "type" , BAD_CAST "p2p");
	xmlNewProp(fnode , BAD_CAST "session-id" , BAD_CAST share->sessionid);
	fnode = xmlNewChild(node , NULL , BAD_CAST "file" , NULL);
	xmlNewProp(fnode , BAD_CAST "name" , BAD_CAST share->filename);
	memset(size, 0, sizeof(size));
	xmlNewProp(fnode , BAD_CAST "size" , BAD_CAST size);
	xmlNewProp(fnode , BAD_CAST "url" , BAD_CAST "");
	xmlNewProp(fnode , BAD_CAST "md5" , BAD_CAST share->md5);
	xmlNewProp(fnode , BAD_CAST "id" , BAD_CAST share->guid);
	xmlNewProp(fnode , BAD_CAST "p2ptorelay" , BAD_CAST "1");
	xmlNewProp(fnode , BAD_CAST "file-type" , BAD_CAST "unknown");
	xmlDocDumpMemory(doc , &buf , NULL);
	xmlFreeDoc(doc);
	return xml_convert(buf);
}

static void fetion_start_transfer(FetionSip *sip){
	
	SipHeader *kheader = NULL;
	SipHeader *theader = NULL;
	char *res = NULL;
	char buf[2048];

	fetion_sip_set_type(sip , SIP_SERVICE);
	
	kheader = fetion_sip_header_new("N" , "StartTransfer");
	theader = fetion_sip_header_new("XI" , SESSIONID);
	fetion_sip_add_header(sip , kheader);
	fetion_sip_add_header(sip , theader);
	res = fetion_sip_to_string(sip , NULL);
	tcp_connection_send(sip->tcp , res , strlen(res));
	free(res);
	memset(buf, 0, sizeof(buf));
	tcp_connection_recv(sip->tcp , buf , sizeof(buf));
	printf("%s\n" , buf);
}

void fetion_share_request(FetionSip *sip , Share *share)
{
	SipHeader *kheader = NULL;
	SipHeader *theader = NULL;
	char *res = NULL;
	char *body = NULL;
	char buf[2048];

	fetion_sip_set_type(sip , SIP_OPTION);
	
	kheader = fetion_sip_header_new("K" , "ShareContent");
	theader = fetion_sip_header_new("T" , share->sipuri);
	fetion_sip_add_header(sip , kheader);
	fetion_sip_add_header(sip , theader);
	body = generate_share_request_body(share);
	res = fetion_sip_to_string(sip , body);
	tcp_connection_send(sip->tcp , res , strlen(res));
	free(res);
	memset(buf, 0, sizeof(buf));
	tcp_connection_recv(sip->tcp , buf , sizeof(buf));
	printf("%s\n" , buf);

	fetion_start_transfer(sip);
}

char* fetion_share_compute_md5(const char *absolutePath)
{
	MD5_CTX ctx;
	FILE *file;
	unsigned char input[1024];
	unsigned char output[16];
	int n , i = 0;
	char* res = (char*)malloc(33);

	file = fopen(absolutePath , "r");
	MD5_Init(&ctx);
	while(1)
	{
		n = fread(input ,  1 , sizeof(input) , file);
		if(n == 0)
			break;
		MD5_Update(&ctx , input , n);
	}
	MD5_Final(output , &ctx);
	memset(res, 0, 33);
	while(i < 16)
	{
		sprintf(res + i * 2 , "%02x" , output[i]);
		i ++;
	};
	return res;
}

long long fetion_share_get_filesize(const char *absolutePath)
{
	struct stat sb;

	if(stat(absolutePath , &sb) == -1)
	{
		debug_error("Can not get the file size");
		return -1;
	}

	return (long long)(sb.st_size);

}

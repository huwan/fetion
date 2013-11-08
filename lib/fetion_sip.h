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

#ifndef FETION_SIP_H
#define FETION_SIP_H
#define SIP_BUFFER_SIZE 2048

typedef enum 
{
	SIP_REGISTER = 1 ,
	SIP_SERVICE ,
 	SIP_SUBSCRIPTION , 
	SIP_NOTIFICATION ,
	SIP_INVITATION , 
	SIP_INCOMING , 
	SIP_OPTION , 
	SIP_MESSAGE ,
	SIP_SIPC_4_0 ,
	SIP_ACKNOWLEDGE ,
	SIP_UNKNOWN
} SipType;

typedef enum
{
	NOTIFICATION_TYPE_PRESENCE ,
	NOTIFICATION_TYPE_CONTACT ,
	NOTIFICATION_TYPE_CONVERSATION ,
	NOTIFICATION_TYPE_REGISTRATION ,
	NOTIFICATION_TYPE_SYNCUSERINFO ,
	NOTIFICATION_TYPE_PGGROUP ,
	NOTIFICATION_TYPE_UNKNOWN
} NotificationType;

typedef enum
{
	NOTIFICATION_EVENT_PRESENCECHANGED ,
	NOTIFICATION_EVENT_ADDBUDDYAPPLICATION ,
	NOTIFICATION_EVENT_USERLEFT ,
	NOTIFICATION_EVENT_DEREGISTRATION , 
	NOTIFICATION_EVENT_SYNCUSERINFO ,
	NOTIFICATION_EVENT_PGGETGROUPINFO , 
	NOTIFICATION_EVENT_UNKNOWN
} NotificationEvent;

typedef enum
{
	SIP_EVENT_PRESENCE = 0,
	SIP_EVENT_SETPRESENCE ,
	SIP_EVENT_CONTACT ,
	SIP_EVENT_CONVERSATION ,
	SIP_EVENT_CATMESSAGE ,
	SIP_EVENT_SENDCATMESSAGE ,
	SIP_EVENT_STARTCHAT ,
	SIP_EVENT_INVITEBUDDY ,
	SIP_EVENT_GETCONTACTINFO ,
	SIP_EVENT_CREATEBUDDYLIST ,
	SIP_EVENT_DELETEBUDDYLIST ,
	SIP_EVENT_SETCONTACTINFO ,
	SIP_EVENT_SETUSERINFO ,
	SIP_EVENT_SETBUDDYLISTINFO ,
	SIP_EVENT_DELETEBUDDY ,
	SIP_EVENT_ADDBUDDY ,
	SIP_EVENT_KEEPALIVE ,
	SIP_EVENT_DIRECTSMS ,
	SIP_EVENT_SENDDIRECTCATSMS ,
	SIP_EVENT_HANDLECONTACTREQUEST ,
	SIP_EVENT_PGGETGROUPLIST ,
	SIP_EVENT_PGGETGROUPINFO , 
	SIP_EVENT_PGGETGROUPMEMBERS ,
	SIP_EVENT_PGSENDCATSMS , 
	SIP_EVENT_PGPRESENCE
} SipEvent;

typedef enum
{
	INCOMING_NUDGE ,
	INCOMING_SHARE_CONTENT ,
	INCOMING_INPUT,
	INCOMING_UNKNOWN
} IncomingType;

typedef enum
{
	INCOMING_ACTION_ACCEPT ,
	INCOMING_ACTION_CANCEL , 
	INCOMING_ACTION_UNKNOWN
} IncomingActionType;

/**
 * create a FetionSip object and initialize it 
 * @param tcp The tcp connection object that FetionSip used to send and recv message
 * @param sid Login user`s fetion number
 * @return The FetionSip object created;
 */
extern FetionSip* fetion_sip_new(FetionConnection* tcp , const char* sid);

/**
 * clone a new FetionSip object with the existed FetionSip object
 * @param sip The FetionSip object to be cloned from
 * @return The new FetionSip object just cloned
 */
extern FetionSip* fetion_sip_clone(FetionSip* sip);

/**
 * create a sip header in type of in the form of "T:sip:*****@fetion.com.cn"
 * @note this function can just be used when you want to exannd the 
 * fetion protocal stack,otherwise just call the protocal functions ,
 * they can meet most of your demand.
 * @param name The name of the header , in the example above , name is "T"
 * @param value The value of the header , int the example above , value is "sip:*****@fetion.con.cn"
 * @return The sip header just created
 */
extern SipHeader* fetion_sip_header_new(const char* name , const char* value);

/**
 * set the type of sip , only need to be called before 
 * constructing a new sip request message
 * such as message with type 'M' , "I" , "S",etc...
 * @note this function can just be used when you want to expand the 
 * fetion protocal stack,otherwise just call the protocal functions ,
 * they can meet most of your demand.
 * @param sip The FetionSip object to be set.
 * @param type The new sip type.
 */
extern void fetion_sip_set_type(FetionSip* sip , SipType type);

/**
 * set the sip`s callid with the given callid instead of using the autoincrement callid
 * @note this function can just be used when you want to exannd the 
 * fetion protocal stack,otherwise just call the protocal functions ,
 * they can meet most of your demand.
 * @param sip The sip object to be sest.
 * @param callid The new callid
 */
extern void fetion_sip_set_callid(FetionSip* sip , int callid);

/** 
 * @note this function can just be used when you want to expand the 
 * fetion protocal stack,otherwise just call the protocal functions ,
 * they can meet most of your demand.
 */
extern SipHeader* fetion_sip_authentication_header_new(const char* response);

/** 
 * @note this function can just be used when you want to expand the 
 * fetion protocal stack,otherwise just call the protocal functions ,
 * they can meet most of your demand.
 */
extern SipHeader* fetion_sip_ack_header_new(const char* code
		, const char* algorithm , const char* type , const char* guid);

/** 
 * @note this function can just be used when you want to expand the 
 * fetion protocal stack,otherwise just call the protocal functions ,
 * they can meet most of your demand.
 */
extern SipHeader* fetion_sip_event_header_new(int eventType);

/** 
 * @note this function can just be used when you want to expand the 
 * fetion protocal stack,otherwise just call the protocal functions ,
 * they can meet most of your demand.
 */
extern SipHeader* fetion_sip_credential_header_new(const char* credential);

/** 
 * @note this function can just be used when you want to expand the 
 * fetion protocal stack,otherwise just call the protocal functions ,
 * they can meet most of your demand.
 */
extern void fetion_sip_add_header(FetionSip* sip , SipHeader* header);

/** 
 * @note this function can just be used when you want to expand the 
 * fetion protocal stack,otherwise just call the protocal functions ,
 * they can meet most of your demand.
 */
extern char* fetion_sip_to_string(FetionSip* sip , const char* body);

/**
 * free the resource of the sip object after use
 */
extern void fetion_sip_free(FetionSip* sip);

/**
 * A very commonly used function.It convert the sipuri to sid
 * @param sipuri The sip uri to get sid from
 * @return sid get from the sipuri. Need to be freed after use.
 */
extern char* fetion_sip_get_sid_by_sipuri(const char* sipuri);

/**
 * Another very commonly used function.It convert the fetion group uri
 * to sid , when pgsipuri is 'sip:PG444444@fetion.com.cn' , it returns 444444
 * @return Need to be freed after use.
 */
extern char* fetion_sip_get_pgid_by_sipuri(const char *pgsipuri);

/**
 * get the attribute value with the given attribute name in a sip message
 * @param sipmsg The sip message to be parsed 
 * @param name The attribute name
 * @param retulst The attribute value
 * @return 1 if success , or else -1
 */
extern int fetion_sip_get_attr(const char* sipmsg
		, const char* name , char* result);

/**
 * get the length of the sip message body which is in form of xml
 * @return the length of the xml body , 0 if no xml body
 */
extern int fetion_sip_get_length(const char* sipmsg);

/**
 * get the code of the reponse sip message , for example
 * the input message is "SIP-C/4.0 200 OK ...." , it returns 200
 */
extern int fetion_sip_get_code(const char* sip);

/**
 * get the type of the sip message 
 */
extern int fetion_sip_get_type(const char* sip);

/** 
 * @note this function can just be used when you want to expand the 
 * fetion protocal stack,otherwise just call the protocal functions ,
 * they can meet most of your demand.
 */
extern void fetion_sip_get_auth_attr(const char* auth , char** ipaddress
		, int* port , char** credential);

/**
 * get the response message when a request message already sent
 * @return The response message sent from the sipc server,need to be freed after use.
 */
extern char* fetion_sip_get_response(FetionSip* sip);

/**
 * set a new tcp connection object for the specified sip object
 */
extern void fetion_sip_set_connection(FetionSip* sip
		, FetionConnection* conn);

/**
 * listen the sipc message channel,and returns the sip message 
 * pushed from the server.
 * @note this function should be put at a loop
 * @return the SipMsg object which contains the message related information
 */
extern SipMsg* fetion_sip_listen(FetionSip* sip, int *error);

extern int fetion_sip_keep_alive(FetionSip* sip);

extern void fetion_sip_message_free(SipMsg* msg);

extern void fetion_sip_message_append(SipMsg* msglist , SipMsg* msg);

extern void fetion_sip_parse_notification(const char* sip 
		, int* type , int* event , char** xml);

extern void fetion_sip_parse_message(FetionSip* sip
		, const char* sipmsg , Message** msg);

extern void fetion_sip_parse_invitation(FetionSip* sip
		, Proxy *proxy, const char* sipmsg
		, FetionSip** conversionSip , char** sipuri);

extern void fetion_sip_parse_addbuddyapplication(const char* sipmsg
		, char** sipuri	, char** userid
		, char** desc , int* phrase);

extern void fetion_sip_parse_incoming(FetionSip* sip
		, const char* sipmsg , char** sipuri
		, IncomingType* type , IncomingActionType *action);

extern void fetion_sip_parse_userleft(const char* sipmsg , char** sipuri);

extern int fetion_sip_parse_shareaccept(FetionSip *sip 
		, const char* sipmsg , Share *share);

extern void fetion_sip_parse_sysmsg(const char* sipmsg , int *type
		, int *showonce , char **content , char **url);

extern int fetion_sip_parse_sipc(const char *sipmsg , int *callid , char **xml);

extern struct tm convert_date(const char* date);

extern inline void fetion_sip_set_conn(FetionSip *sip, FetionConnection *conn);
#endif

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

#ifndef FETION_TYPES_H
#define FETION_TYPES_H


/* inline function to trace program track */

#ifdef UNUSED
#elif defined(__GNUC__)
# 	define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#else
#	define UNUSED(x) x
#endif

#define FETION_NAME "OpenFetion"
#define FETION_VERSION ""
#define PROTO_VERSION "4.3.0980"
#define NAVIGATION_URI "nav.fetion.com.cn"
#define PGGROUP_SPACE_URI "http://group.feixin.10086.cn/space/Index/"
#define LOGIN_TYPE_FETIONNO    			1
#define LOGIN_TYPE_MOBILENO    			0
#define BOUND_MOBILE_ENABLE    			1
#define BOUND_MOBILE_DISABLE   			0
#define BASIC_SERVICE_NORMAL   			1
#define BASIC_SERVICE_ABNORMAL 			0
#define CARRIER_STATUS_OFFLINE			-1
#define CARRIER_STATUS_NORMAL  			0
#define CARRIER_STATUS_DOWN    			1
#define CARRIER_STATUS_CLOSED           2 
#define RELATION_STATUS_AUTHENTICATED   1
#define RELATION_STATUS_UNAUTHENTICATED 0

#define SERVICE_DOWN_MESSAGE "您目前无法使用此功能\n\n您的手机已停机，无法使\n用手机相关功能，请缴费\n后重试"
/**
 * some other buddylists
 */
typedef enum {
	BUDDY_LIST_NOT_GROUPED = 0 ,
	BUDDY_LIST_STRANGER =   -1 ,
	BUDDY_LIST_PGGROUP =      -2
} BuddyListType;

/**
 * Presence states
 */
typedef enum {
	P_ONLINE = 		 400 , 
	P_RIGHTBACK = 	 300 ,
	P_AWAY = 		 100 ,
	P_BUSY = 		 600 ,
	P_OUTFORLUNCH =  500 ,
	P_ONTHEPHONE = 	 150 ,
	P_MEETING = 	 850 ,
	P_DONOTDISTURB = 800 ,
	P_HIDDEN = 		 0 ,
	P_OFFLINE =      -1
} StateType;

/**
 * Type used to indicate whether user`s portrait has been changed
 */
typedef enum {
	IMAGE_NOT_INITIALIZED = -1 ,		/* portrait has not been initialized */
	IMAGE_NOT_CHANGED ,					/* portrait does not change 		 */
	IMAGE_CHANGED ,						/* portrait has been changed 		 */
	IMAGE_ALLREADY_SET
} ImageChangedType;

/**
 * Type to indicate user`s service status 
 */
typedef enum {
	STATUS_NORMAL = 1 ,					/* normal status											 */
	STATUS_OFFLINE ,					/* user offline , deleted you from his list or out of service*/
	STATUS_NOT_AUTHENTICATED ,			/* user has not accept your add buddy request				 */
	STATUS_SMS_ONLINE ,					/* user has not start fetion service						 */
	STATUS_REJECTED ,					/* user rejected your add buddy request,wait for deleting 	 */
	STATUS_SERVICE_CLOSED , 			/* user has closed his fetion service 						 */
	STATUS_NOT_BOUND					/* user doesn`t bound fetion number to a mobile number 		 */
} StatusType;

/**
 * Two-way linked list that can contans any types
 */
typedef struct fxlist {
	struct fxlist *pre;
	void          *data;
	struct fxlist *next;
} FxList;

/**
 * Fetion Connection
 */
typedef struct {
	int socketfd;						/* socket file descriptor*/
	char local_ipaddress[16];			/* local ip address      */
	int local_port;						/* local port			 */
	char remote_ipaddress[16];			/* remote ip address	 */
	int remote_port;					/* remote port 			 */
	SSL* ssl;							/* SSL handle			 */
	SSL_CTX* ssl_ctx;					/* SSL ctx struct 		 */
} FetionConnection;

/**
 * Sip header that in form of "name: value" such as "AK: ak-value"
 */
typedef struct sipheader {
	char              name[8];			/* sip header namne*/
	char             *value;			/* sip header value*/
	struct sipheader *next;				/* next sip header */
} SipHeader;

/**
 * Sip type include some common attributes
 */
typedef struct {
	int type;							/* sip message type						  */
	char from[20];						/* sender`s fetion no ,in sip it`s "F: "  */
	int callid;
	int sequence;						/* sequence number , in sip it`s "Q: "    */
	int threadCount;					/* listening threads count using this sip */
	char sipuri[48];					/* outer sipuri used when listening       */
	SipHeader* header;					/* some othre header list				  */
	FetionConnection* tcp;				/* fetion connection used to send message */
} FetionSip;

/**
 * Sip message list that parsed from received chunk 
 */
typedef struct sipmsg {
	char          *message;
	struct sipmsg *next;
} SipMsg;

/**
 * Contact lists information (Two-way linked list) 
 */
typedef struct contact {
	char userId[16];					/* userid used since v4 protocal      				*/
	char sId[16];						/* fetion no					      				*/
	char sipuri[48];					/* sipuri like 'sip:100@fetion.com.cn'				*/
	char localname[256];					/* name set by yourself				  				*/
	char nickname[256];					/* user`s nickname					    			*/
	char impression[2048];				/* user`s mood phrase				    			*/
	char mobileno[12];					/* mobile phone number				    			*/
	char devicetype[10];				/* user`s client type , like PC and J2ME,etc		*/
	char portraitCrc[12];				/* a number generated by crc algorithm 				*/
	char birthday[16];					/* user`s bitrhday									*/
	char country[6];					/* user`s country`s simplified form,like CN 		*/
	char province[6];					/* user`s province`s simplified form,like bj		*/
	char city[6];						/* user`s city`s code ,like 10 for beijing			*/
	int identity;						/* whethere to show mobileno to this user   		*/
	int scoreLevel;						/* user`s score level,unused now					*/
	int serviceStatus;					/* basic service status 							*/
	int carrierStatus;
	int relationStatus;
	char carrier[16];
	StateType state;					/* state type like online,busy,etc					*/
	char groupids[48];
	int groupid;						/* buddylist id										*/
	int gender;							/* gender 1 for male 2 for female,0 for private		*/
	int imageChanged;					/* whether user`s portrait has changed				*/
	int dirty;                          /* whether the contact just read from the server is 
										   newer than that int the local disk */
	struct contact* next;
	struct contact* pre;
} Contact;

/**
 * Buddy lists information (Two-way linked list)
 */
typedef struct group {
	char groupname[32];					/* current buddy list name  */
	int groupid;						/* current buddy list Id	*/
	int dirty;
	struct group *next;
	struct group *pre;
} Group;

typedef struct pggroupmember {
	char sipuri[64];
	char nickname[256];
	char clientType[64];
	char userId[16];
	int state;
	int identity;
	int getContactInfoCallId;
	Contact *contact;
	struct pggroupmember *next;
	struct pggroupmember *pre;
} PGGroupMember;

typedef struct pggroup {
	char pguri[64];
	char name[256];
	int statusCode;
	int category;
	int currentMemberCount;
	int limitMemberCount;
	int groupRank;
	int maxRank;
	int identity;

	int hasAcked;
	int hasDetails;
	int hasImage;
	int inviteCallId;
	int getMembersCallId;

	char createTime[48];
	char bulletin[1024];
	char summary[1024];
	char getProtraitUri[1024];
	PGGroupMember *member;
	struct pggroup *next;
	struct pggroup *pre;
} PGGroup;



/**
 * Verification information used for picture code confirm
 */
typedef struct {
	char *algorithm;
	char *type;
	char *text;
	char *tips;
	char *code;
	char *guid;
} Verification;

/**
 * User list store in local data file  (One-way linked list)
 */
struct userlist {
	char no[24];						/* fetion no or mobile no  		*/
	char password[48];					/* password 			   		*/
	char userid[48];
	char sid[48];
	int laststate;						/* last state when logining		*/
	int islastuser;						/* is the last logined user		*/
	struct userlist *pre;
	struct userlist *next;				/* next User node				*/
};

/**
 * structure used to describe global proxy information
 */
typedef struct {
	int proxyEnabled;					/* whether http proxy is enable							  */
	char proxyHost[48];					/* proxy host name or ip address						  */
	int proxyPort;					/* port number of proxy server							  */
	char proxyUser[48];					/* username for proxy authentication					  */
	char proxyPass[48];					/* password for proxy authentication 					  */
} Proxy;

/* for close action */
#define CLOSE_ALERT_ENABLE   0
#define CLOSE_ALERT_DISABLE  1
/* for nitification action */
#define MSG_ALERT_ENABLE   0
#define MSG_ALERT_DISABLE  1
/**
 * Configuration information 
 */
typedef struct {
	char globalPath[256];				/* global path,default $(HOME)/.openfetion                */
	char userPath[256];					/* user path , directory name by user`s sid in globalPath */
	char iconPath[256];					/* path stores user`s friend portraits in user`s path     */	
	char sipcProxyIP[20];				/* sipc proxy server`s ip ,read from configuration.xml    */
	int sipcProxyPort;					/* sipc proxy server`s port , read from configuration.xml */
	char portraitServerName[48];		/* portrait server`s hostname ,read from configuration.xml*/
	char portraitServerPath[32];		/* portrait server`s path , such as /HD_POOL8             */
	int iconSize;						/* portrait`s display size default 25px					  */
	int closeAlert;						/* whether popup an alert when quiting					  */
	int autoReply;						/* whether auto reply enabled							  */
	int isMute;
	char autoReplyMessage[180];			/* auto reply message content							  */
	int msgAlert;
	int autoPopup;						/* whether auto pupup chat dialog enabled				  */
	int sendMode;						/* press enter to send message or ctrl + enter 			  */
	int closeMode;						/* close button clicked to close window or iconize it	  */
	int canIconify;
	int allHighlight;
	int autoAway;
	int autoAwayTimeout;
	int onlineNotify;
	int closeSysMsg;
	int closeFetionShow;
	int useStatusIcon;

	int window_width;
	int window_height;
	int window_pos_x;
	int window_pos_y;

	char configServersVersion[16];		/* the version of some related servers such as sipc server	*/
	char configParametersVersion[16];
	char configHintsVersion[16];		/* the version of hints										*/

	struct userlist* ul;				/* user list stored in local data file					  */
	Proxy *proxy;						/* structure stores the global proxy information 		  */
} Config;

/**
 * User`s personal information and some related structs 
 */
typedef struct {
	char sId[16];						/* fetion number 											*/
	char userId[16];					/* user id													*/
	char mobileno[16];					/* mobile phone number										*/
	char password[48];					/* raw password not hashed									*/
	char sipuri[48];					/* sipuri like 'sip:100@fetion.com.cn'						*/
	char publicIp[32];					/* public ip of current session								*/
	char lastLoginIp[32];				/* public ip of last login									*/
	char lastLoginTime[48];				/* last login time , got after sipc authentication			*/

	char personalVersion[16];			/* the version of personal information						*/
	char contactVersion[16];			/* the version of contact information						*/
	char customConfigVersion[16];		/* the version of custom config string,unused now			*/

	char nickname[48];					/* nickname of yourself										*/
	char impression[256];				/* mood phrase of yourself									*/
	char portraitCrc[16];				/* a number generated by crc algorithm						*/
	char country[6];					/* the country which your number belongs to					*/
	char province[6];					/* the province which your number belongs to				*/
	char city[6];						/* the city which your number belongs to 					*/
	int gender;							/* the gender of your self									*/
	char smsOnLineStatus[32];

	int smsDayLimit;
	int smsDayCount;
	int smsMonthLimit;
	int smsMonthCount;

	int pgGroupCallId;					/* callid for get group list request */
	int groupInfoCallId;					/* callid for get group info request */

	int state;							/* presence state											*/
	int loginType;   					/* using sid or mobileno									*/
	int loginStatus; 					/* login status code 										*/
	int carrierStatus;
	int boundToMobile;					/* whether this number is bound to a mobile number  */
	long loginTimes;
	int contactCount;
	int groupCount;
	char* ssic;						    /* cookie string read from reply message after ssi login 	*/
	char* customConfig;					/* custom config string used to set personal information	*/
	Verification* verification;			/* a struct used to generate picture code					*/	 
	Contact* contactList;				/* friend list of current user								*/
	Group* groupList;					/* buddylist list of current user							*/
	PGGroup* pggroup;					/* group list */
	Config* config;						/* config information										*/
	FetionSip* sip;						/* sip object used to handle sip event						*/
} User;

/**
 * structure used to describe onversation information 
 */
typedef struct {
	Contact    *currentContact;			 /* current friend who you a chating with					   */
	User       *currentUser;			 /* current user,ourselves									   */
	FetionSip  *currentSip;				 /* sip struct used to send message 
										  * NULL if did not start a chat channel for this conversation */
} Conversation;

/**
 * structure used to describe message information 
 */
typedef struct {
	char      *message;						 /* message content  		*/
	char      *sipuri;						 /* sender`s sip uri 		*/
	char      *pguri;
	int        callid;
	int        sysback;
	struct tm  sendtime;					 /* message sent time 		*/
} Message;

/**
 * structure used to describe chat history
 */
typedef struct {
	char name[48];						 /* name of message sender	   */
	char userid[16];					 /* userid of message sender   */
	char sendtime[32];					 /* message sent time		   */
	char message[4096];					 /* message content			   */
	int  issend;						 /* message is sent of received*/	
} History;

typedef struct {
	User* user;
	sqlite3 *db;
} FetionHistory;

#define FILE_RECEIVE 1
#define FILE_SEND 2

#define FILE_ACCEPTED 1
#define FILE_DECLINED 2

typedef struct {

	FetionSip *sip;
	int shareMode;
	int shareState;
	char guid[64];
	char sessionid[64];
	char absolutePath[1024];
	char filename[64];
	char sipuri[64];
	char md5[64];
	long long filesize;
	char preferType[8];
	char innerIp[24];
	int innerUdpPort;
	int innerTcpPort;
	char outerIp[24];
	int outerUdpPort;
	int outerTcpPort;
} Share;

struct unacked_list {
	int timeout;
	Message *message;
	struct unacked_list *next;
	struct unacked_list *pre;
};

#endif

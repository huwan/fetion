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
#include <errno.h>
#include <unistd.h>

static int parse_configuration_xml(User *user, const char *xml);
static char* generate_configuration_body(User* user);
static void save_phrase(xmlNodePtr node, User *user);

Config* fetion_config_new()
{
	char* homepath = NULL;
	Config* config = NULL;

	homepath = getenv("HOME");
	config = (Config*)malloc(sizeof(Config));
	memset(config , 0 , sizeof(Config));

	snprintf(config->globalPath, sizeof(config->globalPath)-1, "%s/.openfetion" , homepath);
	config->globalPath[sizeof(config->globalPath)-1] = '\0';
	int e;
	e = mkdir(config->globalPath, S_IRWXU|S_IRWXO|S_IRWXG);
	if(e && access(config->globalPath,R_OK|W_OK)){
		debug_error("%s,cannot create, read or write", config->globalPath);
		free(config);
		return NULL;
	}
	config->ul = NULL;
	config->iconSize = 25;
	return config;
}

FxList* fetion_config_get_phrase(Config* config)
{
	
	char path[256];
	char sql[1024];
	sqlite3 *db;
	char **sqlres;
	int ncols, nrows, i, start;
	FxList *list, *pos;
	Phrase *phrase;

	list = fx_list_new(NULL);

	snprintf(path, sizeof(path)-1, "%s/data.db" , config->userPath);
	if(sqlite3_open(path, &db)){
		debug_error("failed to load user list");
		return list;
	}

	snprintf(sql, sizeof(sql)-1, "select * from phrases order by id desc;");
	if(sqlite3_get_table(db, sql, &sqlres, &nrows, &ncols, NULL)){
		debug_error("read phrases :%s",	sqlite3_errmsg(db));
		sqlite3_close(db);
		return list;
	}

	for(i = 0; i < nrows; i ++){
		phrase = (Phrase*)malloc(sizeof(Phrase));
		start = ncols + i * ncols;
		phrase->phraseid = atoi(sqlres[start]);
		strncpy(phrase->content, sqlres[start+1], 255);
		pos = fx_list_new(phrase);
		fx_list_append(list , pos);
	}
	
	sqlite3_free_table(sqlres);
	sqlite3_close(db);
	return list;
}
void fetion_phrase_free(Phrase* phrase)
{
	free(phrase);
}

void fetion_config_free(Config *config)
{
	if(config != NULL){
	    fetion_user_list_free(config->ul);
	    free(config);
	}
}
struct userlist* fetion_user_list_new(const char *no,
	    	const char *password, const char *userid,
		const char *sid, int laststate , int islastuser)
{
	struct userlist* ul;

	ul = (struct userlist*)malloc(sizeof(struct userlist));
	memset(ul , 0 , sizeof(struct userlist));
	if(no)
		strcpy(ul->no , no);
	if(password)
		strcpy(ul->password , password);
	if(userid)
		strcpy(ul->userid, userid);
	if(sid)
		strcpy(ul->sid, sid);
	ul->laststate = laststate;
	ul->islastuser = islastuser;
	ul->next = ul->pre = ul;
	return ul;
}

void fetion_user_list_append(struct userlist* head , struct userlist* ul)
{
	head->next->pre = ul;
	ul->next = head->next;
	ul->pre = head;
	head->next = ul;
}

void fetion_user_list_save(Config* config , struct userlist* ul)
{	
	char path[256];
	char sql[1024];
	char password[1024];
	sqlite3 *db;
	struct userlist *pos;

	memset(path , 0 , sizeof(path));
	snprintf(path, sizeof(path)-1, "%s/data.db" , config->globalPath);
	if(sqlite3_open(path, &db)){
		debug_error("failed to save user list");
		return;
	}

	snprintf(sql, sizeof(sql)-1, "delete from userlist;");
	if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
		debug_error("delete userlist failed:%s", sqlite3_errmsg(db));
		sqlite3_close(db);
		return;
	}


	foreach_userlist(ul, pos){
		snprintf(password, sizeof(password)-1, "%s", pos->password);
		escape_sql(password);
		snprintf(sql, sizeof(sql)-1, "insert into userlist values"
					"('%s','%s',%d,%d,'%s','%s')",
					pos->no, password,
					pos->laststate, pos->islastuser,
				   	pos->userid, pos->sid);
		if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
			debug_error("insert no : %s failed: %s",
					pos->no, sqlite3_errmsg(db));
			continue;
		}
	}
	sqlite3_close(db);
}

void fetion_user_list_set_lastuser_by_no(struct userlist* ul , const char* no)
{
	struct userlist* pos;
	foreach_userlist(ul , pos){
		if(strcmp(pos->no , no) == 0)
			pos->islastuser = 1;
		else
			pos->islastuser = 0;
	}
}
struct userlist* fetion_user_list_find_by_no(struct userlist* list , const char* no)
{
	struct userlist* pos;
	foreach_userlist(list , pos){
		if(strcmp(pos->no , no) == 0)
			return pos;
	}
	return NULL;
}

static int create_userlist_table(sqlite3 *db)
{
	char sql[1024];
	sprintf(sql, "create table userlist (no, password"
					", laststate, islastuser,userid,sid);");
	if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
		debug_error("create table userlist failed:%s", sqlite3_errmsg(db));
		return 1;
	}
	return 0;
}

struct userlist* fetion_user_list_load(Config* config)
{
	char path[256];
	char sql[1024];
	sqlite3 *db;
	char **sqlres;
	struct userlist *res = NULL , *pos;
	int ncols, nrows, i, start;

	res = fetion_user_list_new(NULL, NULL, NULL, NULL, 0, 0);

	snprintf(path, sizeof(path)-1, "%s/data.db" , config->globalPath);
	if(sqlite3_open(path, &db)){
		debug_error("failed to load user list");
		return res;
	}

	snprintf(sql, sizeof(sql)-1, "select sid from userlist;");
	if(sqlite3_get_table(db, sql, &sqlres, &nrows, &ncols, NULL)){
create_ul_table:
		if(create_userlist_table(db)){
			sprintf(sql, "drop table userlist;");
			if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
				goto create_ul_table;
			}
			sqlite3_close(db);
			return res;
		}
	}
	sqlite3_free_table(sqlres);
	
	snprintf(sql, sizeof(sql), "select * from userlist order by islastuser desc;");
	if(sqlite3_get_table(db, sql, &sqlres, &nrows, &ncols, NULL)){
		if(create_userlist_table(db)){
			sqlite3_close(db);
			return res;
		}
	}

	if(nrows == 0 || ncols == 0){
		goto list_load_re;
	}

	debug_info("Loading user list store in local data file");
	for(i = 0; i < nrows; i ++){
		start = ncols + i * ncols;
		pos = fetion_user_list_new(sqlres[start],
					sqlres[start + 1],
					sqlres[start + 4],
					sqlres[start + 5],
				   	atoi(sqlres[start + 2]),
					atoi(sqlres[start + 3]));
		unescape_sql(pos->password);
		fetion_user_list_append(res , pos);
	}

list_load_re:	
	sqlite3_free_table(sqlres);
	sqlite3_close(db);
	return res;
}

void fetion_user_list_update_userid(Config *config,
				const char *no, const char *userid)
{
	char path[256];
	char sql[1024];
	sqlite3 *db;

	snprintf(path, sizeof(path), "%s/data.db" , config->globalPath);
	if(sqlite3_open(path, &db)){
		debug_error("failed to load user list");
		return;
	}

	snprintf(sql, sizeof(sql), "update userlist set userid='%s' "
			"where no='%s';",userid, no);
	if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
		debug_error("update userlist:%s", sqlite3_errmsg(db));
	}
	
	sqlite3_close(db);
	return;
}

void fetion_user_list_free(struct userlist *list)
{
	struct userlist *cur;
	struct userlist *tmp;

	if(list == NULL)
	    return;

	cur = list->next;

	while(cur != list){
		tmp = cur;
		cur = cur->next;
		free(tmp);
	}
}

int fetion_config_download_configuration(User* user)
{
	char http[1025] , *body , *res;
	FetionConnection* conn = NULL;
	Config *config = user->config;
	int ret;
	char uri[] = "nav.fetion.com.cn";
	char* ip;

	ip = get_ip_by_name(uri);
	if(ip == NULL){
		debug_error("Parse configuration uri (%s) failed!!!", uri);
		return -1;
	}
	conn = tcp_connection_new();
	if(config->proxy != NULL && config->proxy->proxyEnabled)
		ret = tcp_connection_connect_with_proxy(conn , ip , 80 , config->proxy);
	else
		ret = tcp_connection_connect(conn , ip , 80);

	if(ret < 0)
		return -1;

	body = generate_configuration_body(user);
	snprintf(http, sizeof(http), "POST /nav/getsystemconfig.aspx HTTP/1.1\r\n"
				   "User-Agent: IIC2.0/PC "PROTO_VERSION"\r\n"
				   "Host: %s\r\n"
				   "Connection: Close\r\n"
				   "Content-Length: %d\r\n\r\n%s"
				 , uri , strlen(body) , body);
	ret = tcp_connection_send(conn , http , strlen(http));
	if(ret < 0)
		return -1;

	res = http_connection_get_response(conn);
	parse_configuration_xml(user, res);
	
	free(res);
	free(ip);
	free(conn);
	free(body);
	return 1;
}
int fetion_config_initialize(Config* config , const char* userid)
{

//	DIR *userdir , *icondir;
	snprintf(config->userPath, sizeof(config->userPath), "%s/%s" , config->globalPath , userid);
	snprintf(config->iconPath, sizeof(config->iconPath), "%s/icons" , config->userPath );

	int e;
	e = mkdir(config->userPath, S_IRWXU|S_IRWXO|S_IRWXG);
	if(e && access(config->userPath,R_OK|W_OK)){
		debug_error("%s,cannot create, read or write", config->userPath);
		return 1;
	}

	e = mkdir(config->iconPath, S_IRWXU|S_IRWXO|S_IRWXG);
	if(e && access(config->iconPath,R_OK|W_OK)){
		debug_error("%s,cannot create, read or write",config->iconPath);
		return 1;
	}
		
	return 0;
}

static int parse_configuration_xml(User *user, const char *xml)
{
	Config* config = user->config;
	char sipcIP[20] , sipcPort[6]; 
	char* pos;
	int n;
	xmlChar* res;
	xmlDocPtr doc;
	xmlNodePtr node;
	xmlNodePtr cnode;

	memset(sipcIP, 0, sizeof(sipcIP));
	memset(sipcPort, 0, sizeof(sipcPort));

	doc = xmlParseMemory(xml, strlen(xml));

	if(!doc){
		debug_error("Can not read configuration");
		return -1;
	}

	node = xmlDocGetRootElement(doc);
	cnode = xml_goto_node(node, "servers");
	if(cnode && xmlHasProp(cnode, BAD_CAST "version")){
		res = xmlGetProp(cnode, BAD_CAST "version");
		strcpy(config->configServersVersion, (char*)res);
		xmlFree(res);
	}
	cnode = xml_goto_node(node, "parameters");
	if(cnode && xmlHasProp(cnode, BAD_CAST "version")){
		res = xmlGetProp(cnode, BAD_CAST "version");
		strncpy(config->configParametersVersion, (char*)res, 
			sizeof(config->configParametersVersion));
		xmlFree(res);
	}
	cnode = xml_goto_node(node, "hints");
	if(cnode && xmlHasProp(cnode, BAD_CAST "version")){
		res = xmlGetProp(cnode, BAD_CAST "version");
		strncpy(config->configHintsVersion, (char*)res,
			sizeof(config->configHintsVersion));
		xmlFree(res);
	}
	cnode = xml_goto_node(node, "sipc-proxy");
	if(cnode){
		res = xmlNodeGetContent(cnode);
		n = strlen((char*)res) - strlen(strstr((char*)res , ":"));
		strncpy(config->sipcProxyIP , (char*)res , n);
		pos = strstr((char*)res , ":") + 1;
		config->sipcProxyPort = atoi(pos);
		xmlFree(res);
	}

	cnode = xml_goto_node(node , "get-uri");
	if(cnode){
		res = xmlNodeGetContent(cnode);
		pos = strstr((char*)res , "//") + 2;
		n = strlen(pos) - strlen(strstr(pos , "/"));
		strncpy(config->portraitServerName , pos , n);
		pos = strstr(pos , "/") + 1;
		n = strlen(pos) - strlen(strstr(pos , "/"));
		strncpy(config->portraitServerPath , pos , n);
		xmlFree(res);
	}
	save_phrase(node, user);
	return 1;
}

int fetion_config_load_size(Config *config)
{
	char path[256];
	char sql[4096];
	char **sqlres;
	sqlite3 *db;
	int ncols, nrows;

	sprintf(path, "%s/data.db", config->globalPath);

	if(sqlite3_open(path, &db)){
		debug_error("open data.db:%s", sqlite3_errmsg(db));
		return -1;
	}

	snprintf(sql, sizeof(sql), "select * from size;");
	if(sqlite3_get_table(db, sql, &sqlres, &nrows, &ncols, NULL)){
		sqlite3_close(db);
		return -1;
	}

	config->window_width = atoi(sqlres[ncols]);
	config->window_height = atoi(sqlres[ncols+1]);
	config->window_pos_x = atoi(sqlres[ncols+2]);
	config->window_pos_y = atoi(sqlres[ncols+3]);

	sqlite3_free_table(sqlres);
	sqlite3_close(db);
	return 1;
}

int fetion_config_save_size(Config *config)
{
	char path[256];
	char sql[4096];
	sqlite3 *db;

	snprintf(path, sizeof(path), "%s/data.db" , config->globalPath);

	if(sqlite3_open(path, &db)){
		debug_error("failed to load user list");
		return -1;
	}

	snprintf(sql, sizeof(sql), "delete from size;");
	if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
		snprintf(sql, sizeof(sql), "create table size ("
				"window_width,window_height,"
				"window_pos_x,window_pos_y);");
		if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
			debug_error("create table size:%s", sqlite3_errmsg(db));
		}
	}

	snprintf(sql, sizeof(sql), "insert into size values ("
				"%d,%d,%d,%d);",
				config->window_width,
				config->window_height,
				config->window_pos_x,
				config->window_pos_y);
	if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
		debug_error("save size:%s", sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}
	
	sqlite3_close(db);
	return 1;
}

int fetion_config_get_use_status_icon(Config *config)
{
	char path[256];
	char sql[4096];
	char **sqlres;
	sqlite3 *db;
	int ncols, nrows;

	sprintf(path, "%s/data.db", config->globalPath);

	if(sqlite3_open(path, &db)){
		debug_error("open data.db:%s", sqlite3_errmsg(db));
		return -1;
	}

	snprintf(sql, sizeof(sql), "select * from statusicon;");
	if(sqlite3_get_table(db, sql, &sqlres, &nrows, &ncols, NULL)){
		sqlite3_close(db);
		return -1;
	}

	config->useStatusIcon = atoi(sqlres[ncols]);

	sqlite3_free_table(sqlres);
	sqlite3_close(db);
	return 1;
}

int fetion_config_set_use_status_icon(Config *config)
{
	char path[256];
	char sql[4096];
	sqlite3 *db;

	snprintf(path, sizeof(path), "%s/data.db" , config->globalPath);

	if(sqlite3_open(path, &db)){
		return -1;
	}

	snprintf(sql, sizeof(sql), "delete from statusicon;");
	if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
		snprintf(sql, sizeof(sql), "create table statusicon (use);");
		if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
			debug_error("create table statusicon:%s", sqlite3_errmsg(db));
		}
	}

	snprintf(sql, sizeof(sql), "insert into statusicon values (%d);",
				config->useStatusIcon);
	if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
		debug_error("save statusicon:%s", sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}
	
	sqlite3_close(db);
	return 1;
}

int fetion_config_load(User *user)
{
	char path[256];
	char sql[4096];
	char **sqlres;
	sqlite3 *db;
	int ncols, nrows;
	Config *config = user->config;

	sprintf(path, "%s/data.db", config->userPath);

	debug_info("Load configuration");
	if(sqlite3_open(path, &db)){
		debug_error("open data.db:%s error.", sqlite3_errmsg(db));
		return -1;
	}

	sprintf(sql, "select * from config_2_2_0;");
	if(sqlite3_get_table(db, sql, &sqlres, &nrows, &ncols, NULL)){
		sqlite3_close(db);
		return -1;
	}

	strcpy(config->sipcProxyIP, sqlres[ncols]);
	config->sipcProxyPort = atoi(sqlres[ncols+1]);
	strcpy(config->portraitServerName, sqlres[ncols+2]);
	strcpy(config->portraitServerPath, sqlres[ncols+3]);
	config->iconSize = atoi(sqlres[ncols+4]);
	config->closeAlert = atoi(sqlres[ncols+5]);
	config->autoReply = atoi(sqlres[ncols+6]);
	config->isMute = atoi(sqlres[ncols+7]);
	strcpy(config->autoReplyMessage, sqlres[ncols+8]);
	config->msgAlert = atoi(sqlres[ncols+9]);
	config->autoPopup = atoi(sqlres[ncols+10]);
	config->sendMode = atoi(sqlres[ncols+11]);
	config->closeMode = atoi(sqlres[ncols+12]);
	config->canIconify = atoi(sqlres[ncols+13]);
	config->allHighlight = atoi(sqlres[ncols+14]);
	strcpy(config->configServersVersion, sqlres[ncols+15]);
	strcpy(config->configParametersVersion, sqlres[ncols+16]);
	strcpy(config->configHintsVersion, sqlres[ncols+17]);
	config->autoAway = atoi(sqlres[ncols+18]);
	config->autoAwayTimeout = atoi(sqlres[ncols+19]);
	config->onlineNotify = atoi(sqlres[ncols+20]);
	config->closeSysMsg = atoi(sqlres[ncols+21]);
	config->closeFetionShow = atoi(sqlres[ncols+22]);
	config->useStatusIcon = atoi(sqlres[ncols+23]);

	sqlite3_free_table(sqlres);
	sqlite3_close(db);

	fetion_config_get_use_status_icon(config);
	return 1;
}

int fetion_config_save(User *user)
{
	char path[256];
	char sql[4096];
	char sql1[4096];
	int count = 0;
	sqlite3 *db;
	Config *config = user->config;

	snprintf(path, sizeof(path), "%s/data.db" , config->userPath);

	debug_info("Save configuration");

	if(sqlite3_open(path, &db)){
		debug_error("failed to load user list");
		return -1;
	}

	sprintf(sql, "delete from config_2_2_0;");
	if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
recreate:
		snprintf(sql, sizeof(sql), "create table config_2_2_0 ("
				"sipcProxyIP,sipcProxyPort,"
				"portraitServerName,portraitServerPath,"
				"iconSize,closeAlert,autoReply,isMute,"
				"autoReplyMessage,msgAlert,autoPopup,"
				"sendMode,closeMode,canIconify,allHighlight,"
				"serversVersion,paremetersVersion,"
				"hintsVersion,autoAway,autoAwayTimeout,"
				"onlineNotify,closeSysMsg,closeFetionShow,useStatusIcon);");
		count ++;
		if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
			debug_error("create table config:%s",sqlite3_errmsg(db));
			if(count == 2){
				sqlite3_close(db);
				return -1;
			}
		}
	}

	sprintf(sql, "insert into config_2_2_0 values ("
				"'%s',%d,'%s','%s',%d,%d,%d,"
				"%d,'%s',%d,%d,%d,%d,%d,%d,"
				"'%s','%s','%s',%d,%d,%d,%d,%d,%d);",
				config->sipcProxyIP,
				config->sipcProxyPort,
				config->portraitServerName,
				config->portraitServerPath,
				config->iconSize,
				config->closeAlert,
				config->autoReply,
				config->isMute,
				config->autoReplyMessage,
				config->msgAlert,
				config->autoPopup,
				config->sendMode,
				config->closeMode,
				config->canIconify,
				config->allHighlight,
				config->configServersVersion,
				config->configParametersVersion,
				config->configHintsVersion,
				config->autoAway,
				config->autoAwayTimeout,
				config->onlineNotify,
				config->closeSysMsg,
				config->closeFetionShow,
				config->useStatusIcon);
	if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
		debug_error("save config:%s", sqlite3_errmsg(db));

		sprintf(sql1, "drop table config_2_2_0;");
		if(sqlite3_exec(db, sql1, NULL, NULL, NULL)){
			debug_error("drop table config:%s", sqlite3_errmsg(db));
		}
		goto recreate;
	}
	sqlite3_close(db);
	return 1;
}

Proxy* fetion_config_load_proxy()
{
	Proxy *proxy=NULL;
	sqlite3 *db;
	char **sqlres;
	char sql[1024];
	char path[1024];
	int ncols, nrows;

	snprintf(path, sizeof(path),"%s/.openfetion/data.db",getenv("HOME"));

	debug_info("Read proxy information");
	if(sqlite3_open(path, &db)){
		debug_error("open data.db:%s", sqlite3_errmsg(db));
		return NULL;
	}

	sprintf(sql, "select * from proxy;");
	
	if(sqlite3_get_table(db, sql, &sqlres, &nrows, &ncols, NULL)){
		sprintf(sql, "create table proxy ("
					"proxyEnabled, proxyHost,"
					"proxyPort, proxyUser, proxyPass);");
		if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
			debug_info("create table proxy:%s", sqlite3_errmsg(db));
		}
		goto load_proxy;
	}
	if(!nrows)
		goto load_proxy;
	
	proxy = (Proxy*)malloc(sizeof(Proxy));
	proxy->proxyEnabled = atoi(sqlres[ncols]);
	strcpy(proxy->proxyHost, sqlres[ncols+1]);
	proxy->proxyPort = atoi(sqlres[ncols+2]);
	strcpy(proxy->proxyUser, sqlres[ncols+3]);
	strcpy(proxy->proxyPass, sqlres[ncols+4]);

load_proxy:	
	sqlite3_close(db);
	return proxy;
}

void fetion_config_save_proxy(Proxy *proxy)
{
	sqlite3 *db;
	char **sqlres;
	char sql[1024];
	char path[1024];
	int ncols, nrows;

	snprintf(path, sizeof(path), "%s/.openfetion/data.db", getenv("HOME"));

	debug_info("Save proxy information");
	if(sqlite3_open(path, &db)){
		debug_error("open data.db:%s", sqlite3_errmsg(db));
		return;
	}

	sprintf(sql, "select * from proxy;");
	if(sqlite3_get_table(db, sql, &sqlres, &nrows, &ncols, NULL)){
		sprintf(sql, "create table proxy ("
					"proxyEnabled, proxyHost,"
					"proxyPort, proxyUser, proxyPass);");
		if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
			debug_error("create table proxy:%s",sqlite3_errmsg(db));
		}
		nrows = 0;
	}

	if(nrows == 0){
		snprintf(sql, sizeof(sql), "insert into proxy values("
					"%d,'%s',%d,'%s','%s');",
					proxy->proxyEnabled,
					proxy->proxyHost,
					proxy->proxyPort,
					proxy->proxyUser,
					proxy->proxyPass);
		if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
			debug_error("insert into proxy:%s",sqlite3_errmsg(db));
			return;
		}
	}else{
		snprintf(sql, sizeof(sql), "update proxy set proxyEnabled=%d,"
					"proxyHost='%s',proxyPort='%d',"
					"proxyUser='%s',proxyPass='%s';",
					proxy->proxyEnabled,
					proxy->proxyHost,
					proxy->proxyPort,
					proxy->proxyUser,
					proxy->proxyPass);
		if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
			debug_error("update proxy:%s", sqlite3_errmsg(db));
			return;
		}
	}
	
}

static char* generate_configuration_body(User* user)
{
	xmlChar* buf;
	xmlDocPtr doc;
	xmlNodePtr node , cnode;
	char body[] = "<config></config>";
	doc = xmlParseMemory(body , strlen(body));
	node = xmlDocGetRootElement(doc);
	cnode = xmlNewChild(node , NULL , BAD_CAST "user" , NULL);

	if(user->loginType == LOGIN_TYPE_FETIONNO)
		xmlNewProp(cnode , BAD_CAST "sid" , BAD_CAST user->sId);
	else
		xmlNewProp(cnode , BAD_CAST "mobile-no" , BAD_CAST user->mobileno);
	
	cnode = xmlNewChild(node , NULL , BAD_CAST "client" , NULL);
	xmlNewProp(cnode , BAD_CAST "type" , BAD_CAST "PC");
	xmlNewProp(cnode , BAD_CAST "version" , BAD_CAST PROTO_VERSION);
	xmlNewProp(cnode , BAD_CAST "platform" , BAD_CAST "W5.1");
	cnode = xmlNewChild(node , NULL , BAD_CAST "servers" , NULL);
	xmlNewProp(cnode , BAD_CAST "version",
				   	BAD_CAST user->config->configServersVersion);
	cnode = xmlNewChild(node , NULL , BAD_CAST "parameters" , NULL);
	xmlNewProp(cnode , BAD_CAST "version",
				   	BAD_CAST user->config->configParametersVersion);
	cnode = xmlNewChild(node , NULL , BAD_CAST "hints" , NULL);
	xmlNewProp(cnode , BAD_CAST "version",
				   	BAD_CAST user->config->configHintsVersion);
#if 0
	cnode = xmlNewChild(node , NULL , BAD_CAST "service-no" , NULL);
	xmlNewProp(cnode , BAD_CAST "version" , BAD_CAST "0");
	cnode = xmlNewChild(node , NULL , BAD_CAST "http-applications" , NULL);
	xmlNewProp(cnode , BAD_CAST "version" , BAD_CAST "0");
	cnode = xmlNewChild(node , NULL , BAD_CAST "client-config" , NULL);
	xmlNewProp(cnode , BAD_CAST "version" , BAD_CAST "0");
	cnode = xmlNewChild(node , NULL , BAD_CAST "services" , NULL);
	xmlNewProp(cnode , BAD_CAST "version" , BAD_CAST "0");
#endif
	xmlDocDumpMemory(doc , &buf , NULL);
	xmlFreeDoc(doc);	
	return xml_convert(buf);
}

xmlNodePtr xml_goto_node(xmlNodePtr node , const char* name)
{
	xmlNodePtr pos = node;
	xmlNodePtr tmp = NULL;
	while(pos != NULL)
	{
		if(strcmp(name , (char*)pos->name) == 0)
			return pos;
		tmp = pos->xmlChildrenNode;
		if(tmp != NULL && xmlStrcmp(tmp->name , BAD_CAST "text") != 0
		   &&tmp->type == XML_ELEMENT_NODE
		   && (tmp = xml_goto_node(tmp , name)) != NULL )
			return tmp;
		pos = pos->next;
	};
	return NULL;
}
char* xml_convert(xmlChar* in)
{
	char *res , *pos ;
	pos = strstr((char*)in , "?>") + 2;
	res = (char*)malloc(strlen(pos) + 1);
	memset(res , 0 , strlen(pos) + 1);
	memcpy(res , pos , strlen(pos));
	xmlFree(in);
	return res;
}
char* fetion_config_get_city_name(const char* province , const char* city)
{
	char path[] = RESOURCE_DIR"city.xml"; 
	xmlChar* res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlParseFile(path);
	node = xmlDocGetRootElement(doc);
	node = node->xmlChildrenNode;
	while(node != NULL)
	{
		if(node->type != XML_ELEMENT_NODE)
		{
			node = node->next;
			continue;
		}
		res = xmlGetProp(node , BAD_CAST "id");
		if(xmlStrcmp(res , BAD_CAST province) == 0)
		{
			node = node->xmlChildrenNode;
			while(node != NULL)
			{
				if(node->type != XML_ELEMENT_NODE)
				{
					node = node->next;
					continue;
				}
				xmlFree(res);
				res = xmlGetProp(node , BAD_CAST "id");
				if(xmlStrcmp(res , BAD_CAST city) == 0)
				{
					xmlFree(res);
					return (char*)xmlNodeGetContent(node);
					break;
				}
				node = node->next;
			}
			break;
		}
		xmlFree(res);
	
		node = node->next;
	}
	return NULL;
}

char* fetion_config_get_province_name(const char* province)
{
	char path[] = RESOURCE_DIR"province.xml"; 
	xmlChar* res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlReadFile(path , "UTF-8" , XML_PARSE_RECOVER);
	node = xmlDocGetRootElement(doc);
	node = node->xmlChildrenNode;
	while(node != NULL)
	{
		res = xmlGetProp(node , BAD_CAST "id");
		if(xmlStrcmp(res , BAD_CAST province) == 0)
		{
			return (char*)xmlNodeGetContent(node);
			xmlFree(res);
			break;
		}
		xmlFree(res);
		node = node->next;
	}
	xmlFreeDoc(doc);
	return NULL;
}

static void save_phrase(xmlNodePtr node, User *user)
{
	char path[256];
	char sql[4096];
	sqlite3 *db;
	xmlChar *res, *res1;
	Config *config = user->config;

	node = xml_goto_node(node , "addbuddy-phrases");
	if(!node)
		return;

	snprintf(path, sizeof(path), "%s/data.db",config->userPath);

	debug_info("Load user information");
	if(sqlite3_open(path, &db)){
		debug_error("open data.db:%s",sqlite3_errmsg(db));
		return;
	}

	snprintf(sql, sizeof(sql),"delete from phrases;");
	if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
		sprintf(sql, "create table phrases (id,content);");
		if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
			debug_error("create table phrase:%s", sqlite3_errmsg(db));
			sqlite3_close(db);
			return;
		}
	}
	node = node->xmlChildrenNode;
	while(node){
		res = xmlNodeGetContent(node);
		res1 = xmlGetProp(node , BAD_CAST "id");
		snprintf(sql, sizeof(sql),"insert into phrases values (%s,'%s');", 
				(char*)res1, (char*)res);
		xmlFree(res);
		xmlFree(res1);
		if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
			debug_error("insert phrase:%s\n%s", sqlite3_errmsg(db), sql);
		}
		node = node->next;
	}
}

int fetion_user_list_remove(Config *config, const char *no)
{
	char path[256];
	char sql[4096];
	sqlite3 *db;

	snprintf(path, sizeof(path), "%s/data.db", config->globalPath);

	if(sqlite3_open(path, &db)){
		debug_error("open data.db:%s", sqlite3_errmsg(db));
		return -1;
	}

	snprintf(sql, sizeof(sql), "delete from userlist where no='%s';", no);
	if(sqlite3_exec(db, sql, NULL, NULL, NULL)){
		debug_info("remove user list:%s", sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}
	
	sqlite3_close(db);
	return 1;
}

void escape_sql(char *in)
{
	while(*in){if(*in == '\'') *in = (char)255; in ++;}
}
void unescape_sql(char *inn)
{
	char *in = inn;
	while(*in){if(*in == (char)255) *in = '\''; in ++;}
}


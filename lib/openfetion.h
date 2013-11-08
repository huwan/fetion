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

#ifndef OPENFETION_H
#define OPENFETION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#define _XOPEN_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <libxml/parser.h>
#include <pthread.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <sqlite3.h>
#include <fetion_types.h>
#include <fetion_list.h>
#include <fetion_debug.h>
#include <fetion_message.h>
#include <fetion_connection.h>
#include <fetion_sip.h>
#include <fetion_user.h>
#include <fetion_contact.h>
#include <fetion_login.h>
#include <fetion_config.h>
#include <fetion_conversation.h>
#include <fetion_buddylist.h>
#include <fetion_history.h>
#include <fetion_share.h>
#include <fetion_directsms.h>
#include <fetion_group.h>

#ifdef __cplusplus
}
#endif

#endif

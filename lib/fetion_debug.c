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

struct tm* get_currenttime()
{
	time_t t;
	time(&t);
	return localtime(&t);
}
void debug_info(const char* format , ...)
{
	char t_str[32] = { 0 };
	char fmt[4096] = { 0 };
	va_list ap;
	struct tm* t = get_currenttime();
	strftime(t_str , sizeof(t_str) , "%T" , t );
	sprintf(fmt , "[\e[32m\e[1m%s\e[0m]  %s\n" , t_str , format);
	va_start(ap, format);
	vfprintf(stdout , fmt , ap);
	va_end(ap);
}
void debug_error(const char* format , ...)
{
	char fmt[4096] = { 0 };
	va_list ap;
	sprintf(fmt , "[\e[31m\e[1mFAIL\e[0m] %s\n" , format);
	va_start(ap, format);
	vfprintf(stderr , fmt , ap);
	va_end(ap);
}


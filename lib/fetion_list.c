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

FxList* fx_list_new(void* data)
{
	FxList* fxlist = (FxList*)malloc(sizeof(FxList));
	memset(fxlist , 0 , sizeof(FxList));
	fxlist->pre = fxlist;
	fxlist->data = data;
	fxlist->next = fxlist;
	return fxlist;
}

void fx_list_free(FxList *fxlist)
{
	if(fxlist != NULL)
		free(fxlist);
}

void fx_list_append(FxList *fxlist , FxList *fxitem)
{
	fxlist->next->pre = fxitem;
	fxitem->next = fxlist->next;
	fxitem->pre = fxlist;
	fxlist->next = fxitem;
}

void fx_list_prepend(FxList *fxlist , FxList *fxitem)
{
	fxlist->pre->next = fxitem;
	fxitem->pre = fxlist->pre;
	fxitem->next = fxlist;
	fxlist->pre = fxitem;
}

void fx_list_remove(FxList *fxitem)
{
	fxitem->next->pre = fxitem->pre;
	fxitem->pre->next = fxitem->next;
}

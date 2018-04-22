
/*
 * this file is part of pioe
 * 
 * Copyright (C) 2016-2017 Konrad Lother <k@hiddenbox.org>                            
 *                                                                               
 * This program is free software; you can redistribute it and/or                
 * modify it under the terms of the GNU General Public License                  
 * version 2, as published by the Free Software Foundation.                     
 *                                                                              
 * This program is distributed in the hope that it will be useful, but          
 * WITHOUT ANY WARRANTY; without even the implied warranty of                   
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU             
 * General Public License version 2 for more details.                           
 *                                                                              
 * You should have received a copy of the GNU General Public License            
 * version 2 along with this program; if not, write to the Free                 
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,          
 * MA 02110-1301, USA.     
 *
 */

#include "pioe/error.h"
#include "pioe/logger.h"

#ifdef _WIN32
#include <windows.h>
#endif

/* holds the error message */
static struct _pe_error *last_error = NULL;

/* strict error handling. */
static bool error_strict = true;

PE_EXPORT int pe_error(int code, unsigned char _abort, const char *file,
		       const char *func, unsigned int line, char *format, ...)
{
	struct _pe_error *e;
	va_list list;
	char buf[ERROR_MAX_LEN];

	va_start(list, format);
	vsprintf(buf, format, list);
	va_end(list);

	e = malloc(sizeof(struct _pe_error));
	e->code = code;
	e->file = strdup(file);
	e->func = strdup(func);
	e->line = line;
	e->message = strdup(buf);

	last_error = e;

	if (1 == _abort) {
		pe_error_dump(e);
		exit(e->code);
	}
	return e->code;
}

PE_EXPORT char *pe_error_format(pe_error_t e)
{
	int bufsize = 1024;
	char buf[bufsize];
	sprintf(buf, "%s:%s:%i: %s: %s (%i)", e.file, e.func, e.line, e.message,
		pe_error_str(e.code));
	return strdup(buf);
}

PE_EXPORT void pe_error_dump(pe_error_t * e)
{
	fprintf(stderr, pe_error_format(*e));
	fflush(stderr);
}

PE_EXPORT char *pe_error_str(int code)
{
#ifdef _WIN32
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		      FORMAT_MESSAGE_FROM_SYSTEM |
		      FORMAT_MESSAGE_IGNORE_INSERTS,
		      NULL,
		      code,
		      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		      (LPTSTR) & lpMsgBuf, 0, NULL);
	return lpMsgBuf;
#else
	return strerror(code);
#endif
}

PE_EXPORT bool pe_error_exist()
{
	return ((NULL != last_error) ? true : false);
}

PE_EXPORT pe_error_t *pe_error_last()
{
	return (pe_error_exist()? last_error : NULL);
}

PE_EXPORT int pe_error_release(pe_error_t * e)
{
	struct _pe_error *_e;

	if (NULL == e && pe_error_exist())
		_e = (struct _pe_error *)last_error;
	else if (NULL != e)
		_e = (struct _pe_error *)e;
	else
		return -1;

	free(_e->file);
	free(_e->func);
	free(_e->message);
	free(_e);
	return 0;
}

PE_EXPORT int pe_errno()
{
#ifdef _WIN32
	return GetLastError();
#else
	return errno;
#endif
}

PE_EXPORT void pe_error_strict(bool b)
{
	error_strict = b;
}

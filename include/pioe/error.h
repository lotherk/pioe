
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
 
 
 
 


/**
 * @brief	pioe error handling
 *
 * More detailed description for pioe's error handling.
 *
 * @date	11/11/2016
 * @file	error.h
 * @author	Konrad Lother
 */

#ifndef PIOENGINE_ERROR_H
#define PIOENGINE_ERROR_H

#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include "pioe/export.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
__declspec(dllimport) __cdecl extern int errno;
#else
extern int errno;
#endif

#define ERROR_MAX_LEN 1024

/**
 * @def PE_ERROR(status, ...)
 * Sets a new core error
 */
#define PE_ERROR(code, ...) \
	pe_error(code, 0, __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)

/**
 * @def PE_ABORT(status, ...)
 * Creates an pe_error_t, dumps it using pe_error_dump() and aborts the program.
 */
#define PE_ABORT(status, ...) \
	pe_error(status, 1, __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)

struct _pe_error {
	int code;
	char *file;
	char *func;
	unsigned int line;
	char *errorstr;
	char *message;
};

typedef const struct _pe_error pe_error_t;

/**
 * @brief Create a new error
 *
 * Allocates memory for a new pe_error_t struct. Should not be called directly.
 *
 * @param code an integer representing the error code
 * @param file the file the error occured in
 * @param func the function the error occured in
 * @param line the line number of the file
 * @param format the error message
 * @param ... don't know what to write here
 * @return the allocated error struct.
 * @see pe_error_release(pe_error_t *e)
 * @see PE_ERROR(status, ...)
 */
PE_EXPORT int pe_error(int code, unsigned char _abort, const char *file,
				   const char *func, unsigned int line,
				   char *format, ...);

PE_EXPORT int pe_errno();

PE_EXPORT char* pe_error_str(int code);
/**
 * @brief Set strict error handling
 *
 * When strict error handling is set you must release a previous set error
 * before you can set a new one. If you try to set an error without releasing
 * the previous one, the program aborts.
 *
 * @param b may be true or false
 * @see pe_error_release(pe_error_t *e)
 */
PE_EXPORT void pe_error_strict(bool b);

PE_EXPORT pe_error_t *pe_error_last();

/**
 * @brief Check if a "core" error has been set.
 *
 * @return true if an error is set, false if not
 */
PE_EXPORT bool pe_error_exist();

/**
 * @brief Releases an pe_error_t (free() etc.)
 *
 * If the provided argument is NULL the "core" error will be released.
 *
 * @param e the pe_error_t* to release or NULL to release the "core" error.
 */

PE_EXPORT int pe_error_release(pe_error_t * e);

/**
 * @brief Dumps an pe_error_t to the console
 *
 * @param e the pe_error_t* to dump
 */
PE_EXPORT void pe_error_dump(pe_error_t * e);

PE_EXPORT char *pe_error_format(pe_error_t e);

#ifdef __cplusplus
}
#endif
#endif
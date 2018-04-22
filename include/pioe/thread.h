
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
 
 
 
 


#ifndef PIOENGINE_THREAD_H
#define PIOENGINE_THREAD_H

#include "pioe/error.h"
#include "pioe/logger.h"
#include "pioe/export.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <pthread.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
typedef HANDLE pe_mutex_t;
typedef HANDLE pe_thread_t;
#else
typedef pthread_mutex_t pe_mutex_t;
typedef pthread_t pe_thread_t;
#endif

PE_EXPORT int pe_mutex_lock(pe_mutex_t * m);
PE_EXPORT int pe_mutex_unlock(pe_mutex_t * m);
PE_EXPORT int pe_mutex_trylock(pe_mutex_t * m);
PE_EXPORT int pe_mutex_init(pe_mutex_t * m);

PE_EXPORT int pe_thread_create(pe_thread_t * t, void *func(void *), void *data);
PE_EXPORT int pe_thread_join(pe_thread_t t);
PE_EXPORT int pe_thread_cancel(pe_thread_t t);

#ifdef __cplusplus
}
#endif

#endif
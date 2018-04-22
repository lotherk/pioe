
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
 * @brief	Add brief description!
 *
 * More detailed description
 *
 * @date	01/10/2017
 * @file	util.h
 * @author	Konrad Lother
 */

#ifndef PIOENGINE_UTIL_H
#define PIOENGINE_UTIL_H

#include <stdlib.h>
#include <stdint.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "pioe/logger.h"
#include "pioe/export.h"
#include "pioe/error.h"

#ifdef __cplusplus
extern "C" {
#endif

#define _EXPAND(prefix, key) _pe_exp_##prefix##_##key

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))


// DLL/SO begin
PE_EXPORT void *pe_dll_open(const char *path);
PE_EXPORT void *pe_dll_sym(void *handle, const char *symbol);
// DLL/SO end


// LIST begin
#define PE_LIST_STEP 2048   /*  values < 2048 take ~15msec on my system
                                to init the list. >= 2048 takes 0msec... */

typedef struct _pe_list {
	void *values;
	size_t used;
	size_t size;
    size_t step;
} pe_list_t;

typedef struct _pe_llist pe_llist_t;

typedef struct _pe_llist {
	void *value;
	pe_llist_t *next;
	pe_llist_t *prev;
} pe_llist_t;

PE_EXPORT pe_llist_t *pe_llist();
PE_EXPORT void *pe_llist_pop(pe_llist_t *l);
PE_EXPORT void *pe_llist_shift(pe_llist_t **l);
PE_EXPORT int pe_llist_push(pe_llist_t **l, void * val);
PE_EXPORT int pe_llist_append(pe_llist_t *l, void * val);
PE_EXPORT int pe_llist_count(pe_llist_t *l);

#define LL_EXPAND(key) _EXPAND(llist, key)

#define LL_(key) \
	static pe_llist_t *LL_EXPAND(key) = NULL; \
	static pe_llist_t *LL_EXPAND(key##_current) = NULL;

#define LL_INIT(key) LL_EXPAND(key) = pe_llist();
#define LL_PUSH(key, value) pe_llist_push(&LL_EXPAND(key), (void*) value);
#define LL_SHIFT(key, type) (type) pe_llist_shift(&LL_EXPAND(key));
#define LL_POP(key, type) (type) pe_llist_pop(LL_EXPAND(key));
#define LL_EACH(key, type, elem) \
	LL_EXPAND(key##_current) = LL_EXPAND(key); \
	while(LL_EXPAND(key##_current)->next != NULL) { \
		type elem = (type) LL_EXPAND(key##_current)->value; \
		LL_EXPAND(key##_current) = LL_EXPAND(key##_current)->next;

#define LL_END() }
#define LL_COUNT(key) pe_llist_count(LL_EXPAND(key))

#define pe_list_init(lst, type, len) \
	lst = malloc(sizeof(pe_list_t)); \
	lst->values = (type *) malloc(len * sizeof(type)); \
	lst->used = 0; \
    lst->step = len >= 1 ? len : PE_LIST_STEP; \
	lst->size = len;

#define pe_list_add(lst, type, elem) \
	if (lst->used == lst->size) { \
		lst->size += lst->step; \
		lst->values = (type *) realloc(lst->values, lst->size * sizeof(type)); \
	} \
	((type *)lst->values)[lst->used++] = elem;

#define pe_list_get(lst, type, index) \
	((type *)lst->values)[index]

#define pe_list_each(lst, type, elem, counter) \
	for(counter = 0; counter < lst->used; counter++) { \
		type elem = pe_list_get(lst, type, counter); \

#define pe_end }

#define pe_list_count(lst) lst->used

#define pe_list_free(lst) \
	free(lst->values); \
	free(lst);

// LIST end

PE_EXPORT void pe_sleep(int ms);
PE_EXPORT uint64_t pe_tstamp_msec();
PE_EXPORT uint64_t pe_tstamp_usec();

typedef struct _perfmon {
	char *name;
	uint64_t start_time;
	uint64_t end_time;
	uint64_t duration;
} pe_perfmon_t;

#define PERFMON_EXPAND(key) _EXPAND(perfmon, key)
#define PERFMON_START(key, name) pe_perfmon_t *PERFMON_EXPAND(key) = pe_perfmon_start(name)
#define PERFMON_END(key)  pe_perfmon_end(PERFMON_EXPAND(key))
#define PERFMON_RESULT(key)  pe_perfmon_result(PERFMON_EXPAND(key))

PE_EXPORT pe_perfmon_t *pe_perfmon_start(char *name);
PE_EXPORT void pe_perfmon_end(pe_perfmon_t *p);
PE_EXPORT void pe_perfmon_result(pe_perfmon_t *p);


PE_EXPORT size_t pe_find_file(const char *path[], size_t plen,
		const char *pattern, char **results, int flag);

#ifdef __cplusplus
}
#endif

#endif

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

#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <inttypes.h>
#include <dirent.h>

#include "pioe/logger.h"
#include "pioe/export.h"
#include "pioe/error.h"
#include "pioe/util.h"

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
static const unsigned __int64 epoch = ((unsigned __int64)116444736000000000ULL);

#else
#include <fnmatch.h>
#endif

// DLL/SO begin
PE_EXPORT void *pe_dll_open(const char *path)
{
	void *handle;
#ifdef _WIN32
	handle = LoadLibrary(path);
	if (NULL == handle) {
		PE_ERROR(-1, "could not load dynamic library %s: %s",
			 path, pe_error_str(pe_errno()));
#else
	handle = dlopen(path, RTLD_LAZY);
	if (NULL == handle) {
		PE_ERROR(-1, "could not load dynamic library %s: %s",
			 path, dlerror());
#endif
		return NULL;
	}

	return handle;
}

PE_EXPORT void *pe_dll_sym(void *handle, const char *symbol)
{
	void *ref;

#ifdef _WIN32
	ref = GetProcAddress((HINSTANCE) handle, symbol);
#else
	ref = dlsym(handle, symbol);
#endif

	if (NULL == ref) {
		PE_ERROR(pe_errno(), "could not load symbol %s: %s",
			 symbol, pe_error_str(pe_errno()));
		return NULL;
	}

	return ref;
}

// DLL/SO end

PE_EXPORT void pe_sleep(int ms)
{
#ifdef _WIN32
	timeBeginPeriod(1);
	Sleep(ms);
	timeEndPeriod(1);
#else
	struct timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;
	nanosleep(&ts, NULL);
#endif
}

PE_EXPORT uint64_t pe_tstamp_msec()
{
	return pe_tstamp_usec() / 1000;
}

PE_EXPORT uint64_t pe_tstamp_usec()
{
	uint64_t res = 0.0;
	uint64_t sec = 0ULL;
	long int usec = 0;

	struct timeval tv;
	gettimeofday(&tv, NULL);
	sec = tv.tv_sec;
	usec = tv.tv_usec;

	sec *= 1000000.0;

	res = sec + usec;

	return res;
}

PE_EXPORT pe_perfmon_t *pe_perfmon_start(char *name)
{
	pe_perfmon_t *ret = malloc(sizeof(pe_perfmon_t));
	ret->start_time = pe_tstamp_usec();
	ret->name = strdup(name);
	return ret;
}

PE_EXPORT void pe_perfmon_end(pe_perfmon_t * p)
{
	p->end_time = pe_tstamp_usec();
	p->duration = p->end_time - p->start_time;
}

PE_EXPORT void pe_perfmon_result(pe_perfmon_t * p)
{
	fprintf(stderr,
		"\nPERFMON RESULT: [%s] took: %.06f seconds\n\n", p->name,
		(float)p->duration / 1000.0 / 1000.0);
	fflush(stderr);
}

PE_EXPORT pe_llist_t *pe_llist()
{
	pe_llist_t *l = malloc(sizeof(pe_llist_t));

	if (l == NULL) {
		PE_ABORT(-1, "could not create list, malloc failed.");
	}

	l->next = NULL;
	l->prev = NULL;
	l->value = NULL;

	return l;
}

PE_EXPORT void *pe_llist_pop(pe_llist_t * l)
{
	if (l == NULL) {
		PE_ERROR(-1, "list must not be NULL");
		return NULL;
	}

	void *retval = NULL;
	pe_llist_t *current = NULL;

	if (l->next == NULL) {
		retval = l->value;
		free(l);
		l = NULL;
		return retval;
	}

	current = l;
	while (current->next->next != NULL)
		current = current->next;

	retval = current->next->value;
	free(current->next);
	current->next = NULL;

	return retval;
}

PE_EXPORT void *pe_llist_shift(pe_llist_t ** l)
{
	void *retval = NULL;
	pe_llist_t *next_node = NULL;

	if (*l == NULL)
		return NULL;

	next_node = (*l)->next;
	retval = (*l)->value;
	free(*l);
	*l = next_node;
	return retval;
}

PE_EXPORT int pe_llist_push(pe_llist_t ** l, void *val)
{
	pe_llist_t *new_node = pe_llist();
	new_node->value = val;
	new_node->next = *l;
	*l = new_node;
	return 0;
}

PE_EXPORT int pe_llist_append(pe_llist_t * l, void *val)
{
	pe_llist_t *current = l;
	while (current->next != NULL)
		current = current->next;

	pe_llist_t *new_list = pe_llist();
	new_list->value = val;
	current->next = new_list;
	return 0;
}

PE_EXPORT int pe_llist_count(pe_llist_t * l)
{
	int counter = 0;
	pe_llist_t *current = l;
	while (current->next != NULL) {
		current = current->next;
		counter++;
	}
	return counter;
}

PE_EXPORT size_t pe_find_file(const char *path[], size_t plen,
			      const char *pattern, char **results, int flag)
{
	size_t len = 0;
	struct dirent *dirent;
	DIR *dir;
	int i;

	if (!results)
		PE_ABORT(-1, "Cannot calloc");

	for (i = 0; i < plen; i++) {
		if (strlen(path[i]) == 0)
			continue;

		dir = opendir(path[i]);

		if (NULL == dir)
			continue;

		while ((dirent = readdir(dir)) != NULL) {

			if (strcmp(dirent->d_name, ".") == 0
			    || strcmp(dirent->d_name, "..") == 0)
				continue;

#if defined(_WIN32)
			PE_ABORT(-1, "NOT IMPLEMENTED");
#else
			if (fnmatch(pattern, dirent->d_name, 0) == 0) {
#endif
				char tmp[strlen(path[i]) +
					 strlen(dirent->d_name) + 1];
				sprintf(tmp, "%s%s", path[i], dirent->d_name);
				results[len] = strdup(tmp);
				len++;
			}

			if (flag == 1 && len == 1)
				break;
		}
	}
	return len;
}

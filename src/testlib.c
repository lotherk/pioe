
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

#include "pioe/testlib.h"
#include "pioe/export.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>

static pe_testlib_t *tests[1024];
static int tests_count = 0;
static uint64_t usec();

static uint64_t usec()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000000.0) + tv.tv_usec;
}

PE_EXPORT int pe_testlib_test(char *name, int (*testfunc) (pe_testlib_t * t))
{
	pe_testlib_t *t = malloc(sizeof(pe_testlib_t));
	t->name = strdup(name);
	t->func = testfunc;
	t->stages_i = 0;
	t->current = NULL;
	tests[tests_count++] = t;
	return 0;
}

PE_EXPORT int pe_testlib_stage(pe_testlib_t * t, char *name)
{
	pe_testlib_stage_t *s = malloc(sizeof(pe_testlib_stage_t));
	s->name = strdup(name);
	s->code = 0;
	s->stime = usec();
	s->test = t;
	s->message = NULL;
	t->stages[t->stages_i++] = s;
	if (t->current != NULL)
		pe_testlib_stage_end(t->current);
	t->current = s;
	fprintf(stdout, "   - %s: ", s->name);
	fflush(stdout);
	return 0;
}

PE_EXPORT int pe_testlib_runtests(char *keys[], size_t size)
{
	int i;
	for (i = 0; i < size; i++) {
		pe_testlib_runtest(keys[i]);
	}
	return 0;
}

PE_EXPORT int pe_testlib_runtest(char *key)
{
	int i;
	pe_testlib_t *t = NULL;
	for (i = 0; i < tests_count; i++) {
		if (strcmp(tests[i]->name, key) == 0) {
			t = tests[i];
			break;
		}
	}

	if (NULL == t) {
		fprintf(stderr, "Test %s not found\n", key);
		fflush(stderr);
		exit(1);
	}

	fprintf(stdout, "-- Running test %s\n", t->name);
	fflush(stdout);
	uint64_t stime = usec();
	int result = t->func(t);
	uint64_t etime = usec();
	if (t->current != NULL)
		pe_testlib_stage_end(t->current);

	uint64_t dtime = etime - stime;
	fprintf(stdout, "%s: (%i) - took: %.06f sec\n\n",
		(result == 0 ? "SUCCESS" : "FAILED"),
		t->code, ((float)(dtime / 1000.0 / 1000.0)));
	fflush(stdout);
	if (result != 0)
		exit(result);
	return 0;
}

PE_EXPORT int pe_testlib_stage_end(pe_testlib_stage_t * s)
{
	s->etime = usec();
	s->dtime = s->etime - s->stime;
	fprintf(stdout, "%s - took: %.06f seconds, code: %i\n",
		s->code == 0 ? "OK" : "FAILED",
		(float)(s->dtime / 1000.0 / 1000.0), s->code);
	fflush(stdout);
	s->test->current = NULL;
	return 0;
}

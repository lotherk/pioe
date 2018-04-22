
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
 * @date	01/15/2017
 * @file	testlib.h
 * @author	Konrad Lother
 */

#ifndef PIOENGINE_TESTLIB_H
#define PIOENGINE_TESTLIB_H

#include <stdarg.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>

#include "pioe/export.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _testlib_t pe_testlib_t;
typedef struct _testlib_stage_t pe_testlib_stage_t;
typedef struct _testlib_stage_t {
	char *name;
	int code;
	char *message;
	uint64_t dtime;
	uint64_t stime;
	uint64_t etime;
	pe_testlib_t *test;
} pe_testlib_stage_t;

typedef struct _testlib_t {
	char *name;
	int (*func)(pe_testlib_t *self);
	pe_testlib_stage_t *stages[1024];
	size_t stages_i;
	int code;
	pe_testlib_stage_t *current;
} pe_testlib_t;

#define TEST_STAGE(t, title) \
	pe_testlib_stage(t, title)

#define TEST_FAIL(t, c) \
	return c;

#define STAGE_FAIL(t, c) \
	t->current->code = c; \
	pe_testlib_stage_end(t->current);

#define FAIL_IF(t, condition) \
	if (condition) { \
		STAGE_FAIL(t, -1); \
		TEST_FAIL(t, -1); \
	}
PE_EXPORT int pe_testlib_test(char *name, int (*testfunc)(pe_testlib_t *self));
PE_EXPORT int pe_testlib_stage(pe_testlib_t *t, char *title);
PE_EXPORT int pe_testlib_stage_end(pe_testlib_stage_t *t);
PE_EXPORT int pe_testlib_runtest(char *key);
PE_EXPORT int pe_testlib_runtests(char *keys[], size_t size);

#ifdef __cplusplus
}
#endif


#endif
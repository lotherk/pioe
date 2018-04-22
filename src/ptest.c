
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

#include "pioe/logger.h"
#include "pioe/util.h"
#include "pioe/testlib.h"
#include "pioe/thread.h"
#include "pioe/error.h"
#include "pioe/engine.h"

static int list_size = 1024;

static int test_pe_error(pe_testlib_t * t)
{
	TEST_STAGE(t, "pe_error_exist() returns 0");
	FAIL_IF(t, pe_error_exist() != 0);

	char *err_message = "test error";
	int err_line = __LINE__;

	TEST_STAGE(t, "pe_error_last() returns NULL");
	FAIL_IF(t, pe_error_last() != NULL);

	pe_error(-1, 0, __FILE__, __func__, err_line, err_message);

	pe_error_t *err = pe_error_last();
	FAIL_IF(t, err == NULL);

	TEST_STAGE(t, "err code is -1");
	FAIL_IF(t, err->code != -1);

	TEST_STAGE(t, "err file is __FILE__");
	FAIL_IF(t, strcmp(err->file, __FILE__) != 0);

	TEST_STAGE(t, "err line is err_line");
	FAIL_IF(t, err->line != err_line);

	TEST_STAGE(t, "err message is err_message");
	FAIL_IF(t, strcmp(err->message, err_message) != 0);

	TEST_STAGE(t, "pe_error_last() returns not NULL");
	FAIL_IF(t, pe_error_last() == NULL);

	TEST_STAGE(t, "pe_error_exist() returns 1");
	FAIL_IF(t, pe_error_exist() != 1);

	TEST_STAGE(t, "pe_error_last()->message is err->message");
	FAIL_IF(t, strcmp(pe_error_last()->message, err->message) != 0);

	return 0;
}

static int test_core_logger(pe_testlib_t * t)
{

	TEST_STAGE(t, ".out is stdout");
	if (pe_logger_core()->out != stdout) {
		STAGE_FAIL(t, -1);
		TEST_FAIL(t, -1);
	}
	TEST_STAGE(t, ".err is stderr");
	if (pe_logger_core()->err != stderr) {
		STAGE_FAIL(t, -1);
		TEST_FAIL(t, -1);
	}
	TEST_STAGE(t, ".level is LALL");
	FAIL_IF(t, pe_logger_core()->level != LALL);

	TEST_STAGE(t, ".format is LOGGER_FORMAT_DEFAULT");
	FAIL_IF(t,
		strcmp(pe_logger_core()->format, LOGGER_FORMAT_DEFAULT) != 0);
	return 0;
}

static int test_llist(pe_testlib_t * t)
{
	TEST_STAGE(t, "define linked list");
	LL_(test_llist);
	if (LL_EXPAND(test_llist) != NULL) {
		STAGE_FAIL(t, -1);
		TEST_FAIL(t, -1);
	}

	TEST_STAGE(t, "initialize linked list");
	LL_INIT(test_llist);
	if (LL_EXPAND(test_llist) == NULL) {
		STAGE_FAIL(t, -1);
		TEST_FAIL(t, -1);
	}

	TEST_STAGE(t, "push elements to list");
	int i = 0;
	for (i = 0; i < list_size; i++) {
		LL_PUSH(test_llist, &i);
		if (LL_EXPAND(test_llist)->value != &i) {
			STAGE_FAIL(t, -1);
			TEST_FAIL(t, -1);
		}
	}

	TEST_STAGE(t, "list size");
	if (LL_COUNT(test_llist) != list_size) {
		STAGE_FAIL(t, -1);
		TEST_FAIL(t, -1);
	}

	TEST_STAGE(t, "each elements");
	i = 0;
	LL_EACH(test_llist, int *, item)
	if (item != &i) {
		STAGE_FAIL(t, -1);
		TEST_FAIL(t, -1);
	}
	i++;
	LL_END();

	return 0;
}

#ifdef _WIN32
#define SLEEP_OFFSET_MAX 30	/* we can't get pioe to sleep for <= 1ms yet.. */
#define SLEEP_OFFSET_MIN 0
#else
#define SLEEP_OFFSET_MAX 1
#define SLEEP_OFFSET_MIN 0
#endif

static int test_pe_sleep(pe_testlib_t * t)
{
	uint64_t stime, etime, dtime = 0;
#define _TEST_SLEEP(title, ms) \
	TEST_STAGE(t, title); \
	stime = pe_tstamp_msec(); \
	pe_sleep(ms); \
	etime = pe_tstamp_msec(); \
	dtime = etime - stime; \
	FAIL_IF(t, dtime > (ms + SLEEP_OFFSET_MAX) || dtime < (ms + SLEEP_OFFSET_MIN));

#ifdef _WIN32
	// initialize timer for the first time
	pe_sleep(128);
#endif

	_TEST_SLEEP("1ms", 1);
	_TEST_SLEEP("50ms", 50);
	_TEST_SLEEP("500ms", 500);
	_TEST_SLEEP("2000ms", 2000);

	return 0;
}

volatile int pe_thread_val = 0;
static pe_mutex_t mutex;

static void *threadfunc(void *args)
{
	int *i = (int *)args;
	while (*i < 100000) {
		if (pe_mutex_trylock(&mutex)) {
			*i += 1;
			pe_mutex_unlock(&mutex);
		}
	}

	return NULL;
};

static int test_pe_thread(pe_testlib_t * t)
{
	TEST_STAGE(t, "pe_mutex_init returns 0");
	FAIL_IF(t, pe_mutex_init(&mutex) != 0);

	TEST_STAGE(t, "pe_mutex_lock returns 0");
	FAIL_IF(t, pe_mutex_lock(&mutex) != 0);

	TEST_STAGE(t, "pe_mutex_trylock returns 1");
	FAIL_IF(t, pe_mutex_trylock(&mutex) == 1);

	TEST_STAGE(t, "pe_mutex_unlock returns 0");
	FAIL_IF(t, pe_mutex_unlock(&mutex) != 0);

	TEST_STAGE(t, "pe_thread_val");
	pe_thread_t t1;
	pe_thread_t t2;
	int c1 = 0;
	int c2 = 0;
	pe_thread_create(&t1, threadfunc, &c1);
	pe_thread_create(&t2, threadfunc, &c2);
	TEST_STAGE(t, "wait for t1 to count to 100000");
	pe_thread_join(t1);
	TEST_STAGE(t, "wait for t2 to count to 100000");
	pe_thread_join(t2);
	TEST_STAGE(t, "check both have counted to 100000");
	FAIL_IF(t, (c1 != 100000 || c2 != 100000));

	return 0;
}

static int test_pe_engine(pe_testlib_t * t)
{
	TEST_STAGE(t, "load ruby engine");
	FAIL_IF(t, pe_engine_load_by_name("ruby") != 0);
	TEST_STAGE(t, "load python engine");
	FAIL_IF(t, pe_engine_load_by_name("python") != 0);
}

int test_lukrop_joined_project(pe_testlib_t * t)
{
	TEST_STAGE(t, "persuade lukrop to join the project");
	FAIL_IF(t, -1337);

	return 0;
}

int main(int argc, char *argv[])
{
	PERFMON_START(main, "main");

	pe_logger_init(stdout, stderr);
	pe_logger_set_level(LALL);

	pe_testlib_test("logger", &test_core_logger);
	pe_testlib_test("error", &test_pe_error);
	pe_testlib_test("linked_list", &test_llist);
	pe_testlib_test("pe_sleep", &test_pe_sleep);
	pe_testlib_test("pe_thread", &test_pe_thread);
	pe_testlib_test("pe_engine", &test_pe_engine);
	pe_testlib_test("lukrop", &test_lukrop_joined_project);

	int i;
	for (i = 1; i < argc; i++) {
		pe_testlib_runtest(argv[i]);
	}

	PERFMON_END(main);
	PERFMON_RESULT(main);
	return 0;
}

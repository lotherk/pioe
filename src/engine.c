
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

#include "pioe/engine.h"
#include "pioe/logger.h"
#include "pioe/export.h"
#include "pioe/error.h"
#include "pioe/util.h"
#include "pioe/thread.h"
#include "config.h"

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>

static void sigint_handler(int sig);

static pe_thread_t threads[MAX_ENGINES];
static size_t threads_i = 0;

static pe_list_t *engine_handles = NULL;
static pe_engine_state_t current_state = STATE_STOP;
static pe_frame_t frame;

#define CHECK_ENGINE_AVAIL \
	if(pe_list_count(engine_handles) == 0) \
		PE_ABORT(-1,"NO ENGINE AVAILABLE");

#if defined(_WIN32)
#define LIB_PREFIX "lib"
#define LIB_SUFFIX "dll"
static const char *_engine_search_path[] = { "" };
#elif defined(_CYGWIN)
#define LIB_PREFIX "cyg"
#define LIB_SUFFIX "dll"
static const char *_engine_search_path[] = { "" };
#else
#define LIB_PREFIX "lib"
#define LIB_SUFFIX "so"
static const char *_engine_search_path[] =
    { "./", "./engines/", "./lib/", "../lib/", "../lib/pioe/", "" };
#endif

static char *_compute_lib_path(const char *key)
{
	char res[1024];
	sprintf(res, LIB_PREFIX "pioe%sengine." LIB_SUFFIX, key);
	return strdup(res);

}

static void sigint_handler(int sig)
{
	LOG_DEBUG("Signal %i received", sig);
	pe_engine_quit();
}

PE_EXPORT size_t pe_engine_find_engines(char **results)
{
	return pe_find_file(_engine_search_path,
			    ARRAY_SIZE(_engine_search_path),
			    _compute_lib_path("*"), results, 0);

}

PE_EXPORT int pe_engine_load_by_name(const char *name)
{

	if (name[0] == '.' || name[0] == '/') {
		if (access(name, F_OK) != 0) {
			PE_ABORT(-1, "No such file or directory: %s", name);
		}
		return pe_engine_load(name);
	}

	char *libname = _compute_lib_path(name);

	int i;
	char path[PATH_MAX];
	for (i = 0;
	     i < (sizeof(_engine_search_path) / sizeof(_engine_search_path[0]));
	     i++) {
		sprintf(path, "%s%s", _engine_search_path[i], libname);
		if (pe_engine_load(path) == 0)
			return 0;
	}
	return -1;
}

PE_EXPORT int pe_engine_load(const char *path)
{
#define SYM(p, h, n) \
	p->h = pe_dll_sym(p->handle, n); \
	if (p->h == NULL) { \
		PE_ABORT(pe_errno(), "Could not load symbol %s", n); \
	}

	if (engine_handles == NULL) {
		// init engine_handles list
		pe_list_init(engine_handles, pe_engine_handle_t *, 0);
	}

	void *handle = pe_dll_open(path);
	if (handle == NULL) {
		return PE_ERROR(pe_error_last()->code, (char *)path);
	}

	LOG_DEBUG("Loading engine from %s", path);
	pe_engine_handle_t *eh = malloc(sizeof(pe_engine_handle_t));
	eh->engine = malloc(sizeof(pe_engine_t));

	if (NULL == eh->engine)
		PE_ABORT(-1, "out of memory");

	eh->handle = handle;

	SYM(eh, load, "engine_load");
	eh->load(eh->engine);

	SYM(eh, unload, "engine_unload");
	SYM(eh, init, "engine_init");
	SYM(eh, frame, "engine_frame");
	SYM(eh, start, "engine_start");
	SYM(eh, stop, "engine_stop");
	SYM(eh, load_script, "engine_load_script");
	SYM(eh, execute_code, "engine_execute_code");

	eh->engine->mutex = malloc(sizeof(pe_mutex_t));
	pe_mutex_init(eh->engine->mutex);

	pe_list_add(engine_handles, pe_engine_handle_t *, eh);

	LOG_INFO
	    ("Engine %s (%s) loaded. Script language: %s, Script suffix: %s",
	     eh->engine->name, eh->engine->version, eh->engine->script_language,
	     eh->engine->script_suffix);

	return 0;
}

PE_EXPORT int pe_engine_unload(pe_engine_handle_t * eh)
{
	LOG_DEBUG("Unloading engine %s", eh->engine->name);
	eh->stop();
	eh->unload();

	/*      free(eh->load);
	   free(eh->unload);
	   free(eh->init);
	   free(eh->frame);
	   free(eh->start);
	   free(eh->stop);
	   free(eh->load_script);
	   free(eh->execute_code);
	   free(eh->engine->name);
	   free(eh->engine->version);
	   free(eh->engine->script_language);
	   free(eh->engine->script_suffix);
	   free(eh->engine->mutex);
	   free(eh->engine);
	   free(eh);
	 */
	return 0;
}

PE_EXPORT int pe_engine_init()
{
	CHECK_ENGINE_AVAIL;

	signal(SIGINT, sigint_handler);
	frame.id = 0;
	pe_mutex_init(&(frame.mutex));

	int i;

	pe_list_each(engine_handles, pe_engine_handle_t *, e, i)
	    e->init();
	pe_end;

	return 0;
}

static void *engine_thread_func(void *arg)
{
	pe_engine_handle_t *eh = arg;
	if (eh->frame((eh->_frame))) {
		LOG_ERROR("frame failed");
		return NULL;
	}
	pe_mutex_unlock(eh->engine->mutex);
}

PE_EXPORT int pe_engine_run()
{
	CHECK_ENGINE_AVAIL;
	current_state = STATE_RUNNING;

	for (frame.id = 0; current_state == STATE_RUNNING; frame.id++) {
		int i;
		pe_list_each(engine_handles, pe_engine_handle_t *, eh, i) {

			if (pe_mutex_trylock(eh->engine->mutex)) {
				continue;
			}

			if (frame.id > 0)
				pe_thread_join(eh->thread);

			eh->_frame = frame;
			if (pe_thread_create
			    (&(eh->thread), engine_thread_func, eh)) {
				PE_ABORT(pe_errno(), "could not create thread");
			}

		}
		pe_end;
		pe_sleep(PIOE_FRAME_RESOLUTION_MS);
		frame.id++;
	}

	return 0;
}

PE_EXPORT int pe_engine_load_script(const char *file)
{
	CHECK_ENGINE_AVAIL;

	if (access(file, F_OK) == -1) {
		return PE_ERROR(-1, "No such file or directory");
	}

	const char *suffix = strrchr(file, '.') + 1;
	pe_engine_handle_t *eh = NULL;

	int i;
	pe_list_each(engine_handles, pe_engine_handle_t *, e, i) {
		if (NULL == e)
			continue;

		if (NULL == e->engine)
			PE_ABORT(-1, "e->engine is NULL");

		if (strcmp(suffix, e->engine->script_suffix) == 0) {
			eh = e;
			break;
		}
	}
	pe_end;

	if (NULL == eh)
		return -1;

	// read file contents
	FILE *fp;
	char *fbuf;
	size_t fsize;

	fp = fopen(file, "r");
	if (NULL == fp)
		return PE_ERROR(pe_errno(), (char *)file);

	fseek(fp, 0L, SEEK_END);
	fsize = ftell(fp);
	rewind(fp);

	fbuf = calloc(1, fsize + 1);
	if (!fbuf)
		PE_ABORT(-1, "out of memory");

	if (fread(fbuf, fsize, 1, fp) != 1)
		PE_ABORT(pe_errno(), (char *)file);

	fclose(fp);

	// ADD PREPROCESSOR HERE

	if (eh->load_script(fbuf))
		PE_ABORT(pe_errno(), (char *)file);

	free(fbuf);
	return 0;
}

PE_EXPORT int pe_engine_quit()
{
	int i;
	current_state = STATE_STOP;
	pe_list_each(engine_handles, pe_engine_handle_t *, eh, i) {
		pe_thread_cancel(eh->thread);
		pe_engine_unload(eh);
	}
	pe_end;
	return 0;
}

PE_EXPORT int pe_engine_start()
{
	return 0;
}

PE_EXPORT int pe_engine_stop()
{
	return 0;
}

PE_EXPORT uint64_t pe_engine_frame_id()
{
	return frame.id;
}

PE_EXPORT pe_class_t *pe_engine_define_class(pe_plugin_t * plugin,
					     const char *name,
					     pe_class_t * parent)
{
	CHECK_ENGINE_AVAIL;
	LOG_DEBUG("Defining class for plugin %s", plugin->name);
	pe_class_t *c = malloc(sizeof(pe_class_t));
	c->name = strdup(name);
	pe_list_t *im = c->instance_methods;
	pe_list_init(c->instance_methods, pe_method_t *, 0);
	pe_list_init(c->class_methods, pe_method_t *, 0);
	pe_list_init(c->instances, pe_instance_t *, 0);
	return c;
}

PE_EXPORT pe_method_t *pe_engine_define_class_method(pe_class_t * c,
						     int (*method) (pe_param_t
								    *),
						     size_t num_params, ...)
{
	CHECK_ENGINE_AVAIL;
	return 0;
}

PE_EXPORT pe_method_t *pe_engine_define_instance_method(pe_class_t * c,
							int (*method)
							(pe_param_t *),
							size_t num_params, ...)
{
	CHECK_ENGINE_AVAIL;
	return 0;
}

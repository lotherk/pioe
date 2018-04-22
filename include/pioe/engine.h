
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
 * @date	01/09/2017
 * @file	engine.h
 * @author	Konrad Lother
 */

#ifndef PIOENGINE_ENGINE_H
#define PIOENGINE_ENGINE_H
#include <stdarg.h>
#include "pioe/export.h"
#include "pioe/thread.h"
#include "pioe/plugin.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ENGINES 32

typedef enum {
	STATE_RUNNING,
	STATE_START,
	STATE_STOP,
} pe_engine_state_t;

typedef struct pe_frame pe_frame_t;
typedef struct pe_engine pe_engine_t;

struct pe_frame {
	uint64_t id;
	pe_mutex_t mutex;
};
struct pe_engine {
	unsigned int id;
	char *name;
	char *version;
	char *script_language;
	char *script_suffix;
	pe_mutex_t *mutex;
};

typedef struct pe_engine_handle {
        void *handle;
        pe_engine_state_t state;
        int (*load) (pe_engine_t *);
        int (*unload) ();
        int (*init) ();
        int (*frame) (pe_frame_t frame);
        int (*start) ();
        int (*stop) ();
        int (*load_script) (const char *);
        int (*execute_code) (const char *);
        pe_engine_t *engine;
	pe_thread_t thread;
	pe_frame_t _frame;
} pe_engine_handle_t;

#ifdef _WIN32

#else
#endif


PE_EXPORT int pe_engine_load_by_name(const char *name);
PE_EXPORT int pe_engine_load(const char *path);
PE_EXPORT int pe_engine_unload(pe_engine_handle_t *e);

PE_EXPORT int pe_engine_start();
PE_EXPORT int pe_engine_run();
PE_EXPORT int pe_engine_stop();
PE_EXPORT int pe_engine_load_script(const char *file);
PE_EXPORT size_t pe_engine_find_engines(char **result);
PE_EXPORT int pe_engine_init();
PE_EXPORT int pe_engine_quit();
PE_EXPORT uint64_t pe_engine_frame_id();

PE_EXPORT pe_class_t *pe_engine_define_class(pe_plugin_t *p, const char *name, pe_class_t *parent);
PE_EXPORT pe_method_t *pe_engine_define_class_method(pe_class_t *c, int (*method)(pe_param_t*), size_t num_params, ...);
PE_EXPORT pe_method_t *pe_engine_define_instance_method(pe_class_t *c, int (*method)(pe_param_t*), size_t num_params, ...);

#ifdef __cplusplus
}
#endif

#endif
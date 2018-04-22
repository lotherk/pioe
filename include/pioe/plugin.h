
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
 * @brief	Functions to register a plugin into pioe
 *
 * More detailed description
 *
 * @date	12/07/2016
 * @file	plugin.h
 * @author	Konrad Lother
 */

#ifndef PIOENGINE_PLUGIN_H
#define PIOENGINE_PLUGIN_H

#include <stdbool.h>

#include "pioe/error.h"
#include "pioe/logger.h"
#include "pioe/export.h"
#include "pioe/util.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PLUGIN_REGISTER(name, version) \
	static pe_plugin_t *PLUGIN = pe_plugin_register(name, version)

#define LOAD() PE_EXPORT int plugin_load()
#define UNLOAD() PE_EXPORT int plugin_unload()
#define INIT() PE_EXPORT int plugin_init()

#define PARAM(p, i, ptr) pe_plugin_param(p, i, ptr)
#define RETURN(p, ptr, t) pe_plugin_return(p, ptr, t)

union parameter {
	int i;
	float f;
	char *s;
	void *o;
};

typedef const union parameter pe_parameter_u;

typedef enum {
	INTEGER_T,
	FLOAT_T,
	STRING_T,
	OBJECT_T,
	INTEGER_A,
	FLOAT_A,
	STRING_A,
	OBJECT_A,
	CLASS_T,
	INSTANCE_T
} pe_parameter_t;

#define MAX_PARAMS 64
struct parameters {
	size_t size;
	pe_parameter_u params[MAX_PARAMS];
	pe_parameter_t types[MAX_PARAMS];
	pe_parameter_u rval;
	pe_parameter_t rtype;
};
typedef const struct parameters pe_param_t;

typedef const struct pe_plugin pe_plugin_t;
typedef struct pe_class pe_class_t;
typedef struct pe_method pe_method_t;

struct pe_plugin {
	char *name;
	char *version;
	pe_class_t *classes;
	size_t classes_i;
};

typedef struct pe_instance {

} pe_instance_t;

struct pe_class {
	char *name;
	pe_class_t *parent;
	pe_list_t *instance_methods;
	pe_list_t *class_methods;
	pe_list_t *instances;
};

struct pe_method {
	char *name;
	pe_param_t params;
	int (*method)(pe_parameter_t*);
};

/**
 * @brief Register plugin
 *
 * Bla Bla
 *
 * @param name name of the plugin
 * @param version versionnumber as char*
 * @return pe_plugin_t returns NULL on error
 */
PE_EXPORT pe_plugin_t *pe_plugin_register(char *name, char *version);

/**
 * @brief Read a parameter
 *
 * Bla bla
 *
 * @param p pe_param_t
 * @param i index of parameter to read
 * @param ptr pointer to write the value to
 * @return bool true if success, false if failure
 */
PE_EXPORT bool pe_plugin_param(pe_param_t *p, size_t i, void *ptr);

/**
 * @brief Write a return value
 *
 * Bla bla
 *
 * @param p pe_param_t to write the return value into
 * @param ptr value to write
 * @param t type of value to write
 * @return bool true if success, false if failure
 */
PE_EXPORT bool pe_plugin_return(pe_param_t *p, void *ptr, pe_parameter_t t);

#ifdef __cplusplus
}
#endif

#endif
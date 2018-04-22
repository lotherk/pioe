
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

#include "pioe/plugin.h"

static pe_plugin_t *plugins[1024];
static unsigned int plugins_index = 0;

PE_EXPORT pe_plugin_t *pe_plugin_register(char *name, char *version)
{
	struct pe_plugin *p = malloc(sizeof(struct pe_plugin));
	p->name = strdup(name);
	p->version = strdup(version);
	plugins[plugins_index++] = (pe_plugin_t *) p;

	LOG_DEBUG("Plugin %s (%s) registered.", name, version);
	return (pe_plugin_t *) p;
}

PE_EXPORT bool pe_plugin_param(pe_param_t * p, size_t i, void *ptr)
{
	if (p->size > i) {
		PE_ERROR(-1, "Parameter index %i is out of bounds (%i)",
			 i, p->size);
		return false;
	}

	pe_parameter_t t = p->types[i];

	switch (t) {
	case INTEGER_T:
		*((int *)ptr) = p->params[i].i;
		break;
	case STRING_T:
		strcpy(ptr, p->params[i].s);
		break;
	case FLOAT_T:
		*((float *)ptr) = p->params[i].f;
		break;
	case OBJECT_T:
		ptr = p->params[i].o;
		break;
	default:
		PE_ERROR(-1, "Unknown pe_parameter_t: %i", t);
		return false;
	}

	return true;
}

PE_EXPORT bool pe_plugin_return(pe_param_t * p, void *ptr, pe_parameter_t t)
{
	p = (struct parameters *)p;
	union parameter *up = (union parameter *)&(p->rval);
	switch (t) {
	case INTEGER_T:
		up->i = *((int *)ptr);
		break;
	case STRING_T:
		up->s = strdup(ptr);
		break;
	case FLOAT_T:
		up->f = *((float *)ptr);
		break;
	case OBJECT_T:
		up->o = ptr;
		break;
	default:
		PE_ERROR(-1, "Unknown pe_parameter_t: %i", t);
		return false;
	}
	//p->rtype = t;
	return true;
}

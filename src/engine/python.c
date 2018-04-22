
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

#include "pioe/engine/python.h"
#include "pioe/engine.h"
#include "pioe/logger.h"
#include "pioe/export.h"

#include <Python.h>
#include <stdarg.h>

static pe_engine_t *engine;

static pe_logger_t logger;
#undef MACRO_LOGGER
#define MACRO_LOGGER logger

static int python_code(const char *fmt, ...);
static int handle_exception();

PE_EXPORT int engine_load(pe_engine_t * p)
{
	pe_logger_new(&logger, "python-engine");
	LOG_DEBUG("Loading");
	engine = p;
	engine->name = "Python";
	engine->version = "0.0.1";
	engine->script_language = "Python";
	engine->script_suffix = "py";
	return 0;
}

PE_EXPORT int engine_unload()
{
	LOG_DEBUG("Unloading");
	return 0;
}

PE_EXPORT int engine_init()
{
	LOG_DEBUG("Initializing");
	Py_SetProgramName((wchar_t *) "PIOE");
	Py_Initialize();

	return 0;
}

PE_EXPORT int engine_frame(pe_frame_t frame)
{
	if ((frame.id % 500) == 0) {
		LOG_DEBUG("500st!");
	}
	return 0;
}

PE_EXPORT int engine_start()
{

	LOG_DEBUG("Stating");
	return 0;
}

PE_EXPORT int engine_stop()
{
	LOG_DEBUG("Stopping");

	return 0;
}

PE_EXPORT int engine_quit()
{
	LOG_DEBUG("Quit");
	Py_Finalize();
	return 0;
}

PE_EXPORT int engine_load_script(const char *code)
{
	return 0;
}

PE_EXPORT int engine_execute_code(const char *code)
{
	LOG_DEBUG("Execute code: %s", code);

	return python_code(code);
}

PE_EXPORT int engine_define_class(pe_class_t * c)
{
	LOG_DEBUG("Defining class %s", c->name);

	return 0;
}

static int define_method(int type, pe_class_t * c, pe_method_t * m)
{
	const char *code = "%s.define%s_method(:%s) { |*args| "
	    "PIOE.method_callback(self, :%s, args) " "}";

	char *t;
	if (0 == type)
		t = "_singleton";
	else if (1 == type)
		t = "";
	else
		PE_ABORT(-1, "Undefined type: %i", type);

	int buf_len = 0;
	buf_len += strlen(code);
	buf_len += strlen(t);
	buf_len += strlen(c->name);
	buf_len += strlen(m->name) * 2;
	buf_len -= 8;		// 4 * %s in *code

	char buf[buf_len];
	sprintf(buf, code, c->name, t, m->name, m->name);
	LOG_DEBUG("Code:\n%s", buf);

	return 0;
}

PE_EXPORT int engine_define_class_method(pe_class_t * c, pe_method_t * m)
{
	LOG_DEBUG("Defining class method %s::%s", c->name, m->name);
	define_method(0, c, m);
	return 0;
}

PE_EXPORT int engine_define_instance_method(pe_class_t * c, pe_method_t * m)
{
	LOG_DEBUG("Defining instance method %s#%s", c->name, m->name);
	define_method(1, c, m);
	return 0;
}

static int handle_exception()
{
	/*
	   VALUE exception = rb_errinfo();
	   if (exception == Qnil)
	   return 0;

	   VALUE m = rb_funcall(exception, rb_intern("message"), 0);
	   VALUE c = rb_funcall(exception, rb_intern("class"), 0);
	   c = rb_funcall(c, rb_intern("to_s"), 0);
	   VALUE b = rb_funcall(exception, rb_intern("backtrace"), 0);
	   b = rb_funcall(b, rb_intern("to_s"), 0);

	   char *err = StringValueCStr(m);
	   char *trace = StringValueCStr(b);
	   char *klass = StringValueCStr(c);
	   LOG_ERROR("Error: %s, Trace: %s, Class: %s", err, trace, klass);
	 */
	return 0;
}

static int python_code(const char *fmt, ...)
{
	va_list list;
	va_start(list, fmt);
	char buf[4096];
	vsprintf(buf, fmt, list);
	va_end(list);

	// rly?!
	char sbuf[4096 + 256];
	sprintf(sbuf, "%s", buf);
	LOG_DEBUG(sbuf);

	int error = 0;
	PyRun_SimpleString(sbuf);

	if (error) {
		return handle_exception();
	}
	return 0;
}

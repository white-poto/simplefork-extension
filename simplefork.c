/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_simplefork.h"

#include <zend.h>
#include <zend_API.h>
#include <zend_string.h>

#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>



/* If you declare any globals in php_simplefork.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(simplefork)
*/

/* True global resources - no need for thread safety here */
static int le_simplefork;

static zend_class_entry *simplefork_exception_entry = NULL;

/* SimpleFork\CacheInterface */
zend_class_entry *cache_interface_entry = NULL;

ZEND_BEGIN_ARG_INFO(cache_key, 0)
    ZEND_ARG_INFO(0, cache_key)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(cache_set, 0)
    ZEND_ARG_INFO(0, cache_key)
    ZEND_ARG_INFO(0, cache_value)
ZEND_END_ARG_INFO()

static zend_function_entry cache_interface_methods[]={
    ZEND_ABSTRACT_ME(cache_interface_entry, get, cache_key) 
    ZEND_ABSTRACT_ME(cache_interface_entry, delete, cache_key)
    ZEND_ABSTRACT_ME(cache_interface_entry, set, cache_set) 
    ZEND_ABSTRACT_ME(cache_interface_entry, flush, NULL) 
    {NULL,NULL,NULL}
};

/* SimpleFork\QueueInterface */
zend_class_entry *queue_interface_entry = NULL;

ZEND_BEGIN_ARG_INFO(queue_key, 0)
    ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(queue_put, 0)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, queue_value)
ZEND_END_ARG_INFO()

static zend_function_entry queue_interface_methods[]={
	ZEND_ABSTRACT_ME(queue_interface_entry, put, queue_put)
	ZEND_ABSTRACT_ME(queue_interface_entry, get, queue_key)
	ZEND_ABSTRACT_ME(queue_interface_entry, size, queue_key)
	ZEND_ABSTRACT_ME(queue_interface_entry, remove, NULL)
	{NULL, NULL, NULL}
};

/* SimpleFork\LockInterface */
zend_class_entry *lock_interface_entry = NULL;

static zend_function_entry lock_interface_methods[]={
	ZEND_ABSTRACT_ME(lock_interface_entry, acquire, NULL)
	ZEND_ABSTRACT_ME(lock_interface_entry, release, NULL)
	{NULL,NULL,NULL}
};

/* SimpleFork\Runnable */
zend_class_entry *runnable_interface_entry = NULL;

static zend_function_entry runnable_interface_methods[]={
	ZEND_ABSTRACT_ME(runnable_interface_entry, run, NULL)
	{NULL,NULL,NULL}
};



/* SimpleFork\Process */
zend_class_entry *process_class_entry = NULL;
//

ZEND_BEGIN_ARG_INFO(process_construct_args, 0)
	ZEND_ARG_INFO(0, execution)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(process_set_cache_args, 0)
	ZEND_ARG_OBJ_INFO(1, cache, CacheInterface, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(process_set_queue_args, 0)
	ZEND_ARG_OBJ_INFO(1, queue, QueueInterface, 0)
ZEND_END_ARG_INFO()

static zend_function_entry process_class_methods[]={
	PHP_ME(Process, __construct, process_construct_args, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Process, __destruct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_DTOR)
	PHP_ME(Process, setCache, process_set_cache_args, ZEND_ACC_PUBLIC)
	PHP_ME(Process, setQueue, process_set_queue_args, ZEND_ACC_PUBLIC)
	PHP_ME(Process, getPid, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Process, isAlive, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Process, exitCode, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Process, on, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Process, start, NULL, ZEND_ACC_PUBLIC)
	{NULL,NULL,NULL}
};


/* SimpleFork\Process Methods start */
/** {{{
*/
PHP_METHOD(Process, __construct)
{
	zval *execution = NULL;
	char *execution_name = NULL;
	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "z", &execution)){
		RETURN_FALSE;
	}

	if(!zend_is_callable(execution, 0, &execution_name)){
		zend_throw_exception(simplefork_exception_entry, "execution param must be callable",0 TSRMLS_CC);
	}
	zend_update_property(process_class_entry, getThis(), "execution", sizeof("execution")-1, execution TSRMLS_CC);
}
/* }}} */

/** {{{
*/
PHP_METHOD(Process, __destruct)
{

}
/* }}} */


PHP_METHOD(Process, setCache)
{
	zval *cache = NULL;
	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &cache)){
    	RETURN_FALSE;
    }
	zend_update_property(process_class_entry, getThis(), "cache", sizeof("cache")-1, cache TSRMLS_CC);
}

PHP_METHOD(Process, setQueue)
{
	zval *queue = NULL;
	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &queue)){
		RETURN_FALSE;
	}
	zend_update_property(process_class_entry, getThis(), "queue", sizeof("queue")-1, queue TSRMLS_CC);
}

PHP_METHOD(Process, getPid)
{
	zval *pid = zend_read_property(process_class_entry, getThis(), "pid", sizeof("pid")-1, 0 TSRMLS_DC);
	RETURN_LONG(Z_LVAL_P(pid));
}

PHP_METHOD(Process, isAlive)
{
	zval *is_alive = zend_read_property(process_class_entry, getThis(), "alive", sizeof("alive")-1, 0 TSRMLS_DC);
	zend_bool alive = Z_BVAL_P(is_alive);
	if(alive){
		RETURN_TRUE;
	}else{
		RETURN_FALSE;
	}
}


PHP_METHOD(Process, exitCode)
{
	zval *status = zend_read_property(process_class_entry, getThis(), "status", sizeof("alive")-1, 0 TSRMLS_DC);
	RETURN_LONG(Z_LVAL_P(status));
}

PHP_METHOD(Process, start)
{
	zval *method_name = NULL;
	MAKE_STD_ZVAL(method_name);
	ZVAL_STRING(method_name, "isAlive", 1);
	zval *obj = getThis();
	zval *is_alive = NULL;

	if(call_user_function(NULL, &(obj), method_name, is_alive, 0 ,NULL TSRMLS_CC) != SUCCESS){
		zend_throw_exception(simplefork_exception_entry, "call isAlive method failed", 0 TSRMLS_CC);
	}

	zend_bool alive = Z_BVAL_P(is_alive);
	if(alive){
		zend_throw_exception(simplefork_exception_entry, "the process is running already", 0 TSRMLS_CC);
	}
}

PHP_METHOD(Process, on)
{
	char *event = NULL;
	int event_len = 0;
	zval *callback = NULL;
	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sz", &event, &event_len, &callback)){
    	RETURN_FALSE;
    }

    if(!zend_is_callable(callback, 0, NULL)){
    	zend_throw_exception(simplefork_exception_entry, "try to register a function that it is not callable", 0 TSRMLS_CC);
    }

    zval *callbacks = zend_read_property(process_class_entry, getThis(), "callbacks", sizeof("callbacks")-1, 0 TSRMLS_DC);
    if(callbacks == NULL){
    	MAKE_STD_ZVAL(callbacks);
    	array_init(callbacks);
    }
    add_accos_zval(callbacks, &event, callback);

	RETURN TRUE;
}





/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("simplefork.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_simplefork_globals, simplefork_globals)
    STD_PHP_INI_ENTRY("simplefork.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_simplefork_globals, simplefork_globals)
PHP_INI_END()
*/
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_simplefork_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_simplefork_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "simplefork", arg);
	RETURN_STRINGL(strg, len, 0);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_simplefork_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_simplefork_init_globals(zend_simplefork_globals *simplefork_globals)
{
	simplefork_globals->global_value = 0;
	simplefork_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(simplefork)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/

	zend_class_entry simplefork_exception;
    INIT_CLASS_ENTRY(simplefork_exception, "SimpleForkException", NULL);
    simplefork_exception_entry = zend_register_internal_class_ex(&simplefork_exception, (zend_class_entry *)zend_exception_get_default(), NULL TSRMLS_CC);

	zend_class_entry cache_interface;
  	INIT_NS_CLASS_ENTRY(cache_interface, "SimpleFork", "CacheInterface", cache_interface_methods);
  	cache_interface_entry = zend_register_internal_interface(&cache_interface TSRMLS_CC);

  	zend_class_entry queue_interface;
  	INIT_NS_CLASS_ENTRY(queue_interface, "SimpleFork", "QueueInterface", queue_interface_methods);
  	queue_interface_entry = zend_register_internal_interface(&queue_interface TSRMLS_CC);

    zend_class_entry lock_interface;
    INIT_NS_CLASS_ENTRY(lock_interface, "SimpleFork", "LockInterface", lock_interface_methods);
    lock_interface_entry = zend_register_internal_interface(&lock_interface TSRMLS_CC);

    zend_class_entry runnable_interface;
    INIT_NS_CLASS_ENTRY(runnable_interface, "SimpleFork", "Runnable", runnable_interface_methods);
    runnable_interface_entry = zend_register_internal_interface(&runnable_interface TSRMLS_CC);

    zend_class_entry process_class;
    INIT_NS_CLASS_ENTRY(process_class, "SimpleFork", "Process", process_class_methods);
    process_class_entry = zend_register_internal_class(&process_class TSRMLS_CC);
    zend_declare_property_null(process_class_entry, "queue", strlen("queue"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(process_class_entry, "cache", strlen("cache"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(process_class_entry, "runnable", strlen("runnable"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(process_class_entry, "execution", strlen("execution"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(process_class_entry, "pid", strlen("pid"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(process_class_entry, "alive", strlen("alive"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(process_class_entry, "status", strlen("status"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(process_class_entry, "callbacks", strlen("callbacks"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_class_constant_stringl(process_class_entry, ZEND_STRL("BEFORE_START"), ZEND_STRL("beforeStart") TSRMLS_CC);
    zend_declare_class_constant_stringl(process_class_entry, ZEND_STRL("BEFORE_EXIT"), ZEND_STRL("BEFORE_EXIT") TSRMLS_CC);

	return SUCCESS;
}




/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(simplefork)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(simplefork)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(simplefork)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(simplefork)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "simplefork support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ simplefork_functions[]
 *
 * Every user visible function must have an entry in simplefork_functions[].
 */
const zend_function_entry simplefork_functions[] = {
	PHP_FE(confirm_simplefork_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in simplefork_functions[] */
};
/* }}} */

/* {{{ simplefork_module_entry
 */
zend_module_entry simplefork_module_entry = {
	STANDARD_MODULE_HEADER,
	"simplefork",
	simplefork_functions,
	PHP_MINIT(simplefork),
	PHP_MSHUTDOWN(simplefork),
	PHP_RINIT(simplefork),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(simplefork),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(simplefork),
	PHP_SIMPLEFORK_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_SIMPLEFORK
ZEND_GET_MODULE(simplefork)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

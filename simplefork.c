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
#include <zend_exceptions.h>

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
    ZEND_ABSTRACT_ME(cache_interface_entry, has, cache_key)
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
	ZEND_ARG_INFO(0, runnable)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(process_name_args, 0)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(process_shutdown_args, 0)
	ZEND_ARG_INFO(0, block)
	ZEND_ARG_INFO(0, signal)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(process_wait_args, 0)
	ZEND_ARG_INFO(0, block)
	ZEND_ARG_INFO(0, signal)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(process_register_signal_handler_args, 0)
	ZEND_ARG_INFO(0, block)
	ZEND_ARG_INFO(0, signal)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(process_update_status_args, 0)
	ZEND_ARG_INFO(0, block)
ZEND_END_ARG_INFO()

static zend_function_entry process_class_methods[]={
	PHP_ME(Process, __construct, process_construct_args, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Process, getPid, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Process, name, process_name_args, ZEND_ACC_PUBLIC)
	PHP_ME(Process, updateStatus, process_update_status_args, ZEND_ACC_PUBLIC)
	PHP_ME(Process, isRunning, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Process, isStopped, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Process, isStarted, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Process, errorNo, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Process, errmsg, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Process, ifSignal, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Process, start, NULL, ZEND_ACC_PUBLIC)
//	PHP_ME(Process, shutdown, process_shutdown_args, ZEND_ACC_PUBLIC)
	PHP_ME(Process, wait, process_wait_args, ZEND_ACC_PUBLIC)
/*	PHP_ME(Process, registerSignalHandler, process_register_signal_handler_args, ZEND_ACC_PUBLIC)
	PHP_ME(Process, run, NULL, ZEND_ACC_PUBLIC)

	PHP_ME(Process, signal, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Process, getCallable, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Process, initStatus, NULL, ZEND_ACC_PUBLIC)
	*/
	{NULL,NULL,NULL}
};


/* SimpleFork\Process Methods start */
/** {{{
*/
PHP_METHOD(Process, __construct)
{
	zval *runnable;
	zval *process_name;
	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|z!z!", &runnable, &process_name)){
		RETURN_FALSE;
	}
	if (runnable != NULL && !zend_is_callable(runnable, 0, NULL)) {
        zend_throw_exception(simplefork_exception_entry, "execution param must be callable", 0 TSRMLS_CC);
        return;
    } else if (runnable != NULL) {
        zend_update_property(process_class_entry, getThis(), "runnable", sizeof("runnable")-1, runnable TSRMLS_CC);
    }

    if(process_name != NULL) {
	    zend_update_property(process_class_entry, getThis(), "name", sizeof("name")-1, process_name TSRMLS_CC);
    }

    zval *property_started = zend_read_property(process_class_entry, getThis(), "started", sizeof("started")-1, 0 TSRMLS_DC);
    ZVAL_BOOL(property_started, 0);
}
/* }}} */


PHP_METHOD(Process, getPid)
{
	zval *pid = zend_read_property(process_class_entry, getThis(), "pid", sizeof("pid")-1, 0 TSRMLS_DC);
	RETURN_LONG(Z_LVAL_P(pid));
}

PHP_METHOD(Process, name)
{
    // don't forget to set default value
    char *name = NULL;
    int name_len = 0;
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|s", &name, &name_len)){
        RETURN_FALSE;
    }

    zval *process_name = zend_read_property(process_class_entry, getThis(), "name", sizeof("name")-1, 0 TSRMLS_DC);
    if(!name)
    {
        RETURN_ZVAL(process_name, 1, 0);
    }else {
        ZVAL_STRING(process_name, name, name_len);
    }
}

PHP_METHOD(Process, updateStatus)
{
    long block = 0;
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &block)) {
        RETURN_FALSE;
    }

    zval *is_running = zend_read_property(process_class_entry, getThis(), "running", sizeof("running")-1, 0 TSRMLS_DC);
    if(!Z_BVAL_P(is_running)) {
        RETURN_NULL();
    }

    zval *property_pid = zend_read_property(process_class_entry, getThis(), "pid", sizeof("pid")-1, 0 TSRMLS_DC);
    long pid = Z_LVAL_P(property_pid);
    int stat_loc = 0;
    int wait_stat = 0;
    php_printf("pid:%ld\n", pid);
    if (block) {
        wait_stat = waitpid(pid, &stat_loc);
    }else{
        wait_stat = waitpid(pid, &stat_loc, WNOHANG);
    }
    php_printf("wait_stat:%ld\n", wait_stat);

    if(wait_stat == -1){
        zend_throw_exception(simplefork_exception_entry, "waitpid failed. the process maybe available", 0 TSRMLS_CC);
        return;
    }else if(wait_stat == 0) {
        ZVAL_BOOL(is_running, 1);
    }else {
        int error_no = 0;
        char *errmsg = NULL;
        int term_signal = 0;
        int if_signal = 0;
        int stop_signal = 0;
        error_no = WEXITSTATUS(stat_loc);
        errmsg = strerror(error_no);
        /*
        if(WIFEXITED(stat_loc)) {
            error_no = WEXITSTATUS(stat_loc);
            errmsg = strerror(error_no);
        }else
        */
        if (WIFSIGNALED(stat_loc)) {
            term_signal = WTERMSIG(stat_loc);
            if_signal = 1;
        }else if (WIFSTOPPED(stat_loc)) {
            stop_signal = WSTOPSIG(stat_loc);
        }

        ZVAL_BOOL(is_running, 0);

        zval *property_errno = zend_read_property(process_class_entry, getThis(), "errno", sizeof("errno")-1, 0 TSRMLS_DC);
        ZVAL_LONG(property_errno, error_no);

        zval *property_errmsg = zend_read_property(process_class_entry, getThis(), "errmsg", sizeof("errmsg")-1, 0 TSRMLS_DC);
        ZVAL_STRING(property_errmsg, errmsg, strlen(errmsg));

        zval *property_term_signal = zend_read_property(process_class_entry, getThis(), "term_signal", sizeof("term_signal")-1, 0 TSRMLS_DC);
        ZVAL_LONG(property_term_signal, term_signal);

        zval *property_if_signal = zend_read_property(process_class_entry, getThis(), "if_signal", sizeof("if_signal")-1, 0 TSRMLS_DC);
        ZVAL_BOOL(property_if_signal, if_signal);

        zval *property_stop_signal = zend_read_property(process_class_entry, getThis(), "stop_signal", sizeof("stop_signal")-1, 0 TSRMLS_DC);
        ZVAL_LONG(property_stop_signal, stop_signal);
    }
}

PHP_METHOD(Process, isRunning)
{
    zval *retval_ptr;

    zval method_name;
    INIT_ZVAL(method_name);
    ZVAL_STRING(&method_name, "updateStatus", 1);
    if (call_user_function_ex(
        CG(function_table), &getThis(), &method_name,
        &retval_ptr, 0, NULL, 0, NULL TSRMLS_CC
    ) == FAILURE
    ) {
        zend_throw_exception(simplefork_exception_entry, "call updateStatus failed", 0 TSRMLS_CC);
        return;
    }

    zval_ptr_dtor(&retval_ptr);
    zval_dtor(&method_name);

    zval *running = zend_read_property(process_class_entry, getThis(), "running", sizeof("running")-1, 0 TSRMLS_DC);
    RETURN_ZVAL(running, 1, 0);
}

PHP_METHOD(Process, isStopped)
{
    zval *retval_ptr;

    zval method_name;
    INIT_ZVAL(method_name);
    ZVAL_STRING(&method_name, "isRunning", 1);
    if (call_user_function_ex(
        CG(function_table), &getThis(), &method_name,
        &retval_ptr, 0, NULL, 0, NULL TSRMLS_CC
    ) == FAILURE
    ) {
        zend_throw_exception(simplefork_exception_entry, "call updateStatus failed", 0 TSRMLS_CC);
        return;
    }

    zval *is_running;
    MAKE_STD_ZVAL(is_running);
    if(Z_BVAL_P(retval_ptr) == 0) {
        ZVAL_BOOL(is_running, 1);
    }else {
        ZVAL_BOOL(is_running, 0);
    }

    zval_ptr_dtor(&retval_ptr);
    zval_dtor(&method_name);

    RETURN_ZVAL(is_running, 1, 0);
}

PHP_METHOD(Process, isStarted)
{
    zval *is_started = zend_read_property(process_class_entry, getThis(), "started", sizeof("started")-1, 0 TSRMLS_DC);
    RETURN_ZVAL(is_started, 1, 0);
}

PHP_METHOD(Process, errorNo)
{
    zval *error_no = zend_read_property(process_class_entry, getThis(), "errno", sizeof("errno")-1, 0 TSRMLS_DC);
    RETURN_ZVAL(error_no, 1, 0);
}

PHP_METHOD(Process, errmsg)
{
    zval *errmsg = zend_read_property(process_class_entry, getThis(), "errmsg", sizeof("errmsg")-1, 0 TSRMLS_DC);
    RETURN_ZVAL(errmsg, 1, 0);
}

PHP_METHOD(Process, ifSignal)
{
    zval *if_signal = zend_read_property(process_class_entry, getThis(), "if_signal", sizeof("if_signal")-1, 0 TSRMLS_DC);
    RETURN_ZVAL(if_signal, 1, 0);
}

PHP_METHOD(Process, start)
{
	zval method_name;
    INIT_ZVAL(method_name);
    ZVAL_STRING(&method_name, "isStarted", 1);
	zval *is_started = NULL;

	if(call_user_function_ex(CG(function_table),
	    &getThis(), &method_name, &is_started,
	    0, NULL, 0, NULL TSRMLS_CC) != SUCCESS){
		zend_throw_exception(simplefork_exception_entry, "call isStarted method failed", 0 TSRMLS_CC);
		return;
	}

	zend_bool started = Z_BVAL_P(is_started);
	if(started){
	    zval_ptr_dtor(&is_started);
        zval_dtor(&method_name);
		zend_throw_exception(simplefork_exception_entry, "the process is started already", 0 TSRMLS_CC);
		return;
	}
    zval_ptr_dtor(&is_started);
    zval_dtor(&method_name);

	pid_t pid = fork();
	if(pid < 0) {
	    zend_throw_exception(simplefork_exception_entry, "fork failed", 0 TSRMLS_CC);
	    return;
	}else if(pid > 0) {
	    zval *property_pid = zend_read_property(process_class_entry, getThis(), "pid", sizeof("pid")-1, 0 TSRMLS_DC);
        ZVAL_LONG(property_pid, pid);

        zval *property_running = zend_read_property(process_class_entry, getThis(), "running", sizeof("running")-1, 0 TSRMLS_DC);
        ZVAL_BOOL(property_running, 1);

        zval *property_started = zend_read_property(process_class_entry, getThis(), "started", sizeof("started")-1, 0 TSRMLS_DC);
        ZVAL_BOOL(property_started, 1);
	}else {
	    zval *runnable = zend_read_property(process_class_entry, getThis(), "runnable", sizeof("runnable")-1, 0 TSRMLS_DC);
	    if (Z_TYPE_P(runnable) != IS_NULL && !zend_is_callable(runnable, 0, NULL)) {
            zend_throw_exception(simplefork_exception_entry, "execution param must be callable", 0 TSRMLS_CC);
            return;
        }else if(Z_TYPE_P(runnable) != IS_NULL) {
            zval *retval_ptr;
            if (call_user_function_ex(
                CG(function_table), NULL, runnable,
                &retval_ptr, 0, NULL, 0, NULL TSRMLS_CC
            ) == FAILURE
            ) {
                zend_throw_exception(simplefork_exception_entry, "call runnable failed", 0 TSRMLS_CC);
                return;
            }
            zval_ptr_dtor(&retval_ptr);
        }else{
            zval *retval_ptr;

            zval callback_name;
            INIT_ZVAL(callback_name);
            ZVAL_STRING(&callback_name, "run", 1);
            if (call_user_function_ex(
                CG(function_table), &getThis(), &callback_name,
                &retval_ptr, 0, NULL, 0, NULL TSRMLS_CC
            ) == FAILURE
            ) {
                zend_throw_exception(simplefork_exception_entry, "call method run failed", 0 TSRMLS_CC);
                return;
            }

            zval_ptr_dtor(&retval_ptr);
            zval_dtor(&callback_name);
            php_printf("exit========\n");
            exit(0);
        }
	}
}

PHP_METHOD(Process, run)
{

}

PHP_METHOD(Process, wait)
{
	zend_bool *block = NULL;
	long sleep = 100;
	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|bl", &block, sleep)){
        RETURN_FALSE;
    }

    zval *running = zend_read_property(process_class_entry, getThis(), "running", sizeof("running")-1, 0 TSRMLS_DC);
	while(1){
	    php_printf("runnnnnnnnnnn\n");

        zval *retval_ptr;

        zval method_name;
        INIT_ZVAL(method_name);
        ZVAL_STRING(&method_name, "updateStatus", 1);
        if (call_user_function_ex(
            CG(function_table), &getThis(), &method_name,
            &retval_ptr, 0, NULL, 0, NULL TSRMLS_CC
        ) == FAILURE
        ) {
            zend_throw_exception(simplefork_exception_entry, "call updateStatus failed", 0 TSRMLS_CC);
            return;
        }

        zval_ptr_dtor(&retval_ptr);
        zval_dtor(&method_name);

        int is_running = Z_BVAL_P(running);
        php_printf("running:%ld\n", is_running);
        if(is_running == 0){
            RETURN_TRUE;
        }

		if(!block || &block == 0){
			RETURN_FALSE;
		}

		usleep(sleep);
	}
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
    zend_declare_property_null(process_class_entry, "runnable", strlen("runnable"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(process_class_entry, "pid", strlen("pid"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(process_class_entry, "name", strlen("name"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(process_class_entry, "started", strlen("started"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(process_class_entry, "running", strlen("running"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(process_class_entry, "term_signal", strlen("term_signal"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(process_class_entry, "stop_signal", strlen("stop_signal"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(process_class_entry, "errno", strlen("errno"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(process_class_entry, "errmsg", strlen("errmsg"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(process_class_entry, "if_signal", strlen("if_signal"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(process_class_entry, "signal_handlers", strlen("signal_handlers"), ZEND_ACC_PROTECTED TSRMLS_CC);

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

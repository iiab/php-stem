/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 2002-2009 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt.                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Snowball sources copyright (c) M.F. Porter                           |
  +----------------------------------------------------------------------+
  | Author: Jay Smith     <jay@php.net>                                  |
  |         Olivier Hill  <ohill@php.net>                                |
  +----------------------------------------------------------------------+
*/

/* $Id: stem.c 338739 2016-03-14 14:10:31Z ohill $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_stem.h"
#if PHP_MAJOR_VERSION >= 7
ZEND_BEGIN_ARG_INFO_EX(arginfo_stem_params, 0, 0, 1)
	ZEND_ARG_INFO(0, string)
	ZEND_ARG_INFO(0, lang)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stem_params_lang_preset, 0, 0, 1)
	ZEND_ARG_INFO(0, string)
	ZEND_ARG_INFO(0, lang)
ZEND_END_ARG_INFO()
#else
#define arginfo_stem_params NULL
#define arginfo_stem_params_lang_preset NULL
#endif

/* {{{ some forward declarations: */
static short int stem_enabled(int algorithm);
/* }}} */

/* {{{ stem_functions[]
*/
zend_function_entry stem_functions[] = {
	PHP_FE(stem,				arginfo_stem_params)
	PHP_FE(stem_enabled,		arginfo_stem_params)

#define STEMMER(php_func, c_func, constant, name) \
	PHP_FE(stem_ ## php_func,	arginfo_stem_params_lang_preset)
#include "stemmers.def"
#undef STEMMER

	{NULL, NULL, NULL}	
};
/* }}} */

/* {{{ stem_module_entry
 */
zend_module_entry stem_module_entry = {
	STANDARD_MODULE_HEADER,
	"stem",
	stem_functions,
	PHP_MINIT(stem),
	PHP_MSHUTDOWN(stem),
	NULL,
	NULL,
	PHP_MINFO(stem),
	PHP_STEM_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_STEM
ZEND_GET_MODULE(stem)
#endif

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(stem)
{
	/* Just set up our constants for the PHP function stem(). */

#define STEMMER_FORCE 1
#define STEMMER(php_func, c_func, constant, name) \
	REGISTER_LONG_CONSTANT("STEM_" # constant, STEM_ ## constant, CONST_CS | CONST_PERSISTENT);
#include "stemmers.def"
#undef STEMMER_FORCE
#undef STEMMER

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(stem)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(stem)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "stem support", "enabled");
	php_info_print_table_row(2, "version", PHP_STEM_VERSION);
	php_info_print_table_colspan_header(2,
	#ifdef COMPILE_DL_STEM
	"compiled as dynamic module"
	#else
	"compiled as static module"
	#endif
	);
	php_info_print_table_colspan_header(2, "Languages Supported");

#define STEMMER_FORCE 1
#define STEMMER(php_func, c_func, constant, name) \
		php_info_print_table_row(2, # name, stem_enabled(STEM_ ## constant) ? "enabled" : "disabled");
#include "stemmers.def"
#undef STEMMER_FORCE
#undef STEMMER

	php_info_print_table_end();
}
/* }}} */

#ifdef PHP_WIN32
#define INIT_FUNCS(x)		create_env = (struct SN_env*) x ## _create_env; \
							stem = (int*) x ## _stem; \
							close_env = (void*) x ## _close_env;
#else
#define INIT_FUNCS(x)		create_env = x ## _create_env; \
							stem = x ## _stem; \
							close_env = x ## _close_env;
#endif


/* {{{ void php_stem(INTERNAL_FUNCTION_PARAMETERS, long lang)
   Return a stemmed string. lang is one of the language constants. By default,
   STEM_PORTER is used. Returns the stemmed word on success, false if there is
   a Snowball error. This is called from the actual PHP functions listed below.
*/
void php_stem(INTERNAL_FUNCTION_PARAMETERS, long lang)
{
	struct SN_env* z;
	struct SN_env* (*create_env)(void);
	void (*close_env)(struct SN_env*);
	int (*stem)(struct SN_env*);

#if PHP_MAJOR_VERSION >= 7
	const char* incoming;
	size_t arglen;
#else
	char * incoming;
	int arglen;
#endif

	if (zend_parse_parameters_ex(0, MIN(ZEND_NUM_ARGS(),2), "s|l", &incoming, &arglen, &lang) == FAILURE) {
		return;
	}

	/* Empty string */
	if (arglen <= 0) {
#if PHP_MAJOR_VERSION >= 7
		RETURN_STRINGL(incoming, arglen);
#else
		RETURN_STRINGL(incoming, arglen, 1);
#endif
	}

	switch (lang)
	{
		case STEM_DEFAULT:

#define STEMMER(php_func, c_func, constant, name) \
		case STEM_ ## constant: \
			INIT_FUNCS(c_func) \
			break;
#include "stemmers.def"
#undef STEMMER
			
		default:
			php_error(E_NOTICE, "%s() couldn't stem word, stemming module not found", get_active_function_name());
			RETURN_FALSE;
	}

	z = create_env();
	SN_set_current(z, arglen, incoming);
	php_strtolower(z->p, arglen);
	stem(z);
	z->p[z->l]= '\0';

#if PHP_MAJOR_VERSION >= 7
	RETVAL_STRINGL(z->p,z->l);
#else
	RETVAL_STRINGL(z->p,z->l, 1);
#endif
	close_env(z);
}
/* }}} */


/* {{{ string stem(string arg [, int lang])
   arg is a string to be stemmed, lang is the language constant listed above.
   STEM_PORTER is used by default. Returns false on Snowball error or the
   stemmed word on success.
*/
PHP_FUNCTION(stem)
{
	php_stem(INTERNAL_FUNCTION_PARAM_PASSTHRU, STEM_DEFAULT);
}
/* }}} */

/* {{{ just check to see if an algorithm is enabled... */
static short int stem_enabled(int algorithm)
{
	switch (algorithm) {
#		define STEMMER(php_func, c_func, constant, name) \
			case STEM_ ## constant:
#		include "stemmers.def"
#		undef STEMMER
			return 1;
	}
	return 0;
}
/* }}} */


/* {{{ bool stem_enabled(int lang)
   lang is one of the language constants. This function just returns true
   if the language is available and false if it isn't.
*/
PHP_FUNCTION(stem_enabled)
{
	int lang;

	if (zend_parse_parameters(ZEND_NUM_ARGS() , "l", &lang) == FAILURE) {
		RETURN_FALSE;
	}

	if (stem_enabled(lang)) {
		RETURN_TRUE;
	}
	else {
		RETURN_FALSE;
	}
}
/* }}} */


/* {{{ string stem_LANG(string arg)
   arg is a string to be stemmed. The language is determined by _LANG, which
   will correspond to any of the available languages. Calling stem_LANG(string)
   is the equivalent of calling stem(string, LANG), i.e. stem_french(string) is
   equivalent to stem(string, STEM_FRENCH).
*/
#define STEMMER(php_func, c_func, constant, name) \
	PHP_FUNCTION(stem_ ## php_func) \
	{ \
		php_stem(INTERNAL_FUNCTION_PARAM_PASSTHRU, STEM_ ## constant); \
	}
#include "stemmers.def"
#undef STEMMER
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 tw=78 fdm=marker
 * vim<600: sw=4 ts=4 tw=78
 */

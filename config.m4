dnl $Id$
dnl config.m4 for extension simplefork

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(simplefork, for simplefork support,
dnl Make sure that the comment is aligned:
dnl [  --with-simplefork             Include simplefork support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(simplefork, whether to enable simplefork support,
Make sure that the comment is aligned:
[  --enable-simplefork           Enable simplefork support])

if test "$PHP_SIMPLEFORK" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-simplefork -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/simplefork.h"  # you most likely want to change this
  dnl if test -r $PHP_SIMPLEFORK/$SEARCH_FOR; then # path given as parameter
  dnl   SIMPLEFORK_DIR=$PHP_SIMPLEFORK
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for simplefork files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       SIMPLEFORK_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$SIMPLEFORK_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the simplefork distribution])
  dnl fi

  dnl # --with-simplefork -> add include path
  dnl PHP_ADD_INCLUDE($SIMPLEFORK_DIR/include)

  dnl # --with-simplefork -> check for lib and symbol presence
  dnl LIBNAME=simplefork # you may want to change this
  dnl LIBSYMBOL=simplefork # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $SIMPLEFORK_DIR/$PHP_LIBDIR, SIMPLEFORK_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_SIMPLEFORKLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong simplefork lib version or lib not found])
  dnl ],[
  dnl   -L$SIMPLEFORK_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(SIMPLEFORK_SHARED_LIBADD)

  PHP_NEW_EXTENSION(simplefork, simplefork.c, $ext_shared)
fi



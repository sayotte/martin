#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([martin], [0.1], [stephen.ayotte@gmail.com])
AC_CONFIG_HEADER([version.h])

AC_ARG_WITH(http-parser, [  --with-http-parser=DIR  Path to http-parser installation @<:@./http-parser@:>@], [http_parser_path=$withval], [http_parser_path=http-parser])
AC_ARG_WITH(pcre, [  --with-pcre=DIR         Path to pcre installation @<:@./pcre@:>@], [pcre_path=$withval], [pcre_path=pcre])
AC_ARG_WITH(libev, [  --with-libev=DIR        Path to libev installation @<:@./libev@:>@], [libev_path=$withval], [libev_path=libev])

# Checks for programs.
AC_PROG_CC
AC_LANG(C)

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([limits.h string.h inttypes.h wait.h sys/wait.h])
AC_CHECK_HEADER(malloc.h, [have_malloc_h=1], [have_malloc_h=0])
AC_CHECK_HEADER(malloc/malloc.h, [have_malloc_malloc_h=1], [have_malloc_malloc_h=0])
if test "$have_malloc_h" == "1"; then
    AC_SUBST(malloc_header, ["#include <malloc.h>"])
else 
    if test "$have_malloc_malloc_h" == "1"; then
        AC_SUBST(malloc_header, ["#include <malloc/malloc.h>"])
    else
        AC_MSG_FAILURE([No malloc.h variant found!])
    fi
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_FORK
#AC_CHECK_FUNCS([malloc_size malloc_usable_size reallocf])
AC_CHECK_FUNCS(malloc_size, [AC_SUBST(have_malloc_size, 1)], [AC_SUBST(have_malloc_size, 0)])
AC_CHECK_FUNCS(malloc_usable_size, [AC_SUBST(have_malloc_usable_size, 1)], [AC_SUBST(have_malloc_usable_size, 0)])
AC_SEARCH_LIBS([dlsym], [dl], [AC_SUBST(libdl, -ldl)]) 

AC_MSG_CHECKING([what extension is used for shared objects])
if test "`uname -s`" == "Darwin"; then
    AC_MSG_RESULT([.dylib])
    AC_SUBST(so_suffix, .dylib)
else
    AC_MSG_RESULT([.so])
    AC_SUBST(so_suffix, .so)
    AC_SUBST(export_dynamic, [-rdynamic])
fi
AC_SUBST(http_parser_path, $http_parser_path)
AC_SUBST(pcre_path, $pcre_path)
AC_SUBST(libev_path, $libev_path)

AC_OUTPUT(config.h)
AC_OUTPUT(Makefile)
AC_OUTPUT(test/Makefile)

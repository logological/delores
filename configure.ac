#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# $Id: configure.ac,v 1.8 2003-12-14 14:47:34 psy Exp $

AC_PREREQ(2.57)
AC_INIT(DELORES, 0.90, Tristan.Miller@dfki.de)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/dl.c])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_LEX
if test "$LEX" != flex; then
  LEX="$SHELL $missing_dir/missing flex"
  AC_SUBST(LEX_OUTPUT_ROOT, lex.yy)
  AC_SUBST(LEXLIB, '')
fi
AC_PROG_YACC

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([inttypes.h limits.h stddef.h stdint.h stdlib.h string.h sys/timeb.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_CHECK_TYPES([unsigned long long, uintmax_t])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_CHECK_FUNCS([ftime memset strrchr strtoul strtoull strdup strtoumax])

# Custom configuration
AC_DEFUN([ENABLE_BGET],
[AC_ARG_WITH([bget],
             AC_HELP_STRING([--with-bget],
                            [use BGET (default is NO)]),
             [ac_cv_use_bget=$withval], [ac_cv_use_bget=no])
AC_CACHE_CHECK([whether to use BGET],
               [ac_cv_use_bget],
               [ac_cv_use_bget=no
                true && ac_cv_use_bget=yes])
if test $ac_cv_use_bget = yes; then
   AC_DEFINE([DL_USE_BGET], 1,
             [Define if you want to use the BGET library.])
fi
])

ENABLE_BGET
AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT

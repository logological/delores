#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# $Id: configure.ac,v 1.3 2003-12-12 13:57:56 psy Exp $

AC_PREREQ(2.57)
AC_INIT(DELORES, 1.0, psychonaut@nothingisreal.com)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/dl.c])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([inttypes.h limits.h stddef.h stdint.h stdlib.h string.h sys/timeb.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_CHECK_FUNCS([ftime memset strrchr strtoul strtoull strdup strtoumax])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT

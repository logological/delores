# $Id: Makefile.am,v 1.11 2003-12-13 19:54:10 psy Exp $
#
# Copyright (C) 1999, 2000 Michael Maher <mjm@math.luc.edu>
# Copyright (C) 1999, 2000, 2003 Tristan Miller <psychonaut@nothingisreal.com>
#  
# This file is free software; the author gives unlimited permission to
# copy and/or distribute it, with or without modifications.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even
# the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
# PURPOSE.

BUILT_SOURCES = parser.h
AM_YFLAGS = -d
AM_CPPFLAGS = -DPRIME_FILE=\"$(datadir)/delores/primes.txt\"
bin_PROGRAMS = delores
delores_SOURCES = bget.c bget.h cmd_line_args.h cmd_line_args.l dl.c dl.h lexer.l dl_malloc.c dl_malloc.h dl_stdbool.h dl_stdint.h dl_strdup.c dl_strdup.h parser.y main.c ohash.c ohash.h timer.c timer.h
delores_LDADD = @LEXLIB@ @LIBS@
dist_pkgdata_DATA = primes.txt

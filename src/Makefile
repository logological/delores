# $Id: Makefile,v 1.2 2003-12-09 19:15:29 psy Exp $

CC = gcc 
CFLAGS = -g -Wall -pedantic
PFLAGS = -pg -a
OFLAGS = -O3
OBJS = dl_lexyy.o dl_y_tab.o ohash.o dl.o timer.o dl_strdup.o dl_malloc.o bget.o cmd_line_args.o main.o
LIBS = -lfl
FLEX = flex
FLEXFLAGS = 
BISON = bison
BISONFLAGS = 
AR = ar
RM = rm -rf
COMPRESS = gzip
CHMOD = chmod 755

all: dl

#Compile a standard executable with debug info
dl:	$(OBJS) dl_compile.h dl.h dl_strdup.h dl_malloc.h dl_stdint.h timer.h
	$(CC) $(CFLAGS) -odl $(OBJS) $(LIBS)
	$(CHMOD) dl

#Compile an optimized executable with no debug info
optimized: $(OBJS) dl_compile.h dl.h dl_strdup.h dl_malloc.h dl_stdint.h timer.h
	$(CC) $(OFLAGS) -odl $(OBJS) $(LIBS)
	strip dl

#Compile in the electric fence library for catching malloc() bugs
efence: $(OBJS) dl_compile.h dl.h dl_strdup.h dl_malloc.h dl_stdint.h timer.h
	$(CC) $(CFLAGS) -odl $(OBJS) -lefence $(LIBS)

#Compile in necessary profiling info for prof or gprof
prof: $(OBJS) dl_compile.h dl.h dl_strdup.h dl_malloc.h dl_stdint.h timer.h
	$(CC) $(CFLAGS) $(PFLAGS) -odl $(OBJS) $(LIBS)

#Don't use any nonstandard extensions (e.g. strdup())
ansi: $(OBJS) dl_compile.h dl.h dl_strdup.h dl_malloc.h dl_stdint.h timer.h
	$(CC) -ansi $(CFLAGS) -odl $(OBJS) $(LIBS)

#Remove all intermediate source/object files
clean:
	$(RM) core
	$(RM) *.o
	$(RM) dl_lexyy.c
	$(RM) dl_y_tab.h
	$(RM) dl_y_tab.c
	$(RM) cmd_line_args.c

#Make a backup of the current working directory
bak:
	make clean
	$(RM) bak
	mkdir bak
	cp *.* bak
	$(RM) bak/*~
	$(RM) bak/#*
	cp Makefile bak
	$(COMPRESS) bak/* &

dl_lexyy.o dl_y_tab.o:	dl_lexyy.c dl_y_tab.h

dl_y_tab.c dl_y_tab.h:	dl.y dl.h dl_compile.h dl_malloc.h dl_stdint.h dl_stdbool.h
	$(BISON) -odl_y_tab.c -d dl.y

dl_lexyy.c:	dl.l dl.h dl_compile.h dl_strdup.h dl_malloc.h dl_stdint.h
	$(FLEX) -odl_lexyy.c dl.l

cmd_line_args.c: cmd_line_args.l cmd_line_args.h dl_compile.h dl_malloc.h dl_stdint.h dl_stdbool.h dl.h
	$(FLEX) -ocmd_line_args.c -Pxx cmd_line_args.l

dl.c:
	#Don't ask me why this rule is here;
	#the project doesn't want to build without it...
	#I think this is due to some quirk of GNU make.

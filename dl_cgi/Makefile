#You may wish to add -DNO_UNISTD and -DNO_SYSTEM to CFLAGS if
#your system lacks the /usr/include/unistd.h header file
#or the system() function, respectively. Non-unix systems
#will also definitely have different linker syntax.

CFLAGS=
CC=gcc
AR=ar
LIBS=-L./ -lcgic

all: libcgic.a dl_cgi.cgi

libcgic.a: cgic.o cgic.h
	rm -f libcgic.a
	$(AR) rc libcgic.a cgic.o

dl_cgi.cgi:	dl_cgi.o libcgic.a
	gcc dl_cgi.o -o dl_cgi.cgi ${LIBS}
	chmod 755 dl_cgi.cgi







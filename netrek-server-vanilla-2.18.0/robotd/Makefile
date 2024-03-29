
srcdir = .
CONFIG = system.mk
include ../$(CONFIG)

.PHONY: clean reallyclean install checkin checkout tar dist depend

DESTDIR = /usr/local/games/netrek-server-vanilla/og
XTREKDIR = /usr/local/games/netrek-server-vanilla
XTREKOWNER = tjk

R_OBJS = 	   \
	    robot.o\
	    data.o \
	    death.o \
	    decide.o \
	    defaults.o \
	    disengage.o \
	    dmessage.o \
	    dodge.o \
	    engage.o \
	    enter.o \
	    escort.o \
	    findslot.o \
	    getarmies.o \
	    getname.o \
	    getship.o \
	    main.o \
	    master.o \
	    newwin.o	\
	    ogg.o \
	    parse.o \
	    ping.o \
	    reserved.o \
	    shmem.o \
	    sintab.o \
	    socket.o \
	    assault.o \
	    util.o\
	    update_players.o \
	    update_planets.o \
	    warfare.o\
	    pwarfare.o \
	    rwarfare.o \
	    ships.o

R_FILES = 	   \
	    ${srcdir}/robot.c\
	    ${srcdir}/data.c \
	    ${srcdir}/death.c \
	    ${srcdir}/decide.c \
	    ${srcdir}/defaults.c \
	    ${srcdir}/disengage.c \
	    ${srcdir}/dmessage.c \
	    ${srcdir}/dodge.c \
	    ${srcdir}/engage.c \
	    ${srcdir}/enter.c \
	    ${srcdir}/escort.c \
	    ${srcdir}/findslot.c \
	    ${srcdir}/getarmies.c \
	    ${srcdir}/getname.c \
	    ${srcdir}/getship.c \
	    ${srcdir}/input.c \
	    ${srcdir}/main.c \
	    ${srcdir}/master.c \
	    ${srcdir}/newwin.c	\
	    ${srcdir}/ogg.c	\
	    ${srcdir}/parse.c \
	    ${srcdir}/ping.c \
	    ${srcdir}/redraw.c \
	    ${srcdir}/reserved.c \
	    ${srcdir}/shmem.c \
	    ${srcdir}/sintab.c \
	    ${srcdir}/socket.c \
	    ${srcdir}/assault.c \
	    ${srcdir}/util.c \
	    ${srcdir}/update_players.c \
	    ${srcdir}/update_planets.c \
	    ${srcdir}/warfare.c \
	    ${srcdir}/pwarfare.c \
	    ${srcdir}/rwarfare.c \
	    ${srcdir}/ships.c 

INCLUDES = struct.h \
	    packets.h \
	    defs.h \
	    copyright.h \
	    copyright2.h \
	    data.h \
	    robot.h 

INPUT = input.o redraw.o

LIBS =  -lgdbm -lresolv -lm 
EXTRALIBS = $(LINKFLAGS)

CC = gcc
CFLAGS =  -funroll-loops -finline-functions -DATM -DPING -DHAVE_NINT=0
LDFLAGS =  
#CFLAGS = -O6 -fomit-frame-pointer -DATM -DPING 

robot: $(R_OBJS) $(INPUT)
	$(CC) $(CFLAGS) ${LDFLAGS} -o robot $(R_OBJS) $(INPUT) $(LIBS) $(EXTRALIBS)

install:	robot
	-mkdir -p $(DESTDIR)$(LIBDIR)/og
	cp robot $(DESTDIR)$(LIBDIR)/og/robot

_robot: $(R_OBJS) $(INPUT)
	$(CC) $(CFLAGS) ${LDFLAGS} -o _robot $(R_OBJS) $(INPUT) $(LIBS) $(EXTRALIBS)

__robot: $(R_OBJS) $(INPUT)
	$(CC) $(CFLAGS) ${LDFLAGS} -o __robot $(R_OBJS) $(INPUT) $(LIBS) $(EXTRALIBS)

clean:
	rm -f $(R_OBJS) $(INPUT) $(X10OBJS) $(X11OBJS) $(GLOBJS)

reallyclean: clean
	rm -rf robot robot.old

tags: $(R_FILES)
	ctags $(R_FILES) $(INCLUDES)

saber :
	#source .suppress
	#load main.c $(R_FILES) -lm

checkin:
	@echo -n 'revision? '; read v; \
	ci -f$${v} $(R_FILES) $(INCLUDES) Makefile

checkout:
	@-co -q $(R_FILES) $(INCLUDES)

tar:
	tar cvf - $(INCLUDES) $(R_FILES) Makefile | compress > ../robot.tar.Z
	mv ../robot.tar.Z .

dist:
	mkdir robotd
	cp $(INCLUDES) $(R_FILES) robotd
	cp Makefile.dist robotd/Makefile
	cp reserved.c.SAFE robotd/reserved.c
	tar cvf robot.tar robotd
	compress robot.tar
	/bin/rm -rf robotd

#####

depend:
	@mymkdep -f Makefile -MM $(R_FILES)
# DO NOT DELETE THIS LINE -- mkdep uses it.

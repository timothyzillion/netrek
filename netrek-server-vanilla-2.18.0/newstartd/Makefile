# Change CONFIG to be the .mk file that matches your system in config

CONFIG = ../system.mk

srcdir = .

# Do not change anything below this line. They should be setup in config.

include $(CONFIG)

INSTALL = /usr/bin/install -c
INSTALLPROG = ${INSTALL}
INSTALLDATA = ${INSTALL} -m 644

N_SRC = ${srcdir}/newstartd.c
N_OBJS = newstartd.o ../ntserv/libnetrek.a

LIBS = $(EXTRALIBS)

CFLAGS = $(FLAGS) $(EXTRAFLAGS) $(PORT) -I../include

all: newstartd

newstartd: $(N_OBJS)
	$(CC) $(CFLAGS) ${LDFLAGS} -o $@ $(N_OBJS) $(LIBS)

clean: rmdependencies
	/bin/rm -f *.o make.out core

reallyclean:  clean
	rm -f newstartd *~ #*#

uninstall:
	-cd $(LIBDIR); rm -f newstartd

install: newstartd
	mkdir -p $(DESTDIR)$(LIBDIR)
	$(INSTALLPROG) $(INSTALLOPTS) newstartd $(DESTDIR)$(LIBDIR)/newstartd
	mkdir -p $(DESTDIR)$(BINDIR)
	ln -sf $(LIBDIR)/newstartd $(DESTDIR)$(BINDIR)/netrekd

depend:
	${CC} -M  $(CFLAGS) $(N_SRC) > .depend

rmdependencies:
	rm -f .depend

sinclude .depend
# DO NOT DELETE THIS LINE -- make depend depends on it.

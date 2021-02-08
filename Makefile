#
# Makefile for rogue
# %W% (Berkeley) %G%
#

DISTNAME=rogue3.6.2

HDRS   = rogue.h daemon.h
CFILES = vers.c armor.c chase.c command.c daemon.c daemons.c fight.c \
	 init.c io.c list.c main.c misc.c monsters.c move.c newlevel.c \
	 options.c pack.c passages.c potions.c rings.c rip.c rooms.c \
	 save.c scrolls.c state.c sticks.c things.c weapons.c wizard.c
OBJS   = $(CFILES:.c=.o)

MISC   = Makefile LICENSE.TXT rogue.6 rogue.r

CC     = gcc
CFLAGS = -O3
CRLIB  = -lcurses
RM     = rm -f
TAR    = tar

rogue: $(HDRS) $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(CRLIB) -o $@

tags: $(HDRS) $(CFILES)
	ctags $^

clean:
	rm -f $(OBJS) core 
	rm -f rogue rogue.exe rogue.tar rogue.tar.gz rogue.cat rogue.doc

cfiles: $(CFILES)

dist.src:
	make clean
	tar cf $(DISTNAME)-src.tar $(CFILES) $(HDRS) $(MISC)
	gzip $(DISTNAME)-src.tar

dist.linux:
	make clean
	make rogue
	groff -P-c -t -ms -Tascii rogue.r | sed -e 's/.\x08//g' > rogue.doc
	groff -man rogue.6 | sed -e 's/.\x08//g' > rogue.cat
	tar cf $(DISTNAME)-linux.tar rogue LICENSE.TXT rogue.cat rogue.doc
	gzip $(DISTNAME)-linux.tar
	
dist.cygwin:
	make clean
	make rogue
	groff -P-c -t -ms -Tascii rogue.r | sed -e 's/.\x08//g' > rogue.doc
	groff -P-c -man -Tascii rogue.6 | sed -e 's/.\x08//g' > rogue.cat
	tar cf $(DISTNAME)-cygwin.tar rogue.exe LICENSE.TXT rogue.cat rogue.doc
	gzip -f $(DISTNAME)-cygwin.tar

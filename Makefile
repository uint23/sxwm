CC = cc

PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

LIBS = -lX11 -lXinerama -lXcursor

CPPFLAGS = -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700
CFLAGS = -std=c99 -pedantic -Wall -Wextra -O2 ${CPPFLAGS} -I/usr/X11R6/include
LDFLAGS = ${LIBS} -L/usr/X11R6/lib

SRC = sxwm.c parser.c
HDR = common.h extern.h parser.h
OUT = sxwm

all: ${SRC} ${HDR}
	${CC} ${SRC} ${CFLAGS} ${LDFLAGS} -o ${OUT} ${LDFLAGS}

clean:
	rm -rf ${OUT}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f ${OUT} ${DESTDIR}${PREFIX}/bin/
	chmod 755 ${DESTDIR}${PREFIX}/bin/sxwm
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	cp -f docs/sxwm.1 ${DESTDIR}${MANPREFIX}/man1/
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/sxwm.1
	mkdir -p ${DESTDIR}${PREFIX}/share
	cp -f default_sxwmrc ${DESTDIR}${PREFIX}/share/sxwmrc

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/sxwm \
	      ${DESTDIR}${MANPREFIX}/man1/sxwm.1 \
	      ${DESTDIR}${PREFIX}/share/sxwmrc

compile_flags:
	rm -f compile_flags.txt
	for f in ${CFLAGS}; do echo $$f >> compile_flags.txt; done

.PHONY: all clean install uninstall compile_flags


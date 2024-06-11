all:	notscotx notscorx

SQLINC=$(shell mariadb_config --include)
SQLLIB=$(shell mariadb_config --libs)

CCOPTS=${SQLINC} -I. -I/usr/local/ssl/include -D_GNU_SOURCE --std=gnu99 -g -Wall -funsigned-char -lpopt
OPTS=-L/usr/local/ssl/lib ${SQLLIB} ${CCOPTS}

pull:
	git pull
	git submodule update --recursive

update:
	-git pull
	git submodule update --init --recursive --remote
	-git commit -a -m "Library update"
	-git push

notscotx:	notscotx.c ../bin/notscolib.o
	cc -fPIC -O -o $@ $< ${OPTS} ${SQLLIB}

notscorx:	notscorx.c ../bin/notscolib.o
	cc -fPIC -O -o $@ $< ${OPTS} ${SQLLIB}

../bin/notscolib.o:	notscolib.c notscolib.h
	cc -fPIC -O -DLIB -c -o $@ $< ${CCOPTS}

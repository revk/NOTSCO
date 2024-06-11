all:	../bin/notscotx ../bin/notscorx ../login/envcgi

SQLINC=$(shell mariadb_config --include)
SQLLIB=$(shell mariadb_config --libs)

CCOPTS=${SQLINC} -I. -I/usr/local/ssl/include -D_GNU_SOURCE --std=gnu99 -g -Wall -funsigned-char -lpopt
OPTS=-L/usr/local/ssl/lib ${SQLLIB} ${CCOPTS} ../AJL/ajl.o

pull:
	git pull
	git submodule update --recursive

update:
	-git pull
	git submodule update --init --recursive --remote
	-git commit -a -m "Library update"
	-git push

../bin/notscotx:	notscotx.c ../lib/notscolib.o
	cc -fPIC -O -o $@.new $< ${OPTS} ${SQLLIB}
	mv -f $@.new $@

../bin/notscorx:	notscorx.c ../lib/notscolib.o
	cc -fPIC -O -o $@.new $< ${OPTS} ${SQLLIB}
	mv -f $@.new $@

../lib/notscolib.o:	notscolib.c notscolib.h
	cc -fPIC -O -DLIB -c -o $@ $< ${CCOPTS}

../login/envcgi: ../login/envcgi.c
	make -C ../login

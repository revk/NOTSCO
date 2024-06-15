all:	../bin/notscotx ../bin/notscorx ../login/envcgi ../xmlsql/xmlsql

SQLINC=$(shell mariadb_config --include)
SQLLIB=$(shell mariadb_config --libs)

CCOPTS=${SQLINC} -I. -I/usr/local/ssl/include -I ../AJL -D_GNU_SOURCE -I../SQLlib/ --std=gnu99 -g -Wall -funsigned-char -lpopt
OPTS=-L/usr/local/ssl/lib ../lib/notscolib.o ../AJL/ajlcurl.o ../AJL/jcgi.o ../SQLlib/sqllib.o -lcurl ${SQLLIB} ${CCOPTS}

sql:
	mysqldump -d notsco > ../notsco.sql

pull:
	git pull
	git submodule update --recursive

update:
	-git pull
	git submodule update --init --recursive --remote
	-git commit -a -m "Library update" ../AJL ../SQLlib ../login ../xmlsql
	-git push

../bin/notscotx:	notscotx.c ../lib/notscolib.o ../AJL/ajlcurl.o ../SQLlib/sqllib.o ../AJL/jcgi.o
	cc -fPIC -O -o $@.new $< ${OPTS} ${SQLLIB}
	mv -f $@.new $@

../bin/notscorx:	notscorx.c ../lib/notscolib.o ../AJL/ajlcurl.o ../SQLlib/sqllib.o ../AJL/jcgi.o
	cc -fPIC -O -o $@.new $< ${OPTS} ${SQLLIB}
	mv -f $@.new $@

../lib/notscolib.o:	notscolib.c notscolib.h
	cc -fPIC -O -DLIB -c -o $@ $< ${CCOPTS}

../login/envcgi: ../login/envcgi.c
	make -C ../login

../AJL/ajlcurl.o: ../AJL/ajl.c
	make -C ../AJL

../AJL/jcgi.o: ../AJL/jcgi.c
	make -C ../AJL

../SQLlib/sqllib.o: ../SQLlib/sqllib.c
	make -C ../SQLlib

../xmlsql/xmlsql: ../xmlsql/xmlsql.c
	make -C ../xmlsql

apt:
	apt install apache2 letsencrypt python3-certbot-apache mariadb-server libpopt-dev libmariadb-dev libcurl4-openssl-dev uuid-dev

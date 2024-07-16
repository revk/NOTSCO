all:	../bin/notscotx ../bin/notscorx ../login/envcgi ../xmlsql/xmlsql ../SQLlib/sql ../SQLlib/sqlwrite ../bin/notsco ../toot/toot

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

../bin/notscotx:	notscotx.c ../lib/notscolib.o ../AJL/ajlcurl.o ../SQLlib/sqllib.o ../AJL/jcgi.o
	cc -fPIC -O -o $@.new $< ${OPTS} ${SQLLIB}
	mv -f $@.new $@

../bin/notscorx:	notscorx.c ../lib/notscolib.o ../AJL/ajlcurl.o ../SQLlib/sqllib.o ../AJL/jcgi.o
	cc -fPIC -O -o $@.new $< ${OPTS} ${SQLLIB}
	mv -f $@.new $@

../bin/notsco:	notscolib.c ../AJL/jcgi.o
	cc -fPIC -O -o $@.new $< ${CCOPTS} ../AJL/ajlcurl.o ../AJL/jcgi.o ../SQLlib/sqllib.o -lcurl ${SQLLIB}
	mv -f $@.new $@

../lib/notscolib.o:	notscolib.c notscolib.h
	cc -fPIC -O -DLIB -c -o $@ $< ${CCOPTS}

../toot/toot: ../toot/toot.c
	make -C ../toot toot

../login/envcgi: ../login/envcgi.c
	make -C ../login envcgi

../AJL/ajlcurl.o: ../AJL/ajl.c
	make -C ../AJL ajlcurl.o

../AJL/jcgi.o: ../AJL/jcgi.c
	make -C ../AJL jcgi.o

../SQLlib/sqllib.o: ../SQLlib/sqllib.c
	make -C ../SQLlib sqllib.o

../SQLlib/sql: ../SQLlib/sqllib.c
	make -C ../SQLlib sql

../SQLlib/sqlwrite: ../SQLlib/sqllib.c
	make -C ../SQLlib sqlwrite

../xmlsql/xmlsql: ../xmlsql/xmlsql.c
	make -C ../xmlsql xmlsql

apt:
	sudo apt install apache2 letsencrypt python3-certbot-apache mariadb-server libpopt-dev libmariadb-dev libcurl4-openssl-dev uuid-dev apache2-ssl-dev
	sudo a2enmod cgi ssl
	sudo mysql 'CREATE DATABASE notsco'
	sudo mysql notsco 'CREATE USER `www-data`@`localhost`'
	sudo mysql notsco 'GRANT USAGE ON *.* TO `www-data`@`localhost`'
	sudo mysql notsco 'GRANT SELECT, INSERT, UPDATE, DELETE ON `notsco`.* TO `www-data`@`localhost`'
	#sudo mysql nostco < ../nostco.sql

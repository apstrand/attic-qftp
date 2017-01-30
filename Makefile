
#CXXFLAGS = -g -Wall
CXXFLAGS = -O2 -Wall
LDFLAGS = 

BINDIR = /usr/bin
MANDIR = /usr/man/man1

VER=0.98

OBJS = qftp.o ftpconn.o net.o flist.o txtif.o conf.o hosts.o misc.o
LIBS = 

FILES = qftp.cc ftpconn.cc net.cc flist.cc txtif.cc conf.cc hosts.cc misc.cc \
	ftpconn.h net.h flist.h txtif.h conf.h hosts.h misc.h \
	qftp README README.html ChangeLog Makefile TODO qftp.man qftp.lsm qftprc

qftp:	${OBJS}
	g++ -s -o $@ ${OBJS} ${LIBS}

clean:
	rm -rf dist *.o qftp *~ *.bak TAGS README

install: qftp
	install -m 755 qftp ${BINDIR}
	install -m 644 qftp.man ${MANDIR}/qftp.1

README:	README.html
	sed 's/<[^>]*>//g' < README.html >README

dist:	qftp README ${FILES}
	rm -rf qftp-${VER}
	rm -rf dist
	mkdir qftp-${VER}
	mkdir dist
	cp ${FILES} qftp-${VER}/
	tar zcf dist/qftp-${VER}.tar.gz qftp-${VER}/
	gzip -c qftp > dist/qftp.gz
	cp README dist/README
	rm -rf qftp-${VER}


conf.o: conf.cc conf.h
flist.o: flist.cc flist.h
ftpconn.o: ftpconn.cc net.h ftpconn.h flist.h
hosts.o: hosts.cc hosts.h conf.h
misc.o: misc.cc
net.o: net.cc net.h
qftp.o: qftp.cc ftpconn.h flist.h net.h txtif.h conf.h hosts.h
txtif.o: txtif.cc conf.h txtif.h flist.h ftpconn.h net.h hosts.h

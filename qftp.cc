/*
 *  qftp
 *  Copyright (C) 1997,1998 Peter Strand
 *  Distributed under the GNU Pulic Licence
 */

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "ftpconn.h"
#include "txtif.h"
#include "conf.h"

#include "hosts.h"

int main(int argc, char *argv[])
{
	TxtIF *txtif;
	Conf gconf;
	Hosts hosts;
	int fd;
	char tmp[200], *p;

	p = getenv("HOME");
	if (p) {
		strcat(tmp, p);
		strcat(tmp, "/.qftprc");	
		fd = open(tmp, O_RDONLY);
		hosts.Parse(fd, gconf);
		close(fd);
	}
	gconf.Parse(gconf, argc, argv, "abcefhlnqzy:u:ABCEFLNQZ");
	txtif = new TxtIF(new FtpConn, gconf, &hosts);
	if (argv[1] && argv[1][0] == '-' && argv[1][1] == 'h') {
		txtif->Khelp(-1, argv);
		exit(0);
	}
	if (gconf.Args())
		txtif->Open(argc, argv);
	txtif->go();
	return 0;
}

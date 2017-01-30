/*
 *  qftp
 *  Copyright (C) 1997,1998 Peter Strand
 *  Distributed under the GNU Pulic Licence
 */


#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>
#include <utime.h>

#ifndef TIOCWINSZ
#include <sys/ioctl.h>
#endif

#include "conf.h"
#include "txtif.h"
#include "flist.h"
#include "ftpconn.h"
#include "hosts.h"
#include "misc.h"


#if __GLIBC__ < 2
typedef __sighandler_t sig_t;
#endif

#define NCPROMPT "qftp> "
#define PROMPTF "%s:%s%%%% "


TxtIF::TxtIF()
{
}

TxtIF::TxtIF(FtpConn *nftp, Conf &nconf, Hosts *h)
{
	char *p, tmp[256];
	hosts = h;
	ftp = nftp;
	gconf.opts = nconf.opts;
	opts.force = opts.cont = opts.longl = opts.anon = 0;
	strcpy(anonuser, "ftp");
	if (gethostname(tmp, 256) == -1)
		strcpy(tmp, "foo.bar");
	sprintf(anonpass, "%s@%s", (p = getenv("LOGNAME"))?p:"unknown", tmp);
	strcpy(prompt, NCPROMPT);
}

void TxtIF::go()
{
	char buf[512];
	char *p, *f[1000];
	int i, n;
	do {

		printf(prompt);
		fflush(stdout);
		if (!fgets(buf, 512, stdin))
			exit(0);
		buf[strlen(buf) - 1] = 0;
		p = buf;

		while (isspace(*p))
			p++;
		i = lookup(p);
		n = procargs(buf, f);
		switch (i) {
		case 0: Open(n, f); break;
		case 1: Cd(n, f); break;
		case 2: List(n, f); break;
		case 3: LongList(n, f); break;
		case 4: Close(n, f); break;
		case 5: Quote(n, f); break;
		case 6: Get(n, f); break;
		case 7: Put(n, f); break;
		case 8: LCd(n, f); break;
		case 9: Help(n, f); break;
		case 10: View(n, f); break;
		case 11: Set(n, f); break;
		case 12: Khelp(n, f); break;
		case 13: Mark(n, f); break;
		case 14: GetMarked(n, f); break;
		case 15: ListMarked(n, f); break;
		default: break;
		}
		
	} while(1);
}

void TrStat(int n, char *s = NULL)
{
	static int count;
	if (n == -1) {
		count = 0;
		printf("Transferring %s:           ", s);
	} else {
		count += n;
		printf("\b\b\b\b\b\b\b\b\b\b%10u", count);
	}
	fflush(stdout);

}

int TxtIF::Set(int n, char *a[])
{
	if (n > 1) {
		Conf conf(gconf, n, a, "bzfclas:mnrqu:BZFCLASMNRQU");
		gconf.opts = conf.opts;
	} else {
		gconf.print();
	}
	return 0;
}

int TxtIF::GetMarked(int n, char *s[])
{
	Entry e;
	
	Conf conf(gconf, n, s, "bcfrBCFR");
	markList.Reset();
	while (markList.GetNext(e)) {
		ftp->Cd(e.wd);
		Get(conf, e.name);
		free(e.wd);
	}
	markList.Flush();
	return 0;
}

int TxtIF::ListMarked(int n, char *s[])
{
	Entry e;
	
	markList.Reset();
	while (markList.GetNext(e)) {
		printf("%s / %s\n", e.wd, e.name);
	}
	markList.Reset();
	return 0;
}

int TxtIF::Mark(int n, char *s[])
{
	Entry e;
	char *p;
	Conf conf(gconf, n, s, "");
	DirList dl;
	
	while ((p = conf.GetNext())) {
#ifdef DEBUG
		printf("-%s-\n", p);
#endif
		Glob(p, dl);
		dl.Reset();
		while (dl.GetNext(e)) {
#ifdef DEBUG
		printf("--%s-\n", e.name);
#endif
			e.wd = strdup(ftp->GetCwd());
			markList.Add(e);
		}
	}
	return 0;
}

int TxtIF::View(int n, char *a[])
{
	char *p, *pager, pg[256], *z;
	FILE *fp;
	sig_t sigh;
	Conf conf(gconf, n, a, "zZ");
	DirList dl;
	Entry e;
	
	if (!(pager = getenv("PAGER")))
		pager = PAGER;
	
	while ((p = conf.GetNext())) {
		Glob(p, dl, 1);
		dl.Reset();
		while (dl.GetNext(e)) {
			sigh = signal(SIGPIPE, SIG_IGN);
			pg[0] = 0;
			z = strrchr(e.name, '.') + 1;
			if (conf.opts.proc && (!strcmp(z, "gz") || !strcmp(z, "Z")))
				strcat(pg, "gzip -dc|");
			strcat(pg, pager);
			fp = popen(pg, "w");
			ftp->Get(e.name, fileno(fp));
			pclose(fp);
			signal(SIGPIPE, sigh);
		}
	}
	return 0;
}

int TxtIF::LCd(int n, char *a[])
{
	char buf[512];
	chdir(a[1]);
	printf("Local directory now: %s\n", getcwd(buf, 512));
	return 0;
}

int TxtIF::Help(int n, char *a[])
{
	ftp->Cmd("HELP", a[1]);
	printf(ftp->GetMsg());
	return 0;
}

void TxtIF::Glob(char *s, DirList &dl, int glob = 0)
{
	Entry e;
	int i, j;
	
	if (*s == '#') {
		s++;
		while (*s) {
			i = atoi(s);
			curList.GetAt(e, i);
			dl.Add(e);
			while (isdigit(*++s));
			if (*s == '-') {
				j = atoi(s + 1);
				while (isdigit(*++s));
				while(++i <= j) {
					curList.GetAt(e, i);
					dl.Add(e);
				}
			}
			if (*s == ',') {
				s++;
			}

		}
		return;
	}
	e.name = s;
	dl.Add(e);
}

int TxtIF::Get(int n, char *a[])
{
	char *p;
	Conf conf(gconf, n, a, "bcfrBCFR");
	DirList dl;
	Entry e;
	
	while ((p = conf.GetNext())) {
		Glob(p, dl, 1);
		dl.Reset();
		while (dl.GetNext(e))
			Get(conf, e.name);
		
	}
	return 0;
}

int TxtIF::Get(Conf &conf, char *p, FtpConn *nftp = NULL)
{
	FtpConn *cftp = (nftp)?nftp:ftp;
	int fd;
	char tmp[100], *t1, *t2;
	time_t mt;
	struct utimbuf ut;
	
	if (conf.opts.bg) {
		if (!fork()) {
			FtpConn nftp;
			nftp.Connect(ftp->GetHost());
			nftp.Login(user, pass);
			nftp.Quote("type i");
			nftp.Cd(ftp->GetCwd());
			conf.opts.bg = 0;
			conf.opts.quiet = 1;
			Get(conf, p, &nftp);
			exit(0);
		} else
			return 0;
	}
	if (conf.opts.rec) {
		if (cftp->Cd(p) < 300) {
			mkdir(p, 0777);
			chdir(p);
			DirList dl;
			Entry e;
			cftp->List(dl);
			dl.Reset();
			while (dl.GetNext(e))
				Get(conf, e.name, cftp);
			chdir("..");
			cftp->Cd("..");
		}
	}
	t1 = p;
	while ((t2 = strchr(t1, '/'))) {
		strncpy(tmp, t1, t2 - t1);
		tmp[t2 - t1] = 0;
		mkdir(tmp, 0777);
		t1 = t2 + 1;
	}
	if (conf.opts.cont) {
		struct stat st;
		if (stat(p, &st))
			st.st_size = 0;
		fd = open(p, O_CREAT | O_APPEND | O_WRONLY, 0666);
		if (st.st_size) {
			sprintf(tmp, "%lu", st.st_size);
			ftp->Cmd("REST", tmp);
		}
	} else {
		if (conf.opts.force)
			fd = open(p, O_CREAT | O_WRONLY, 0666);
		else {
			if ((fd = open(p, O_CREAT | O_EXCL | O_WRONLY, 0666)) == -1) {
				perror(p);
				return 1;
			}
		}
	}
	cftp->Get(p, fd, (conf.opts.quiet)?NULL:TrStat);
	if (!conf.opts.quiet)
		putchar('\n');
	if (conf.opts.mod) {
		ftp->MTime(p, mt);
		ut.actime = ut.modtime = mt;
		utime(p, &ut);
	}
	
	close(fd);
	return 0;
}

int TxtIF::Put(int n, char *a[])
{

	char *p;
	int fd;
	Conf conf(gconf, n, a, "cf");
	Entry e;
	DirList dl;
	
	while ((p = conf.GetNext())) {
		Glob(p, dl);
		dl.Reset();
		while (dl.GetNext(e)) {
			fd = open(e.name, O_RDONLY);
			ftp->Put(e.name, fd, TrStat);
			putchar('\n');
			close(fd);
		}
	}
	return 0;
}

int TxtIF::Quote(int n, char *a[])
{
	char tmp[200];
	int i;
	tmp[0] = 0;
	i = 1;
	if (i < n) {
		do {
			strcat(tmp, a[i++]);
			if (i < n)
				strcat(tmp, " ");
		} while  (i < n);
		ftp->Quote(tmp);
		printf(ftp->GetMsg());
	}
	return 0;
}


int TxtIF::Open(int n, char *a[])
{
	char *p1, *p2;
  	char *p, conn = 0, *wd = NULL, *t1, *t2, *f[3], tmp[512];
	int i;
	HostEnt *h;
	
	Conf conf(gconf, n, a, "y:anu:");
	pass[0] = 0;

	if (!(p = conf.GetNext()))
		return 1;
	markList.Flush();
	user[0] = pass[0] = 0;
	
	if ((p1 = strchr(conf.opts.user, ':'))) {
		strncpy(user, conf.opts.user, p1 - conf.opts.user);
		user[p1 - conf.opts.user] = 0;
		p1++;
		if (*p1)
			strcpy(pass, p1);
	} else if (!*user)
		strcpy(user, conf.opts.user);

	f[0] = "g";
	f[1] = NULL;
	f[2] = NULL;
	if ((t1 = strchr(p, ':'))) {
		*t1++ = 0;
		strcpy(tmp, t1);
		if ((t2 = strrchr(t1, '/'))) {
			wd = tmp;
			tmp[(t2 - t1)?t2 - t1:1] = 0;
			*t2++ = 0;
			if (*t2)
				f[1] = t2;
		} else
			f[1] = t1;
	} else if ((t2 = strchr(p, '/'))) {
		strcpy(tmp, t2);
		*t2++ = 0;
		wd = tmp;
		
		t1 = strrchr(tmp, '/');
		if (t1[1])
			f[1] = &t2[t1 - tmp];
		wd[(t1 - tmp)?t1 - tmp:1] = 0;
		
	} else if (conf.Args() > 1) {
		wd = conf.GetNext();
		if (conf.Args() > 2)
			f[1] = conf.GetNext();
	}
#ifdef DEBUG
	printf("wd: -%s- f: -%s- p:-%s-\n", wd, f[1], p);
#endif
	h = hosts->Find(p);
	if (h) {
		p = h->Name();
		if (!wd)
			wd = h->Wd();
		if (!conf.opts.noinf) {
			strcpy(user, h->User());
			strcpy(pass, h->Password());
		}
	}

	ftp->Close();
	


	while (conf.opts.retry-- + 1 && !conn) {
		if (ftp->Connect(p)) {
			p1 = (char *)ftp->GetError();
			if (!p1)
				p1 = ftp->GetMsg();
			fprintf(stderr, "%s\n", p1);
			sleep(conf.opts.retrsl);
			ftp->Close();
			continue;
		}
		printf(ftp->GetMsg());
		if (conf.opts.anon) {
			strcpy(user, anonuser);
			strcpy(pass, anonpass);
		} else {
			if (!*user || conf.opts.noinf) {
				printf("%s(%s): ", ftp->GetHost(), (p1 = getenv("LOGNAME"))?p1:"ftp");
				fflush(stdout);
				if (!fgets(user, 512, stdin) || user[0] == '\n')
					strcpy(user, p1);
			}
			if (!*pass || conf.opts.noinf) {
				p2 = getpass("password: ");
				strcpy(pass, p2);
			}
		}
		if (ftp->Login(user, pass)) {
			p1 = (char *)ftp->GetError();
			if (!p1)
				p1  = ftp->GetMsg();
			fprintf(stderr, "%s\n", p1);
			sleep(conf.opts.retrsl);
			continue;
		}
		conn = 1;
	}
	if (!conn)
		return 1;
	printf(ftp->GetMsg());
	ftp->Quote("TYPE I");
	i = 250;
	if (wd)
		i = ftp->Cd(wd);
	if (f[1] && i == 250) {
		Get(1, f);
		exit(0);
	}
	sprintf(prompt, PROMPTF, ftp->GetHost(), ftp->GetCwd());
	return 0;
}

int TxtIF::Close(int n, char *a[])
{
	ftp->Close();
	sprintf(prompt, NCPROMPT);
	return 0;
}


int TxtIF::List(int n, char *a[])
{
	char *p;
	int i, j, pc, pl, lines, cols, width;
	DirList dl;
	Entry e;
	Conf conf(gconf, n, a, "lrstqLRSTQ");
	struct winsize ws;
	
	if (conf.opts.longl)
		i = ftp->LongList(dl, conf.GetNext());
	else
		i = ftp->List(dl, conf.GetNext());
	dl.Sort(conf.opts.size?2:conf.opts.time?1:0,conf.opts.rev);
	if (i == -1)
		return 1;
	dl.Reset();
	i = 0;
	curList.Flush();
	if (ioctl(STDIN_FILENO, TIOCGWINSZ, (char *) &ws) < 0) {
		perror("TIOCGWINSZ");
		p = getenv("LINES");
		lines = (p?atoi(p):25) - 1;
		p = getenv("COLUMNS");
		cols = p?atoi(p):80;
	} else {
		lines = ws.ws_row;
		cols = ws.ws_col;
	}
	if (conf.opts.longl) {
		while (dl.GetNext(e)) {
			printf("%3d %10s %-8s %-8s %s %8lu %s\n",
			       i, e.perms, e.user, e.group, e.date, e.size, e.name);
			curList.Add(e);
			i++;
			if (!conf.opts.quiet && !(i % lines))
				getchar();
		}
	} else if (conf.opts.quiet || !lines) {
		while (dl.GetNext(e)) {
			printf("%3d %s\n", i, e.name);
			curList.Add(e);
			i++;
		}		
	} else {
		i = dl.GetWidest() + 4;
		width = (int)(cols / floor(cols / i));
		i = pl = 0;
		while (dl.GetAt(e, i)) {
			pc = width;
			do {
				printf("%3d %s", i++, e.name);
				curList.Add(e);
				for (j = strlen(e.name) + 4;j < width;j++)
					putchar(' ');
				pc += width;
			} while(pc <= cols && dl.GetAt(e, i));
			putchar('\n');
			pl++;
			if (!(pl % lines))
				getchar();
		}
	}
	return 0;
}

int TxtIF::LongList(int n, char *a[])
{
	a[n] = "-l";
	return List(n + 1, a);
}

int TxtIF::Cd(int n, char *a[])
{
	if ((unsigned)ftp->Cd(a[1]?a[1]:a[0]) < 300)
		sprintf(prompt, PROMPTF, ftp->GetHost(), ftp->GetCwd());
	return 0;
}

int TxtIF::lookup(char *s)
{
	int i;
	char *p;
	if (isalpha(s[1])) {
		if ((p = strchr(s, ' ')))
			i = p - s;
		else
			i = strlen(s);
		if (i <= 4 && !strncmp(s, "open", i))
			return 0;
		if (i <= 2 && !strncmp(s, "cd", i))
			return 1;
		if (i <= 2 && !strncmp(s, "ls", i))
			return 2;
		if (i <= 3 && !strncmp(s, "dir", i))
			return 3;
		if (i <= 5 && !strncmp(s, "close", i))
			return 4;
		if (i <= 5 && !strncmp(s, "quote", i))
			return 5;
		if (i <= 3 && !strncmp(s, "get", i))
			return 6;
		if (i <= 3 && !strncmp(s, "put", i))
			return 7;
		if (i <= 3 && !strncmp(s, "lcd", i))
			return 8;
		if (i <= 4 && !strncmp(s, "help", i))
			return 9;
		if (i <= 4 && !strncmp(s, "view", i))
			return 10;
		if (i <= 3 && !strncmp(s, "set", i))
			return 11;
		if (i <= 4 && !strncmp(s, "keys", i))
			return 12;
		if (i <= 4 && !strncmp(s, "mark", i))
			return 13;
		if (i <= 9 && !strncmp(s, "getmarked", i))
			return 14;
		if (i <= 10 && !strncmp(s, "listmarked", i))
			return 15;
		return 1;
	} else {
		switch(*s) {
		case 'o': return 0;
		case 'c': return 1;
		case 'l': return 2;
		case 'd': return 3;
		case 'x': return 4;
		case 'Q': return 5;
		case 'g': return 6;
		case 'p': return 7;
		case 'C': return 8;
		case 'h': return 9;
		case 'v': return 10;
		case 's': return 11;
		case '?': return 12;
		case 'm': return 13;
		case 'G': return 14;
		case 'L': return 15;
		default: return 1;
		}
	}
	return -1;
}


int TxtIF::Khelp(int n, char *a[]) 
{
	if (n >= 0) {
		printf("Commands:\n");
		printf("  ?  keys\n");
		printf("  C  lcd\n");
		printf("  G  getmarked [-bBcCfFeEmM]\n");
		printf("  L  listmarked\n");
		printf("  Q  quote \n");
		printf("  c  cd\n");
		printf("  d  dir [-rRtTsS]\n");
		printf("  g  get [-bBcCfFeEmM]\n");
		printf("  h  help (from remote server)\n");
		printf("  l  ls [-lLrRtTsS]\n");
		printf("  m  mark\n");
		printf("  o  open [-aAnNYU] [-y <retries>:<sleep>] [-u <user>:<password>]\n");
		printf("  p  put\n");
		printf("  s  set [<option>]\n");
		printf("  v  view [-zZ] \n");
		printf("  x  close \n");
	} else {
		printf("qftp version 0.98 by Peter Strand <pst@2.sbbs.se>\n\n");
	}
	printf("Options:\n");
	printf("  a  Anonymous login\n");
	printf("  y  Number of retries and time to sleep between\n");
	printf("  n  Don't use user info from rc file\n");
	printf("  u  Default user and password\n");
	printf("  e  Recursive get\n");
	printf("  q  Quiet operation\n");
	if (n >= 0) {
		printf("  b  Background downloading\n");
		printf("  z  Decompress files before viewing\n");
		printf("  f  Force overwrite\n");
		printf("  c  Continuation (reget)\n");
		printf("  m  Preserve modification time\n");
		printf("  l  Long listing\n");
		printf("  s  sort by size\n");
		printf("  r  reverse sort\n");
		printf("  t  sort by time\n");
	}
	if (n < 0) {
		printf("\nPlease report bugs, improvement ideas, strange behaviour and\n\
your opinion about this program to <pst@2.sbbs.se>\n");
	}
	return 0;
}


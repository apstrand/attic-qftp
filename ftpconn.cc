/*
 *  qftp
 *  Copyright (C) 1997,1998 Peter Strand
 *  Distributed under the GNU Pulic Licence
 */

#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>

#include "net.h"
#include "ftpconn.h"
#include "misc.h"


FtpConn::FtpConn()
{
	ctrl = new Socket;
	data = NULL;
	cwdch = 1;
	conn = 0;
}

FtpConn::~FtpConn()
{
}

int FtpConn::Connect(char *host)
{
	Host h(host);
	if (!ctrl)
		ctrl = new Socket;
	cwdch = 1;
	if (ctrl->Connect(h))
		return -1;
	if (!GetReply())
		return 1;
	conn = 1;
	return 0;
}

void FtpConn::Close()
{
	if (conn) {
		Cmd("QUIT");
		GetReply();
	}
	conn = 0;
	if (ctrl) {
		ctrl->Close();
		ctrl = NULL;
	}
	if (data) {
		data->Close();
		data = NULL;
	}
}

int FtpConn::Cmd(const char *cm, const char *arg)
{
	char buf[1024];
	
	if (arg && *arg)
		sprintf(buf, "%s %s", cm, arg);
	else
		sprintf(buf, "%s", cm);
	if (ctrl->Writeln(buf))
		return -1;
	return GetReply();
}

int FtpConn::GetReply()
{
	int i;
	
	if (ctrl->Readln(retmsg) == -1)
		return -1;
	retval = atoi(retmsg);
	i = 0;
	if (retmsg[3] == '-') {
		do {
			i += strlen(&retmsg[i]);
			if (ctrl->Readln(&retmsg[i]) == -1)
				return -1;
		} while ((atoi(&retmsg[i]) == retval)?(retmsg[i + 3] == '-'):1);
	}
	i += strlen(retmsg);
	return retval;
}

int FtpConn::Get(char *s, int fd, void (*cbf)(int,char *t))
{
	Socket *ns;
	char buf[4096];
	int n;
	
	data = new Socket(ctrl);
	if (data->Listen() == -1)
		return -1;
	if ((unsigned)Port() >= 400)
		return -1;
	
	if ((unsigned)Cmd("RETR", s) >= 400)
		return -1;
	
	if (!(ns = data->Accept()))
		return -1;
	if (cbf)
		cbf(-1, s);
	while ((n = ns->Read(buf, 4096))) {
		write(fd, buf, n);
		if (cbf, NULL)
			cbf(n, NULL);
	}
	delete ns;
	delete data;
	return (GetReply() == -1)?-1:0;
}


int FtpConn::Put(char *s, int fd, void (*cbf)(int,char *t))
{
	Socket *ns;
	char buf[4096];
	int n;
	
	data = new Socket(ctrl);
	if (data->Listen() == -1)
		return -1;
	if ((unsigned)Port() >= 400)
		return -1;
	
	if ((unsigned)Cmd("STOR", s) >= 400)
		return -1;

	if (!(ns = data->Accept()))
		return -1;
	if (cbf)
		cbf(-1,s);
	while ((n = read(fd, buf, 4096)) > 0) {
		ns->Write(buf, n);
		if (cbf)
			cbf(n, NULL);
	}
	delete ns;
	delete data;
	return (GetReply() == -1)?-1:0;
}


int FtpConn::Quote(char *s)
{
	return Cmd(s, NULL);
}

int FtpConn::Cd(char *s)
{
	cwdch = 1;
	return Cmd("CWD", s);
}

int FtpConn::Size(char *s)
{
	return Cmd("SIZE", s);
}

int FtpConn::MTime(char *s, time_t &t)
{
	struct tm tmb;

	if (Cmd("MDTM", s) > 300)
		return -1;
				
	tmb.tm_year = (retmsg[6] - 0x30) * 10 + retmsg[7] - 0x30;
	tmb.tm_mon = (retmsg[8] - 0x30) * 10 + retmsg[9] - 0x31;
	tmb.tm_mday = (retmsg[10] - 0x30) * 10 + retmsg[11] - 0x30;
	tmb.tm_hour = (retmsg[12] - 0x30) * 10 + retmsg[13] - 0x31;
	tmb.tm_min = (retmsg[14] - 0x30) * 10 + retmsg[15] - 0x30;
	tmb.tm_sec = (retmsg[16] - 0x30) * 10 + retmsg[17] - 0x30;
	t = mktime(&tmb);
	return 0;
}

int FtpConn::Login(char *user, char *pass)
{
	if ((unsigned)Cmd("USER", user) >= 400)
		return -1;
	if ((unsigned)Cmd("PASS", (*pass)?pass:"") >= 400)
		return -1;
	return 0;
}

DirList *FtpConn::clookup(char *s)
{
	int i, n;

	n = cache.size();
	for (i = 0;i < n;i++)
		if (!strcmp(cache[i]->path, s))
			return cache[i];
	return NULL;
}

void FtpConn::cset(char *s, DirList *dl)
{
	int i, n;

	n = cache.size();
	for (i = 0;i < n;i++)
		if (!strcmp(cache[i]->path, s))
			break;
	if (n == i) {
		dl->path = strdup(s);
		cache.push_back(dl);
	} else {
		free(cache[i]->path);
		cache[i] = dl;
		cache[i]->path = strdup(s);
	}
}

int FtpConn::List(DirList &dl, char *s)
{
	Entry e;
	int i;
	char *p, path[256];

	DirList *t;
	
	p = split(GetCwd(), s, path);
	
	if (!(t = clookup(path))) {
		t = new DirList;
		list(t, path);
		cset(path, t);
	}
	t->Reset();
	i = 0;
	while (!i && t->GetNext(e))
		if (!strcmp(e.name, p))
			i++;
	if (i && e.type == 'd') {
		strcat(path, "/");
		strcat(path, p);
		if (!(t = clookup(path))) {
			t = new DirList;
			list(t, path);
			cset(path, t);
		}
		p = "";
	}
	t = clookup(path);
	dl = *t;
	return 0;
}


int FtpConn::list(DirList *dl, char *path)
{
	Socket *ns;
	char buf[500];
	int i;
	Entry e;
	data = new Socket(ctrl);
	if (data->Listen() == -1)
		return -1;
	if ((unsigned)Port() >= 400)
		return -1;
	if ((unsigned)Cmd("NLST -Fa", path) >= 400)
		return -1;		
	if (!(ns = data->Accept()))
		return -1;
	dl->Flush();
	dl->isused = 1;
	if (!ns->Readln(buf))
		return -1;
	if (!ns->Readln(buf))
		return -1;
	while ((i = ns->Readln(buf))) {
		if (i == -1)
			return -1;
		i = strlen(buf) - 1;
		if (buf[i] == '\n' || buf[i] == '\r') {
			buf[i] = 0;
			i--;
		}
		if (buf[i] == '\n' || buf[i] == '\r') {
			buf[i] = 0;
			i--;
		}
		e.type = '-';
		if (buf[i] == '/' || buf[i] == '*' || buf[i] == '@') {
			if (buf[i] == '/')
				e.type = 'd';
			else if (buf[i] == '@')
				e.type = 'l';
			buf[i] = 0;
		}
		e.name = strdup(buf);
		e.perm = -1;
		e.date = NULL;
		e.user = NULL;
		e.group = NULL;
		e.size = -1;
		dl->Add(e);
	}
	GetReply();
	delete ns;
	delete data;
	data = NULL;
	return 0;
}

int FtpConn::LongList(DirList &dl, char *s)
{
	Entry e;
	int i;
	char *p, path[256];

	DirList *t;
	
	p = split(GetCwd(), s, path);
	
	if (!(t = clookup(path)) || !t->islong) {
		t = new DirList;
		longList(t, path);
		cset(path, t);
	}
	t->Reset();
	i = 0;
	while (!i && t->GetNext(e))
		if (!strcmp(e.name, p))
			i++;
	if (i && e.type == 'd') {
		strcat(path, "/");
		strcat(path, p);
		if (!(t = clookup(path)) || !t->islong) {
			t = new DirList;
			longList(t, path);
			cset(path, t);
		}
		p = "";
	}
	t = clookup(path);
	dl = *t;
	return 0;
}

int FtpConn::longList(DirList *dl, char *s)
{
	Socket *ns;
	char buf[500], *p1, *p2;
	Entry e;
	int i;

	data = new Socket(ctrl);
	if (data->Listen() == -1)
		return -1;
	if ((unsigned)Port() >= 400)
		return -1;
	if ((unsigned)Cmd("LIST -a", s) >= 400)
		return -1;		
	if (!(ns = data->Accept()))
		return -1;
	if (!ns->Readln(buf))
		return -1;
	if (!ns->Readln(buf))
		return -1;
	if (!ns->Readln(buf))
		return -1;
	dl->Flush();
	while ((i = ns->Readln(buf))) {
		if (i == -1)
			return -1;
		i = strlen(buf) - 1;
		if (buf[i] == '\n' || buf[i] == '\r') {
			buf[i] = 0;
			i--;
		}
		if (buf[i] == '\n' || buf[i] == '\r') {
			buf[i] = 0;
			i--;
		}
		e.type = buf[0];
		e.perm = 0;
		if (buf[1] == 'r')
			e.perm |= 0400;
		if (buf[2] == 'w')
			e.perm |= 0200;
		if (buf[3] == 'x')
			e.perm |= 0100;
		if (buf[4] == 'r')
			e.perm |= 0040;
		if (buf[5] == 'w')
			e.perm |= 0020;
		if (buf[6] == 'x')
			e.perm |= 0010;
		if (buf[7] == 'r')
			e.perm |= 0004;
		if (buf[8] == 'w')
			e.perm |= 0002;
		if (buf[9] == 'x')
			e.perm |= 0001;
		if (buf[3] == 's')
			e.perm |= 04000;
		if (buf[6] == 's')
			e.perm |= 02000;
		if (buf[9] == 's')
			e.perm |= 01000;
		p1 = &buf[10];
		
		while (isspace(*p1)) p1++; while (!isspace(*p1)) p1++;
		buf[10] = 0;
		e.perms = strdup(buf);
		while (isspace(*p1)) p1++; p2 = p1; while (!isspace(*p2)) p2++; *p2 = 0;
		e.user = strdup(p1);
		p1 = p2 + 1;
		
		while (isspace(*p1)) p1++; p2 = p1; while (!isspace(*p2)) p2++; *p2 = 0;
		e.group = strdup(p1);
		p1 = p2 + 1;
		
		while (isspace(*p1)) p1++; p2 = p1; while (!isspace(*p2)) p2++; *p2 = 0;
		e.size = atol(p1);
		p1 = p2 + 1;
		
		while (isspace(*p1)) p1++;
		p2 = p1;
		while (!isspace(*p2)) p2++; while (isspace(*p2)) p2++;
		while (!isspace(*p2)) p2++; while (isspace(*p2)) p2++;
		while (!isspace(*p2)) p2++; while (isspace(*p2)) p2++;
		*--p2 = 0;
		e.date = strdup(p1);
		p1 = p2 + 1;
		while (isspace(*p1))
			p1++;
		p2 = p1;
		while (*p2 && !isspace(*p2))
			p2++;
		*p2 = 0;
		e.name = strdup(p1);
		dl->Add(e);
	}
	dl->islong = 1;
	delete ns;
	delete data;
	data = NULL;
	return (GetReply() == -1)?-1:0;
}


char *FtpConn::GetHost()
{
	return ctrl->GetRHost();
}

char *FtpConn::GetMsg()
{
	return retmsg;
}

const char *FtpConn::GetError()
{
	return ctrl->GetError();
}

char *FtpConn::GetCwd()
{
	char *p;
	int i;
	if (cwdch) {
		if ((unsigned)Cmd("PWD") >= 400)
			return "";
		p = strchr(retmsg, '"') + 1;
		i = 0;
		while ((cwd[i++] = *p++) != '"');
		cwd[i - 1] = 0;
	}
	cwdch = 0;
	return cwd;
}

int FtpConn::Port()
{
	char buf[500];
	long badr, bp;
	badr = ntohl(data->GetLAddr());
	bp = ntohs(data->GetLPort());
	sprintf(buf, "%d,%d,%d,%d,%d,%d", (int) (badr >> 24) & 0xff,
		(int) (badr >> 16) & 0xff, (int) (badr >> 8) & 0xff,
		(int) badr & 0xff, (int) (bp >> 8) & 0xff, (int) bp & 0xff);
	return Cmd("PORT", buf);
}


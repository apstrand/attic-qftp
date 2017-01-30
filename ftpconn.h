/*
 *  qftp
 *  Copyright (C) 1997,1998 Peter Strand
 *  Distributed under the GNU Pulic Licence
 */

#include <time.h>
#include "flist.h"
#include "net.h"

#ifndef _FTPCONN_H_
#define _FTPCONN_H_


class FtpConn {
	struct dlcmp {
		bool operator()(const char *s1, const char *s2) const {
#ifdef DEBUG
			printf("dlcmp: -%s-%s-\n", s1, s2);
#endif
			return (strcmp(s1, s2) < 0);
		}
	};
	vector<DirList *> cache;
	
	Socket *ctrl, *data;
	char cwd[256];
	int cwdch, conn;
	
	int Port();
	int GetReply();
	//	char *split(char *s, char *p);
	char retmsg[16000];
	int retval;
	
	DirList *clookup(char *);
	void cset(char *, DirList *dl);
	int list(DirList *dl, char *path);
	int longList(DirList *dl, char *s = NULL);
 public:
	
	FtpConn();
	~FtpConn();
	
	char *GetHost();
	char *GetCwd();
	char *GetMsg();
	const char *GetError();
	
	int Connect(char *host);
	void Close();
	int Login(char *user, char *pass);
	int Cmd(char *cmd, char *arg = NULL);
	int Quote(char *s);
	int List(DirList &fl, char *s = NULL);
	int LongList(DirList &fl, char *s = NULL);
	int Cd(char *s = NULL);
	int MTime(char *s, time_t &t);
	int Size(char *s = NULL);
	int Get(char *s, int fd, void (*)(int,char*) = NULL);
	int Put(char *s, int fd, void (*)(int,char*) = NULL);

};





#endif




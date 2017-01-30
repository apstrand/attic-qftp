/*
 *  qftp
 *  Copyright (C) 1997,1998 Peter Strand
 *  Distributed under the GNU Pulic Licence
 */


#ifndef _TXTIF_H_
#define _TXTIF_H_

#include "conf.h"
#include "flist.h"
#include "ftpconn.h"
#include "hosts.h"

#define PAGER "less"

struct options {
	char bg;	// b
	char proc;	// z
	char force;	// f
	char cont;	// c
	char longl;	// l
	char anon;	// a
	char retry;	// r
	char retrsl;	// s
	char noinf;	// n
	char rec;	// r
};

class TxtIF {
	FtpConn *ftp;
	Conf gconf;
	Hosts *hosts;
	char user[128], pass[128], anonuser[128], anonpass[128];
	struct options opts;
	char prompt[1024];
	DirList curList;
	DirList markList;
	
	int Get(Conf &conf, char *p, FtpConn *nftp = NULL);
	int Expand(int &n, char *a[]);
	void Glob(char *, DirList&, int glob = 0);
 public:
	TxtIF();
	TxtIF(FtpConn *ftp, Conf &nconf, Hosts *hosts);
	void go();
	int lookup(char *s);
	int Khelp(int n, char *a[]);
	int Open(int n, char *a[]);
	int Close(int n, char *a[]);
	int Mark(int n, char *a[]);
	int GetMarked(int n, char *a[]);
	int ListMarked(int n, char *a[]);
	int List(int n, char *a[]);
	int LongList(int n, char *a[]);
	int Cd(int n, char *a[]);
	int Put(int n, char *a[]);
	int Get(int n, char *a[]);
	int Quote(int n, char *a[]);
	int Help(int n, char *a[]);
	int LCd(int n, char *a[]);
	int Set(int n, char *a[]);
	int View(int n, char *a[]);
};


#endif





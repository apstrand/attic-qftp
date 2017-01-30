/*
 *  qftp
 *  Copyright (C) 1997,1998 Peter Strand
 *  Distributed under the GNU Pulic Licence
 */

#ifndef _HOSTS_H_
#define _HOSTS_H_


#include "conf.h"

#include <vector>

class HostEnt {
	char *alias;
	char *name;
	char *user;
	char *password;
	char *wd;
	
	void mkstr(char **, char *);
 public:
	HostEnt();
	HostEnt(HostEnt &h);
	~HostEnt();
	void Set(char *a, char *n, char *u, char *p, char *w);
	char *Alias(char *s = NULL);
	char *Name(char *s = NULL);
	char *User(char *s = NULL);
	char *Password(char *s = NULL);
	char *Wd(char *s = NULL);
};

class Hosts {
	std::vector<HostEnt *> hosts;

	char *parseline(char *buf, char *f[]);
 public:
	HostEnt *Find(char *s);
	void Add(HostEnt &he);
	void Del(char *s);
	void Parse(int fd, Conf &conf);
};

#endif

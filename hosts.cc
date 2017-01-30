/*
 *  qftp
 *  Copyright (C) 1997,1998 Peter Strand
 *  Distributed under the GNU Pulic Licence
 */

#include <unistd.h>
#include <ctype.h>

#include "hosts.h"
#include "conf.h"


HostEnt *Hosts::Find(char *s)
{
	int i, n;
	n = hosts.size();
	for (i = 0; i < n; i++) {
		if (!strcmp(hosts[i]->Alias(), s))
			return hosts[i];
		if (!strcmp(hosts[i]->Name(), s))
			return hosts[i];
	}
	return NULL;
}

void Hosts::Add(HostEnt &he)
{
	HostEnt *h = new HostEnt(he);
	hosts.push_back(h);
}

void Hosts::Del(char *s)
{
	vector<HostEnt *>::iterator iter;

	iter = hosts.begin();
	while (iter < hosts.end()) {
		if (!strcmp((*iter)->Alias(), s))
			hosts.erase(iter);
		if (!strcmp((*iter)->Name(), s))
			hosts.erase(iter);
	}
	delete *iter;
}

void Hosts::Parse(int fd, Conf &conf)
{
	HostEnt hent;
	
	char buf[10000];
	char *f[10];
	char *p, *c;
	long s;

	s = read(fd, buf, 10000);
	p = buf;
	do {
		p = parseline(p, f);
		if (f[0] && f[1] && f[2] && f[3] && f[4]) {
			hent.Set(f[0], f[1], f[2], f[3], f[4]);
			Add(hent);
		} else if (f[0] && f[0][0] == '-') {
			c = f[0];
			while (*c)
				conf.set(*c++, f[1]);
		}
	} while ((p - buf) < s);
}


char *Hosts::parseline(char *buf, char *f[])
{
        char *p;
        int i = 0;

	p = buf;
	if (*p == '#') {
		while (*p++ != '\n');
	} else {
		f[i++] = p;
		do {
			while (!isspace(*p))
				p++;
			if (*p == '\n')
				break;
			*p++ = 0;
			while (isspace(*p))
				p++;
			f[i++] = p;
		} while (*p);
		*p = 0;
	}
        while (i < 10)
                f[i++] = 0;
	return p + 1;
}

	
HostEnt::HostEnt()
{
	alias = name = user = password = wd = NULL;
}

HostEnt::HostEnt(HostEnt &he)
{
	Alias(he.Alias());
	Name(he.Name());
	User(he.User());
	Password(he.Password());
	Wd(he.Wd());
}

HostEnt::~HostEnt()
{
	if (alias)
		free(alias);
	if (name)
		free(name);
	if (user)
		free(user);
	if (password)
		free(password);
	if (wd)
		free(wd);
}

void HostEnt::Set(char *a, char *n, char *u, char *p, char *w)
{
	Alias(a);
	Name(n);
	User(u);
	Password(p);
	Wd(w);
}

void HostEnt::mkstr(char **s, char *n)
{
	if (n) {
		if (*n == '-')
			n = NULL;
		else
			*s = strdup(n);
	}
}

char *HostEnt::Alias(char *s = NULL)
{
	mkstr(&alias, s);
	return alias;
}

char *HostEnt::Name(char *s = NULL)
{
	mkstr(&name, s);
	return name;
}

char *HostEnt::User(char *s = NULL)
{
	mkstr(&user, s);
	return user;
}

char *HostEnt::Password(char *s = NULL)
{
	mkstr(&password, s);
	return password;
}

char *HostEnt::Wd(char *s = NULL)
{
	mkstr(&wd, s);
	return wd;
}

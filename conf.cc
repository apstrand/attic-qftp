/*
 *  qftp
 *  Copyright (C) 1997,1998 Peter Strand
 *  Distributed under the GNU Pulic Licence
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "conf.h"

Conf::Conf()
{
	setdef();
}

Conf::Conf(Conf &cf)
{
	opts = cf.opts;
}

Conf::Conf(Conf &cf, int argc, char *argv[], char *valid)
{
	Parse(cf, argc, argv, valid);
}

void Conf::Parse(Conf &cf, int argc, char *argv[], char *valid)
{
	char ign;
	int i;

	opts = cf.opts;
	
	argi = 0;
	ign = 0;
	for (i = 1;i < argc;i++) {
		if (argv[i][0] == '-') {
			int j = 1;
			ign = 0;
			while (argv[i][j]) {
				if (strchr("yu", argv[i][j])) {
					ign = 1;
					argptr = argv[i + 1];
				}
				if (strchr(valid, argv[i][j]))
					set(argv[i][j]);
				j++;
			}
		} else if (!ign) {
			if (argv[i][0])
				arg.push_back(argv[i]);
		} else
			ign = 0;
	}
}

void Conf::Reset()
{
	argi = 0;
}

char *Conf::GetNext()
{
	if (argi < arg.size())
		return arg[argi++];
	else
		return NULL;
}

int Conf::Args()
{
	return arg.size();
}

void Conf::print()
{
	printf("(b) Background:       %d\n", opts.bg);
	printf("(z) Un(g)zip (view):  %d\n", opts.proc);
	printf("(f) Force:            %d\n", opts.force);
	printf("(c) Continue (reget): %d\n", opts.cont);
	printf("(m) Preserve mtime:   %d\n", opts.mod);
	printf("(l) Long Listing:     %d\n", opts.longl);
	printf("(r) Reverse sort:     %d\n", opts.rev);
	printf("(s) Sort by size:     %d\n", opts.size);
	printf("(t) Sort by time:     %d\n", opts.time);
	printf("(a) Anonymous:        %d\n", opts.anon);
	printf("(y) Retry:Sleep   %2d:%2d\n", opts.retry, opts.retrsl);
	printf("(n) Forget host&user: %d\n", opts.noinf);
	printf("(e) Recursive:        %d\n", opts.rec);
	printf("(q) Quiet:            %d\n", opts.quiet);
	printf("(u) User:Pass         %s\n", opts.user);
}

void Conf::setdef()
{
	argptr = "";
	set('B');
	set('z');
	set('m');
	set('F');
	set('C');
	set('L');
	set('A');
	set('Y');
	set('N');
	set('E');
	set('Q');
	set('U');
	set('S');
	set('T');
	set('R');
}

void Conf::set(char c, char *s)
{
	char *p;
	if (s)
		argptr = s;
	switch (c) {
	case 'B':
	case 'b': opts.bg = (c & 0x20) >> 5; break;
	case 'Z':
	case 'z': opts.proc = (c & 0x20) >> 5; break;
	case 'F':
	case 'f': opts.force = (c & 0x20) >> 5; break;
	case 'C':
	case 'c': opts.cont = (c & 0x20) >> 5; break;
	case 'L':
	case 'l': opts.longl = (c & 0x20) >> 5; break;
	case 'A':
	case 'a': opts.anon = (c & 0x20) >> 5; break;
	case 'Y':
		opts.retry = 0; opts.retrsl = 0; break;
	case 'y':
		opts.retry = atoi(argptr);
		if ((p = strchr(argptr, ':'))) {
			opts.retrsl = atoi(p + 1);
		}
		break;
	case 'N':
	case 'n': opts.noinf = (c & 0x20) >> 5; break;
	case 'E':
	case 'e': opts.rec = (c & 0x20) >> 5; break;
	case 'Q':
	case 'q': opts.quiet = (c & 0x20) >> 5; break;
	case 'U': *opts.user = 0; break;
	case 'u': strcpy(opts.user, argptr); break;
	case 'S':
	case 's': opts.size = (c & 0x20) >> 5; break;
	case 'T':
	case 't': opts.time = (c & 0x20) >> 5; break;
	case 'R':
	case 'r': opts.rev = (c & 0x20) >> 5; break;
	case 'M':
	case 'm': opts.mod = (c & 0x20) >> 5; break;
		
	}
}


struct config *Conf::Get()
{
	return &opts;
}





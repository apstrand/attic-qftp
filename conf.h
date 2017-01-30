/*
 *  qftp
 *  Copyright (C) 1997,1998 Peter Strand
 *  Distributed under the GNU Pulic Licence
 */

#ifndef _CONF_H_
#define _CONF_H_

#include <vector.h>

struct config {
	char bg;	// b
	char proc;	// z
	char force;	// f
	char cont;	// c
	char longl;	// l
	char anon;	// a
	char retry;
	char retrsl;	// y
	char user[256];	// u
	char noinf;	// n
	char rec;	// e
	char quiet;	// q
	char size;	// s
	char time;	// t
	char rev;	// r
	char mod;	// m
};


class Conf {
	vector<char *> arg, opt;
	char *argptr;
	unsigned int argi;
public:
	struct config opts;
	Conf();
	Conf(Conf &cf);
	Conf(Conf &cf, int argc, char *argv[], char *valid);
	void Parse(Conf &cf, int argc, char *argv[], char *valid);
	int Args();
	void set(char c, char *p = NULL);
	void print();
	void setdef();
	struct config *Get();
	void Reset();
	char *GetNext();

};

#endif

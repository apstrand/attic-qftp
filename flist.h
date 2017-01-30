/*
 *  qftp
 *  Copyright (C) 1997,1998 Peter Strand
 *  Distributed under the GNU Pulic Licence
 */

#ifndef _FLIST_H_
#define _FLIST_H_

#include <stl.h>

class Entry 
{
 public:
	Entry();
	char *wd, *name, *date, *user, *group, *perms;
	int type, perm;
	long size;
};

class DirList {
	vector<Entry> ents;
	vector<Entry>::iterator iter;
	struct { bool operator()(const Entry &a, const Entry &b) const {
			return strcmp(a.name, b.name) < 0;
		}
	} cmpn;
	struct { bool operator()(const Entry &a, const Entry &b) const {
			return strcmp(a.date, b.date) < 0;
		}
	} cmpd;
	struct { bool operator()(const Entry &a, const Entry &b) const {
			return a.size < b.size;
		}
	} cmps;

 public:
	char *path;
	int isused, islong;
	DirList();

	void Flush();
	void Reset();

	int GetWidest();
	void Sort(int what, int dir);
	void Add(Entry &ent);
	void AddAt(Entry &e, int i);
	int GetNext(Entry &ent);
	int GetAt(Entry &ent, int i);
};
#endif

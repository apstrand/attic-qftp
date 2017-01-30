/*
 *  qftp
 *  Copyright (C) 1997,1998 Peter Strand
 *  Distributed under the GNU Pulic Licence
 */

#include <algo.h>


#include <string.h>
#include <stdlib.h>

#include "flist.h"

Entry::Entry()
{
	wd = name = date = user = group = NULL;
	type = perm = size = -1;
}

DirList::DirList()
{
	islong = 0;
	iter = ents.begin();
}


void DirList::Flush()
{
	ents.clear();
	iter = ents.begin();
}

void DirList::Reset()
{
	iter = ents.begin();
}

void DirList::Add(Entry &e)
{
	ents.push_back(e);
}


void DirList::AddAt(Entry &e, int i)
{
	ents[i] = e;
}


int DirList::GetNext(Entry &e)
{
	if (iter == ents.end())
		return 0;
	e = *iter++;
	return 1;
}

int DirList::GetAt(Entry &e, int i)
{
	if ((unsigned)i >= ents.size())
		return 0;
	e = ents[i];
	return 1;
}

int DirList::GetWidest()
{
	vector<Entry>::iterator wi = ents.begin();
	int n = 0, i;
	while (wi != ents.end()) {
		i = strlen((*wi).name);
		if (i > n)
			n = i;
		wi++;
	}
	return n;
}

void DirList::Sort(int what, int dir) {
	switch (what) {
	case 0:
		if (dir)
			sort(ents.rbegin(), ents.rend(), cmpn);
		else
			sort(ents.begin(), ents.end(), cmpn);
		break;
	case 1:
		if (dir)
			sort(ents.rbegin(), ents.rend(), cmpd);
		else
			sort(ents.begin(), ents.end(), cmpd);
		break;
	case 2:
		if (dir)
			sort(ents.rbegin(), ents.rend(), cmps);
		else
			sort(ents.begin(), ents.end(), cmps);
		break;
	}
}







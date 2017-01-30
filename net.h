/*
 *  qftp
 *  Copyright (C) 1997,1998 Peter Strand
 *  Distributed under the GNU Pulic Licence
 */

#ifndef _NET_H
#define _NET_H

#include <arpa/inet.h>
#include <netinet/in.h>

class Host {
	char name[512];
	struct hostent *hp;
	struct in_addr addr;
public:
	Host();
	Host(char *h);
	~Host();

	int Lookup();
	struct in_addr GetAddr();
	void SetAddr(struct in_addr);
	char *GetHost();
	void SetHost(char *);
};

class Socket {
	int s;
	struct sockaddr_in laddr, raddr;
	char *rhost;
	char *error;
	long status;

	char *msgLostConn;
public:
	Socket();
	Socket(Socket *sock);
	Socket(int s, sockaddr_in naddr);
	~Socket();
	
	int Connect(Host &h);
	int Listen();
	Socket *Accept();
	void Close();
	const char *GetError();

	char *GetRHost();
	int GetLPort();
	long GetLAddr();
	struct sockaddr_in GetRSAddr();
	struct sockaddr_in GetLSAddr();
	long GetStatus();
	
	int Read(char *buf, int len = -1);
	int Write(char *buf, int len);
	int Readln(char *buf);
	int Writeln(char *buf);
};


#endif







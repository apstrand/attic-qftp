/*
 *  qftp
 *  Copyright (C) 1997,1998 Peter Strand
 *  Distributed under the GNU Pulic Licence
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#include "net.h"


Socket::Socket()
{
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Socket::Socket.socket()");
	}
	laddr.sin_family = AF_INET;
	laddr.sin_port = 0;
	laddr.sin_addr.s_addr = htonl(INADDR_ANY);
	rhost = NULL;
	error = NULL;
	msgLostConn = strdup("Lost Connection");
}

Socket::Socket(Socket *sock)
{
	char *p;
	if (!sock)
		return;
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Socket::Socket(Socket *).socket()");
	}
	laddr = sock->GetLSAddr();
	raddr = sock->GetRSAddr();
	rhost = NULL;
	error = NULL;
	if ((p = sock->GetRHost())) {
		rhost = strdup(p);
	}
	msgLostConn = strdup("Lost Connection");
}

Socket::Socket(int sock, struct sockaddr_in naddr)
{
	s = sock;
	laddr = naddr;
	rhost = NULL;
	msgLostConn = strdup("Lost Connection");
}

Socket::~Socket()
{
	close(s);
	if (rhost)
		free(rhost);
	free(msgLostConn);
	
}

int Socket::Connect(Host &h)
{
	int i;
	char *p;
	if (h.Lookup()) {
		error = "Host lookup error\n";
		return -1;
	}
	raddr.sin_addr = h.GetAddr();
	raddr.sin_family = AF_INET;
	raddr.sin_port = htons(21);
	if (connect(s, (struct sockaddr *)&raddr, sizeof(raddr))) {
		error = strerror(errno);
		return -1;
	}
	i = sizeof(laddr);
	if (getsockname(s, (struct sockaddr *)&laddr, (unsigned int *)&i)) {
		error = strerror(errno);
		return -1;
	}
	if ((p = h.GetHost())) {
		rhost = strdup(p);
	}
	return 0;
}

int Socket::Listen()
{
	int i;
	laddr.sin_port = 0;
	if (bind(s, (struct sockaddr *)&laddr, sizeof(laddr))) {
		error = strerror(errno);
		return -1;
	}
	i = sizeof(laddr);
	if (getsockname(s, (struct sockaddr *)&laddr, (unsigned int *)&i)) {
		error = strerror(errno);
		return -1;
	}
	if (listen(s, 10)) {
		error = strerror(errno);
		return -1;
	}
	return 0;
}

Socket *Socket::Accept()
{
	int s2, i;
	struct sockaddr_in naddr;

	i = sizeof(naddr);
	if ((s2 = accept(s, (struct sockaddr *)&naddr, (unsigned int *)&i)) == -1) {
		error = strerror(errno);
		return NULL;
	}
	return new Socket(s2, naddr);
}

void Socket::Close()
{
	close(s);
}

long Socket::GetStatus()
{
	return status;
}

int Socket::GetLPort()
{
	return laddr.sin_port;
}

long Socket::GetLAddr()
{
	return laddr.sin_addr.s_addr;
}

char *Socket::GetRHost()
{
	return rhost;
}

struct sockaddr_in Socket::GetLSAddr()
{
	return laddr;
}

struct sockaddr_in Socket::GetRSAddr()
{
	return raddr;
}

int Socket::Read(char *buf, int len = -1)
{
	return read(s, buf, len);
}

int Socket::Write(char *buf, int len)
{
	return write(s, buf, len);
}

int Socket::Readln(char *buf)
{
	char c;
	int i = 0, r;
	
	while ((r = read(s, &c, 1)) && r != -1 && c != '\n') {
		buf[i++] = c;
	}
	if (r > 0)
		buf[i++] = c;
	else if (r == -1)
		error = strerror(errno);
	buf[i] = 0;
	return r;
}

int Socket::Writeln(char *buf)
{
	char tmp[1024];
	sprintf(tmp, "%s\r\n", buf);
	if (write(s, tmp, strlen(tmp)) == -1)
		return -1;
	else
		return 0;
	/*
	  if ((write(s, buf, strlen(buf)) != -1) &&
	  (write(s, "\r\n", 2) != -1))
	  return 0;
	  else {
	  error = strerror(errno);
	  return -1;
	  }
	*/
}

const char *Socket::GetError()
{
	return error;
}

Host::Host()
{
	hp = NULL;
}

Host::Host(char *host)
{
	addr.s_addr = 0;
	hp = NULL;
	strcpy(name, host);
}

Host::~Host()
{
}

void Host::SetHost(char *s)
{
	strcpy(name, s);
}

void Host::SetAddr(struct in_addr a)
{
	addr = a;
}

struct in_addr Host::GetAddr()
{
	return addr;
}

char *Host::GetHost()
{
	return name;
}


int Host::Lookup()
{
	if (!(hp = gethostbyname(name)))
		return -1;
	strcpy(name, hp->h_name);
	addr = *(struct in_addr*)hp->h_addr_list[0];
	return 0;
}

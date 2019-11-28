#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/fcntl.h>
#include "jsB_tcp.h"
#include "jsB_readstream.h"
#include "jsB_timers.h"
#include "jsB_arraybuffer.h"

#define U_NAME "TcpServer"

static void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

static in_port_t get_in_port(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return ((struct sockaddr_in*)sa)->sin_port;
    }
    return ((struct sockaddr_in6*)sa)->sin6_port;
}


const char* set_non_blocking(int sfd)
{
	int flags = fcntl(sfd, F_GETFL, 0);
	if (flags == -1) {
		return strerror(errno);
	}
	if (fcntl(sfd, F_SETFL, flags | O_NONBLOCK) == -1) {
		return strerror(errno);
	}
	return NULL;
}

#define MAXEVENTS 64
#define BUFSIZE 1024

/* send data when it's available */
static void jsB_TcpServer_prototype_handleResponseData(js_State *J)
{
	if (!js_isobject(J, 0)) {
		js_typeerror(J, "Expected session object");
	}
	js_getproperty(J, 0, "context");
	if (!js_isuserdata(J, -1, U_NAME)) {
        js_typeerror(J, "Method get TcpServer.prototype.handleResponseData called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
	}
	js_pop(J, 1);
	/* get fd */
	js_getproperty(J, 0, "fd");
	int fd = js_tointeger(J, -1);
	js_pop(J, 1);
	/* get wstream */
	js_getproperty(J, 0, "wstream");
	int wstream = js_gettop(J) - 1;
	js_getproperty(J, -1, "read");
	int wstreamRead = js_gettop(J) - 1;
	while (1) {
		/* buffer = stream.read(4096) */
		js_copy(J, wstreamRead);
		js_copy(J, wstream);
		js_call(J, 0);
		if (!jsB_ArrayBuffer_instance(J, -1)) {
			break;
		}
		js_back_store_t *backStore = jsB_ArrayBuffer_backstore(J, -1);
		int rv = send(fd, backStore->data, backStore->length, 0);
		if (rv != backStore->length) {
			if (rv == -1) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					/* stream.shift(buffer); */
					js_getproperty(J, wstream, "shift");
					js_copy(J, wstream);
					js_copy(J, -3); /* shift buffer back */
					js_call(J, 1);
					break;
				}
				js_error(J, "send: %s", strerror(errno));
			} else {
				/* stream.shift(buffer.slice(rv)) */
				js_getproperty(J, wstream, "shift");
				js_copy(J, wstream);
					js_copy(J, -3);
					js_getproperty(J, -1, "slice");
					js_rot2(J);
					js_pushnumber(J, (double)rv);
					js_call(J, 1);
				js_call(J, 1);
				js_pop(J, 2); /* pop buffer and return value */
				continue;
			}
		}
		js_pop(J, 1); /* pop buffer */
	}
	js_pushundefined(J);
}

static void jsB_TcpServer_prototype_handleResponseClose(js_State *J)
{
	if (!js_isobject(J, 0)) {
		js_typeerror(J, "Expected session object");
	}
	js_getproperty(J, 0, "context");
	if (!js_isuserdata(J, -1, U_NAME)) {
        js_typeerror(J, "Method get TcpServer.prototype.handleResponseClose called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
	}
	// printf("%s\n", "close connection");
	/* get fd */
	js_getproperty(J, 0, "fd");
	int fd = js_tointeger(J, -1);
	js_pop(J, 1);
	/* delete session */
	js_getlocalregistry(J, -1, "sessions");
	js_delindex(J, -1, fd);
	close(fd);
	/* XXX: should we emit close event on request stream? */
	js_pushundefined(J);
}

static void jsB_TcpServer_prototype_poll(js_State *J)
{
	if (!js_isuserdata(J, 0, U_NAME)) {
        js_typeerror(J, "Method get TcpServer.prototype.poll called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
	}
	if (!js_isnumber(J, 1)) {
		js_typeerror(J, "Expected timeout as first argument");
	}
	char buffer[32 * 1024]; 
	const char *err;
	struct sockaddr_storage remoteaddr;
	socklen_t addrlen = sizeof remoteaddr;
	int timeout = js_tointeger(J, 1);
	struct epoll_event events[MAXEVENTS];
	struct epoll_event event;
	js_getlocalregistry(J, 0, "sfd");
	int listener = js_tointeger(J, -1);
	js_getlocalregistry(J, 0, "efd");
	int epollfd = js_tointeger(J, -1);
  	char tmp[BUFSIZE];
  	char method[BUFSIZE];   /* request method */
  	char uri[BUFSIZE];      /* request uri */
  	char protocol[BUFSIZE]; /* request protocol */
	char remoteIP[INET6_ADDRSTRLEN];
	int rv = epoll_wait(epollfd, events, MAXEVENTS, timeout);

	if (rv == -1) {
		js_error(J, "epoll_wait: %s", strerror(errno));
		return;
	}
	if (rv == 0) {
		js_pushnumber(J, 0);
		return;
	}

	for (int i = 0; i < rv; ++i) {
		if (events[i].events & (EPOLLHUP | EPOLLERR)) {
			/* close read stream */
			js_getlocalregistry(J, 0, "sessions");
			if (js_isdefined(J, -1)) {
				js_getindex(J, -1, events[i].data.fd);
				js_getproperty(J, -1, "rstream");
				js_getproperty(J, -1, "close");
				js_rot2(J);
				js_call(J, 0);
				js_pop(J, 2);
				/* delete session */
				js_delindex(J, -1, events[i].data.fd);
			}
			js_pop(J, 1);
			/* close connection */
			close(events[i].data.fd);
			continue;
		}
		/* new connection */
		if (events[i].data.fd == listener) {
			/* we might have multiple connections */
			while (1) {
				int newfd = accept(listener, (struct sockaddr*)&remoteaddr, &addrlen);
				if (newfd == -1) {
					if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
						/* all connections were processed */
						break;
					}
					js_error(J, "accept: %s", strerror(errno));
				}
				if ((err = set_non_blocking(newfd))) {
					js_error(J, "%s", err);
				}
				event.data.fd = newfd;
				event.events = EPOLLIN | EPOLLOUT | EPOLLET;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, newfd, &event) == -1) {
					js_error(J, "epoll_ctl: %s", strerror(errno));
				}
				/* create new session */
				js_newobject(J);
				/* fd */
				js_pushnumber(J, (double)newfd);
				js_setproperty(J, -2, "fd");
				/* address */
	            js_newobject(J);
		            js_pushconstu(J, 
		            	(remoteaddr.ss_family == AF_INET) ? "IPv4" : "IPv6", 0);
		            js_setproperty(J, -2, "family");
		            js_pushstringu(J, 
		            	inet_ntop(remoteaddr.ss_family, 
		            		get_in_addr((struct sockaddr*)&remoteaddr), 
		            			remoteIP, INET6_ADDRSTRLEN), 0);
		            js_setproperty(J, -2, "host");
		            js_pushnumber(J, (double)ntohs(get_in_port((struct sockaddr*)&remoteaddr)));
		            js_setproperty(J, -2, "port");	
		        js_setproperty(J, -2, "address");
		        /* rstream */
		        jsB_ReadStream(J);
		        js_construct(J, 0);
		        js_setproperty(J, -2, "rstream");
		        /* wstream */
		        jsB_ReadStream(J);
		        js_construct(J, 0);
		        js_setproperty(J, -2, "wstream");
		        /* store server context */
		        js_copy(J, 0);
		        js_setproperty(J, -2, "context");
		        /* store new session in sessions */
		        js_getlocalregistry(J, 0, "sessions");
		        js_copy(J, -2); /* new session */
		        js_setindex(J, -2, newfd);
		        js_pop(J, 1);
		        /* notify about new connection */
		        js_getlocalregistry(J, 0, "handler");
		        js_pushundefined(J);
		        js_getproperty(J, -3, "address");
		        js_getproperty(J, -4, "rstream");
		        js_getproperty(J, -5, "wstream");
		        js_call(J, 3);
		        js_pop(J, 1); /* pop return value */
		        int session = js_gettop(J) - 1;

		        /* add wstream data listener */
		     	js_getproperty(J, session, "wstream");
		     	js_getproperty(J, -1, "addEventListener");
		     	js_rot2(J);
		     	js_pushstring(J, "data");
		     		js_newcfunction(J, jsB_TcpServer_prototype_handleResponseData, "TcpServer.prototype.handleResponseData", 0);
		     		js_getproperty(J, -1, "bind");
		     		js_rot2(J);
		     		js_copy(J, session); /* this */
		     		js_call(J, 1);
		     	js_call(J, 2);
		     	js_pop(J, 1); /* pop response */

		     	/* add wstream close listener */
		     	js_getproperty(J, session, "wstream");
		     	js_getproperty(J, -1, "addEventListener");
		     	js_rot2(J);
		     	js_pushstring(J, "close");
		     		js_newcfunction(J, jsB_TcpServer_prototype_handleResponseClose, "TcpServer.prototype.handleResponseClose", 0);
		     		js_getproperty(J, -1, "bind");
		     		js_rot2(J);
		     		js_copy(J, session); /* this */
		     		js_call(J, 1);
		     	js_call(J, 2);
		     	js_pop(J, 2);

		     	// printf("new connection: %i\n", newfd);
			}
			continue;
		}

		if (events[i].events & EPOLLIN) {
			// printf("pollin %i\n", events[i].data.fd);
			js_getlocalregistry(J, 0, "sessions");
			if (!js_isdefined(J, -1)) {
				js_error(J, "expected session object");
			}
			js_getindex(J, -1, events[i].data.fd);
			js_getproperty(J, -1, "rstream");
			int rstream = js_gettop(J) - 1;
			js_getproperty(J, -1, "push");
			int rstreamPush = js_gettop(J) - 1;
			js_getproperty(J, -2, "end");
			int rstreamEnd = js_gettop(J) - 1;
			while (1) {
				int nbytes = recv(events[i].data.fd, buffer, sizeof buffer, 0);
				/* client has closed connection */
				if (nbytes == 0) {
					// printf("connecton closed: %i\n", events[i].data.fd);
					/* close read stream */
					js_getproperty(J, -3, "close");
					js_copy(J, rstream);
					js_call(J, 0);
					js_pop(J, 1);
					/* delete session */
					js_delindex(J, -5, events[i].data.fd);
					/* closing the descriptor will make epoll remove it
                     from the set of descriptors which are monitored. */
					close(events[i].data.fd);
					break;
				}
				if (nbytes < 0) {
					/* done reading for now */
					if (errno == EAGAIN) {
						/* end read stream */
						js_copy(J, rstreamEnd);
						js_copy(J, rstream);
						js_call(J, 0);
						js_pop(J, 1);
						break;
					}
					js_error(J, "recv: %s", strerror(errno));
				}
				/* rstream.push(arraybuffer) */
				js_copy(J, rstreamPush);
				js_copy(J, rstream);
				jsB_ArrayBuffer(J);
				js_pushnumber(J, nbytes);
				js_construct(J, 1);
				js_back_store_t *backStore = jsB_ArrayBuffer_backstore(J, -1);
				memcpy(backStore->data, buffer, nbytes);
				js_call(J, 1);
				js_pop(J, 1);
			}
			js_pop(J, 5);
		}

		if (events[i].events & EPOLLOUT) {
			js_getlocalregistry(J, 0, "sessions");
			if (!js_isdefined(J, -1)) {
				js_error(J, "expected session object");
			}
			js_getindex(J, -1, events[i].data.fd);
			if (js_isdefined(J, -1)) {
				js_copy(J, -1);
				js_callscoped2(J, jsB_TcpServer_prototype_handleResponseData, 0);
				js_pop(J, 1);
			}
			js_pop(J, 2);
		}
	}
}

static void jsB_TcpServer_prototype_createSocket(js_State *J)
{

	const char* port = js_tostring(J, 1);
	const char* err;
	int listener; /* listening socket descriptor */
	struct addrinfo hints, *ai, *p;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int rv;
	if ((rv = getaddrinfo(NULL, port, &hints, &ai)) != 0) {
		js_error(J, "getaddrinfo: %s\n", gai_strerror(rv));
	}

	for (p = ai; p != NULL; p = p->ai_next) {
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0) {
			continue;
		}
		/* loose pesky "address already in use" error message */
		int yes = 1;
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			close(listener);
			continue;
		}
		break;
	}

	if (p == NULL) {
		js_error(J, "failed to bind on port %s\n", port);
	}

	freeaddrinfo(ai);

	if ((err = set_non_blocking(listener))) {
		close(listener);
		js_error(J, "%s", err);
	}

	js_pushnumber(J, (double)listener);
}


static void jsB_TcpServer_prototype_listen(js_State *J) 
{
	if (!js_isuserdata(J, 0, U_NAME)) {
        js_typeerror(J, "Method get TcpServer.prototype.listen called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
	}
	js_getlocalregistry(J, 0, "sfd");
	if (js_isdefined(J, -1)) {
		js_error(J, "the socket is already opened");
	}
	if (!js_isnumber(J, 1)) {
		js_typeerror(J, "expected port number as first argument");
	}

	js_copy(J, 0);
	js_copy(J, 1);
	js_callscoped2(J, jsB_TcpServer_prototype_createSocket, 1);
	int listener = js_tointeger(J, -1);

	if (listen(listener, SOMAXCONN) == -1) {
		close(listener);
		js_error(J, "listen: %s", strerror(errno));
	}

	int efd = epoll_create1(0);
	if (efd == -1) {
		close(listener);
		js_error(J, "epoll_create1: %s", strerror(errno));
	}

	struct epoll_event event;

	event.data.fd = listener;
	event.events = EPOLLIN | EPOLLET;

	if (epoll_ctl(efd, EPOLL_CTL_ADD, listener, &event) == -1) {
		close(listener);
		close(efd);
		js_error(J, "epoll_ctl: %s", strerror(errno));
	}

	js_pushnumber(J, (double)listener);
	js_setlocalregistry(J, 0, "sfd");
	js_pushnumber(J, (double)efd);
	js_setlocalregistry(J, 0, "efd");

	/* set timer */
	js_getregistry(J, "eventloop");
	if (js_isundefined(J, -1)) {
		close(efd);
		close(listener);
		js_error(J, "could not find eventloop");
	}

	js_getproperty(J, -1, "addHandler");
	js_rot2(J);
	js_newobject(J);

	/* this.poll.bind(this) */
	js_newcfunction(J, jsB_TcpServer_prototype_poll, "TcpServer.prototype.poll", 1);
	js_getproperty(J, -1, "bind");
	js_rot2(J);
	js_copy(J, 0); /* copy this */
	js_call(J, 1);
	js_setproperty(J, -2, "handler");

	js_pushconst(J, "io");
	js_setproperty(J, -2, "type");

	js_pushboolean(J, 1);
	js_setproperty(J, -2, "persistent");

	js_call(J, 1);

	if (!js_isnumber(J, -1)) {
		close(efd);
		close(listener);
		js_typeerror(J, "expected addHandler to return id");
	}

	js_setlocalregistry(J, 0, "timerid");

	js_pushundefined(J);
}

static void jsB_TcpServer_prototype_close(js_State *J)
{
	if (!js_isuserdata(J, 0, U_NAME)) {
        js_typeerror(J, "Method get TcpServer.prototype.close called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
	}
	/* close(socket) */
	js_getlocalregistry(J, 0, "sfd");
	if (js_isdefined(J, -1)) {
		close(js_tointeger(J, -1));
	}
	/* close(epollfd) */
	js_getlocalregistry(J, 0, "efd");
	if (js_isdefined(J, -1)) {
		close(js_tointeger(J, -1));
	}
	/* eventloop.removeHandler(timerid) */
	js_getlocalregistry(J, 0, "timerid");
	if (js_isdefined(J, -1)) {
		js_getregistry(J, "eventloop");
		js_getproperty(J, -1, "removeHandler");
		js_rot2(J);
		js_copy(J, -3);
		js_call(J, 1);
	}

	js_pushundefined(J);
}

static void jsB_Tcp_createServer(js_State *J) 
{
	if (!js_iscallable(J, 1)) {
		js_typeerror(J, "expected response handler to be set");
	}
	js_getregistry(J, "jsB_TcpServer_prototype");
	js_newuserdata(J, U_NAME, NULL, NULL);
	js_copy(J, 1);
	js_setlocalregistry(J, -2, "handler");
	js_newobject(J);
	js_setlocalregistry(J, -2, "sessions");
}

void jsB_Tcp(js_State *J)
{
	js_getregistry(J, "jsB_Tcp");
	/* check cached constructor */
	if (js_isdefined(J, -1)) {
		return;
	}
	/* initialize it otherwise */
	js_pop(J, 1);
	/* http namespace */
	js_newobject(J);
	js_newcfunction(J, jsB_Tcp_createServer, "createServer", 1);
	js_defproperty(J, -2, "createServer", JS_READONLY | JS_DONTCONF);
	js_copy(J, -1);
	js_setregistry(J, "jsB_Tcp");

	/* HttpServer prototype */
	js_newobject(J);
	js_newcfunction(J, jsB_TcpServer_prototype_listen, "listen", 1);
	js_defproperty(J, -2, "listen", JS_READONLY | JS_DONTCONF);
	js_newcfunction(J, jsB_TcpServer_prototype_close, "close", 0);
	js_defproperty(J, -2, "close", JS_READONLY | JS_DONTCONF);
	js_setregistry(J, "jsB_TcpServer_prototype");
}

/*****************************************************************************
*    Copyright (C) 2016 by Stoian Ivanov                                     *
*    s.ivanov@allterco.com                                                   *
*                                                                            *
*    This program is free software: you can redistribute it and/or modify    *
*    it under the terms of the GNU General Public License as published by    *
*    the Free Software Foundation, version 2 of the License                  *
*                                                                            *
*    This program is distributed in the hope that it will be useful,         *
*    but WITHOUT ANY WARRANTY; without even the implied warranty of          *
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
*    GNU General Public License for more details.                            *
*                                                                            *
*    You should have received a copy of the GNU General Public License       *
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <sys/epoll.h>
#include "bindaddr.h"
#include "urlparse.h"
#include "buffer.h"

const char *purl=NULL;
#define PURL_OPT  1

const char *surl=NULL;
#define SURL_OPT 2


static struct option long_options[] =
{
	{"purl",   required_argument, 0, PURL_OPT},
	{"surl",   required_argument, 0, SURL_OPT},
	{0, 0, 0, 0}
};



void usage(const char *name){
	printf(
	"%s --purl <rtsp url to register at>  --surl <rtsp url to register>\n\n"
	"  This tool tries to register <rtsp url to register> to live555ProxyServer running at  <rtsp url to register at>  telling the proxy to "
	"reuse the TCP connection then resends the rest of the RTSP traffic to the host and port mentioned in <rtsp url to register> Default rtsp port is 554 \n"
	,name);
	exit(0);
}


int psok; //proxy connection socket
int ssok; //rtsp source connection socket

parsedurl_t purl_,surl_; //parsed purl,surl

enum {
	connecting=0,registering
} pstate=connecting;


#define PSTATE_CHAR "CR"
#define PSTATEC (PSTATE_CHAR[(unsigned)pstate])

buffer_t *ptx;
buffer_t *prx;

#define BUFF_SZ 2048

#define PTX_SZ  BUFF_SZ 
#define PRX_SZ  BUFF_SZ 

void handle_psok(uint32_t event) {
	if (event & (EPOLLERR|EPOLLHUP)) {
		printf("HUP or ERR on proxy connection in state %c\n",PSTATEC);
		exit(11);
	}
	if (event & EPOLLIN ) { //writable
		unsigned tosend=buffer_datalen(ptx);
		if (tosend) {
			while (tosend) {
				errno=0;
				int written= send(psok,ptx->datastart,tosend,0);
				if (written<0) {
					if (errno==EINTR) continue; 
					if (errno==EAGAIN || errno==EWOULDBLOCK || errno==EWOULDBLOCK) break;
					printf("ERR while writing over proxy connection in state %c\n",PSTATEC);
					exit(11);
				} else if (written==0) break;
				tosend-=written;
				//TODO:fix epoll
			}
		}
	}
	if (event & EPOLLOUT ) { //readable
		
	}
}

void handle_ssok(uint32_t event) {
	
}


int main(int argc, char **argv) {
	
	//option parsing:
	int getoptidx=0;
	while(1){
		int optnum=getopt_long_only (argc,argv,"",long_options,&getoptidx);
		/* Detect the end of the options. */
		if (optnum == -1)
			break;
		if (optnum == '?') {
			//bad option, getopt has printed a message! just quit!
			return 2;
		}
		
		
		switch (optnum)
		{
			case PURL_OPT:
				purl=optarg;
				break;
			case SURL_OPT:
				surl=optarg;
				break;
			default:
				usage(argv[0]);
				
				return 2;
		}
	}	
	
	if (purl==NULL || surl==NULL) usage(argv[0]);
	//printf("starting with purl:%s surl:%s\n",purl,surl);
	
	if (!parseurl(purl,&purl_)) {  printf ("Failed to parse purl [%s] as URL\n",purl); return 1;}
	if (!parseurl(surl,&surl_)) {  printf ("Failed to parse surl [%s] as URL\n",surl); return 1;}
	
	if (purl_.port==0) purl_.port=554;
	if (surl_.port==0) surl_.port=554;

	psok=tcpsocket_nb();
	if (!psok) {printf ("Failed to obtain a socket for proxy connection\n"); return 2;}
	canreuseaddr(psok);
	setnodelay(psok,true);
	if (!connsockhl(psok,purl_.host,purl_.host_l,purl_.port)){
		printf ("Failed to connect to proxy at %.*s:%hu\n",purl_.host_l,purl_.host,purl_.port); 
		return 2;
	}
	
	ptx=buffer_malloc(PTX_SZ);
	prx=buffer_malloc(PRX_SZ);
	if (!ptx || !prx) {
		printf("Failed to allocate tx/rx buffers for proxy connection\n"); 
		return 3;
	}
	
	int epfd=epoll_create(2);
	struct epoll_event epev[2],*cev=epev;
	
	cev->events=EPOLLIN|EPOLLOUT;
	cev->data.fd=psok;
	epoll_ctl(epfd,EPOLL_CTL_ADD, psok ,cev);
	
	while (true) {
		int cnt=epoll_wait(epfd, epev, 2, 5000);
		if (cnt==0) {
			printf("timout waiting for events! All done?\n");
			return 0;
		}
		
		for (cev=epev; cnt>0; cnt--,cev++){
			
		}
	}
	
	return 0;
}

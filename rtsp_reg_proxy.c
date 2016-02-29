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

#define _GNU_SOURCE //we need some memmem 
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <sys/epoll.h>
#include "inettools.h"
#include "urlparse.h"
#include "buffer.h"
#include "epollio.h"

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

epollio_t ep;

void usage(const char *name){
	printf(
	"%s --purl <rtsp url to register at>  --surl <rtsp url to register>\n\n"
	"  This tool tries to register <rtsp url to register> to live555ProxyServer running at  <rtsp url to register at>  telling the proxy to "
	"reuse the TCP connection then resends the rest of the RTSP traffic to the host and port mentioned in <rtsp url to register> Default rtsp port is 554 \n"
	,name);
	exit(0);
}


int psok=0; //proxy connection socket
int ssok=0; //rtsp source connection socket

parsedurl_t purl_,surl_; //parsed purl,surl

enum {
	connecting=0,registering,waitok,waitokend,proxy
} pstate=connecting;


#define PSTATE_CHAR "CR"
#define PSTATEC (PSTATE_CHAR[(unsigned)pstate])

buffer_t *ptx;
buffer_t *prx;

#define BUFF_SZ 2048

#define PTX_SZ  BUFF_SZ 
#define PRX_SZ  BUFF_SZ 

typedef enum {
	incomplete,isok,isNotok
} pres_t;

pres_t tryParseResp (const char *b,unsigned len) {
	if (len<13) return incomplete;
	//1234567890123
	//RTSP/1.0 200 OK
	if (b[8]!=' ') return isNotok;
	if (memcmp(b,"RTSP/1.0",8)!=0)  return isNotok;
	const char *split=b+8;
	const char *splitend=b+len-5;
	while (split<splitend && *split==' ') split++;
	if (*split==' ') return incomplete;
	if (split[0]=='2' && split[1]=='0' && split[2]=='0' && split[3]==' ') return isok;
	return isNotok;
}

static bool startsrccon(){
	ssok=tcpsocket_nb();
	if (!ssok) return false;
	canreuseaddr(ssok);
	setnodelay(ssok,true);
	if (!connsockhl(ssok,surl_.host,surl_.host_l,surl_.port)){
		printf ("Failed to connect to source at %.*s:%d\n",surl_.host_l,surl_.host,surl_.port); 
		return false;
	}
	epoll_data_t epdata;
	epdata.fd=ssok;
	epollio_add (&ep,ssok,&epdata,EPOLLIN|EPOLLOUT|EPOLLET);

	return true;
}
bool handle_ssok(epollio_t *ep,uint32_t event);
bool handle_psok(epollio_t *ep, uint32_t event) {
	if (event & (EPOLLERR|EPOLLHUP)) {
		//flush read if needed
		if (event & EPOLLIN) handle_psok(ep, EPOLLIN);
		printf("HUP or ERR on proxy connection in state %c\n",PSTATEC);
		close(psok);
		close(ssok);
		return true;
	}
	if (event & EPOLLOUT ) { //writable
		unsigned written;
		if (pstate==connecting) {
			unsigned flen=buffer_freelen(ptx);
			int written=snprintf(ptx->freestart, flen, "REGISTER %s RTSP/1.0\r\nCSeq: 1\r\nTransport: reuse_connection; npreferred_delivery_protocol=udp; proxy_url_suffix=%.*s\r\n\r\n",surl,purl_.uri_l-1,purl_.uri+1);
			if (written>=flen) {
				printf ("Failed to prepate register rq!?\n");
				return true;
			} 
			printf("===== register request =====\n%s============================\n",ptx->freestart);
			ptx->freestart+=written;
			pstate=registering;
			return handle_psok(ep, event);
		} else if (pstate==registering){
			if (buffer_writeout(ptx,psok,&written)==w_allwritten) {
				pstate=waitok;
				return handle_psok(ep, event|EPOLLIN); //asume readable 
			}
		} else if (pstate==proxy) {
			if (buffer_writeout(ptx,psok,&written)==w_allwritten && written!=0) {
				printf("==== src-to-proxy flushed out\n"); 
				handle_ssok(ep,EPOLLIN); //try reding from src 
			}
		}
	}
	
	if (event & EPOLLIN ) { //readable
		unsigned dl;
		if (buffer_readin(prx,psok,&dl)==r_overflow) {
			printf("Read overflow?! rx buf now:\n%.*s\n====================\n",dl,prx->datastart);
			return true;
		}
		if (dl==0) return false; //nothing to consider actualy 
		
		if (pstate==waitok) {
			printf("======= %u bytes in 'waitok' ======\n%.*s\n=====================\n",dl,dl,prx->datastart);
			pres_t pres=tryParseResp(prx->datastart,dl);
			if (pres==isok) {
				printf("Proxy ACKed the stream..\n");
				buffer_reset(ptx);
				if (!startsrccon()) {
					printf("Failed to start src conn!\n");
					return true;
				};
				pstate=waitokend;
				return handle_psok(ep, event);
			} else if (pres==isNotok) {
				printf("Proxy did not accepted the stream!\n");
				return false;
			}
			return false; //incomplete 
		}
		if (pstate==waitokend) {
			char *pos=(char *)memmem(prx->datastart,dl,"\r\n\r\n",4);
			if (pos) {
				prx->datastart=pos+4;
				dl=buffer_datalen(prx);
				printf("End of OK reply found! data to proxy atm (len:%u) ======\n%.*s\n=========================\n",dl,(int)dl,prx->datastart);
				pstate=proxy;
				return handle_psok(ep, event);
			}
			return false; //wait more data
		}
		if (pstate==proxy) {
			printf("======= %u bytes from proxy in 'proxy' ======\n%.*s\n=====================\n",dl,dl,prx->datastart);
			handle_ssok(ep,EPOLLOUT); //try sending to src 
		}
	}
	return false;
}

bool handle_ssok(epollio_t *ep,uint32_t event) {
	if (event & (EPOLLERR|EPOLLHUP)) {
		//flush read if needed
		if (event & EPOLLIN) handle_ssok(ep, EPOLLIN);
		printf("HUP or ERR on scr connection\n");
		close(ssok);
		close(psok);
		return true;
	}
	if (event & EPOLLIN ) { //readable
		unsigned dl;
		if (buffer_readin(ptx,ssok,&dl)==r_overflow) {
			printf("Read overflow in src?! rx buf now:\n%.*s\n====================\n",dl,ptx->datastart);
			return true;
		}
		if (dl>0) {
			printf("======= %u bytes from proxy data to send ======\n%.*s\n=====================\n",dl,(int)buffer_datalen(ptx) ,ptx->datastart);
			handle_psok(ep,EPOLLOUT); //try sending to proxy
		}
	}
	if (event & EPOLLOUT ) { //writable
		unsigned written;
		if (buffer_writeout(prx,ssok,&written)==w_allwritten && written>0) {
			printf("==== proxy-to-src flushed out\n"); 
			handle_psok(ep,EPOLLIN); //try reding from proxy
		}
	}
	
	return false;
}

bool handleevents (epollio_t *ep, epoll_data_t*epdata, uint32_t events) {
	if (epdata->fd==psok) return handle_psok(ep,events); 
	if (epdata->fd==ssok) return handle_ssok(ep,events); 
	return true;
}

bool handletmo (epollio_t *ep){
	printf("timout waiting for events! All done?\n");
	return true;
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
		printf ("Failed to connect to proxy at %.*s:%d\n",purl_.host_l,purl_.host,purl_.port); 
		return 2;
	}
	
	ptx=buffer_malloc(PTX_SZ);
	prx=buffer_malloc(PRX_SZ);
	
	if (!ptx || !prx) {
		printf("Failed to allocate tx/rx buffers for proxy connection\n"); 
		return 3;
	}
	
	epoll_data_t epdata;
	epollio_init(&ep,10000,handleevents);
	
	epdata.fd=psok;
	epollio_add (&ep,psok,&epdata,EPOLLIN|EPOLLOUT|EPOLLET);
	
	ep.timeout=handletmo;
	
	epollio_run(&ep);
	printf("epollio loop ended!\n"); 
	return 0;
}

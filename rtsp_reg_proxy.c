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


int psok; //proxy connection socket
int ssok; //rtsp source connection socket

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


bool handle_psok(epollio_t *ep, uint32_t event) {
	if (event & (EPOLLERR|EPOLLHUP)) {
		//flush read if needed
		if (event & EPOLLIN) handle_psok(ep, EPOLLIN);
		printf("HUP or ERR on proxy connection in state %c\n",PSTATEC);
		return true;
	}
	if (event & EPOLLOUT ) { //writable
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
			if (buffer_writeout(ptx,psok)==w_allwritten) {
				pstate=waitok;
				return handle_psok(ep, event|EPOLLIN); //asume readable 
			}
		}
	}
	
	if (event & EPOLLIN ) { //readable
		if (pstate==waitok) {
			if (buffer_readin(prx,psok)==r_overflow) {
				unsigned dl=buffer_datalen(prx);
				printf("Read overflow?! rx buf now:\n%.*s\n====================\n",dl,prx->datastart);
				return true;
			}
			unsigned dl=buffer_datalen(prx);
			if (dl!=0) {
				printf("======= %u bytes in 'waitok' ======\n%.*s\n=====================\n",dl,dl,prx->datastart);
				pres_t pres=tryParseResp(prx->datastart,dl);
				if (pres==isok) {
					pstate=waitokend;
					return handle_psok(ep, event);
				} else if (pres==isNotok) {
					printf("Proxy did not accepted the stream!\n");
					return false;
				} else return false; //incomplete 
				//buffer_reset(prx);
			}
			return false;
		}
		if (pstate==waitok) {
			if (buffer_readin(prx,psok)==r_overflow) {
				unsigned dl=buffer_datalen(prx);
				printf("Read overflow?! rx buf now:\n%.*s\n====================\n",dl,prx->datastart);
				return true;
			}
			//TODO
		}
	}
	return false;
}

bool handle_ssok(epollio_t *ep,uint32_t event) {
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
		printf ("Failed to connect to proxy at %.*s:%hu\n",purl_.host_l,purl_.host,purl_.port); 
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

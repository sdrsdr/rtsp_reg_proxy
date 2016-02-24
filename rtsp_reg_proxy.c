#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <sys/epoll.h>
#include "bindaddr.h"
#include "urlparse.h"

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
	parsedurl_t purl_,surl_;
	if (!parseurl(purl,&purl_)) {  printf ("Failed to parse purl [%s] as URL\n",purl); return 1;}
	if (!parseurl(surl,&surl_)) {  printf ("Failed to parse surl [%s] as URL\n",surl); return 1;}
	
	if (purl_.port==0) purl_.port=554;
	if (surl_.port==0) surl_.port=554;

	int psok=tcpsocket_nb();
	if (!psok) {printf ("Failed to obtain a socket for proxy connection\n"); return 2;}
	canreuseaddr(psok);
	setnodelay(psok,true);
	if (!connsockhl(psok,purl_.host,purl_.host_l,purl_.port)){
		printf ("Failed to connect to proxy at %.*s:%hu\n",purl_.host_l,purl_.host,purl_.port); 
		return 2;
	}
	/*
	int epfd=epoll_create(2);
	struct epoll_event epev;
	epev.events=EPOLLIN;
	epev.data.fd= STDIN_FILENO;
	epoll_ctl(epfd,EPOLL_CTL_ADD, STDIN_FILENO ,&epev);
	*/
	
	return 0;
}

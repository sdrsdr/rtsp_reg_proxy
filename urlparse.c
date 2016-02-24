/***************************************************************************
 *   Copyright (C) 2016 by Stoian Ivanov                                   *
 *   s.ivanov@allterco.com                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License version  *
 *   2 as published by the Free Software Foundation;                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <string.h>
#include <stdlib.h>

#include "urlparse.h"
#include "strtoint.h"

bool parseurl(const char *url, parsedurl_t *pr,unsigned maxlen){
	enum {
		proto,user_or_host,pass_or_port,host,port,uri
	} state=proto;
	memset(pr,0,sizeof(*pr));
	const char *user_or_host_start=NULL;
	unsigned user_or_host_len=0;
	const char *pass_or_port_start=NULL;
	pr->proto=url;
	while (maxlen) {
		char c=*url; 
		if (c==0) break;
		url++; maxlen--;
		switch (state) {
			case proto:{
				if (c==':'){
					if (maxlen>2 && url[0]=='/' && url[1]=='/') {
						pr->proto_l=(url-pr->proto)-1;
						url+=2; maxlen-=2; //skip "//" past ':' 
						
						state=user_or_host;
						if (*url==':' || *url=='@' || *url=='/') return false;
						user_or_host_start=url;
						continue;
					}
				}
			} ; break;
			case user_or_host:{
				if (c==':'){ //end of user or end of host ..
					user_or_host_len=(url-user_or_host_start)-1;
					
					state=pass_or_port;
					if (*url==':' || *url=='@' || *url=='/') return false;
					pass_or_port_start=url;
					continue;
					
				} else if (c=='@') { //end of user start of host
					pr->user=user_or_host_start;
					pr->user_l=(url-user_or_host_start)-1;
					
					state=host;
					if (*url==':' || *url=='@' || *url=='/') return false;
					pr->host=url;
					continue;
				} else if (c=='/') { //end of host! start of uri
					pr->host=user_or_host_start;
					pr->host_l=(url-user_or_host_start)-1;
					
					//DONE!
					state=uri;
					pr->uri=url-1;
					pr->uri_l=maxlen+1; 
					maxlen=0;
					break;
				}
			} ; break;
			case pass_or_port:{
				if (c=='@') { //end of pass! user_or_host was USER!
					pr->user=user_or_host_start;
					pr->user_l=user_or_host_len;
					pr->pass=pass_or_port_start;
					pr->pass_l=(url-pass_or_port_start)-1;
					state=host;
					if (*url==':' || *url=='@' || *url=='/') return false;
					pr->host=url;
					continue;
				} else if (c=='/') { //end of port! start of uri user_or_host was HOST!
					pr->host=user_or_host_start;
					pr->host_l=user_or_host_len;
					unsigned long int res;
					if (!trystrtoul(&res,pass_or_port_start,NULL)){
						pr->port=-1;
					} else {
						if (res>0  && res<=65535) pr->port=(unsigned) res;
						else pr->port=-2;
					}
					//DONE!
					state=uri;
					pr->uri=url-1;
					pr->uri_l=maxlen+1; 
					maxlen=0;
					break;
				}
			} ; break;
			case host:{
				if (c==':') { //end of host! start of port
					pr->host_l=(url-pr->host)-1;
					
					state=port;
					if (*url==':' || *url=='@' || *url=='/') return false;
					pass_or_port_start=url;
					continue;
				} else if (c=='/') { //end of host! start of uri 
					pr->host_l=(url-pr->host)-1;
					
					//DONE!
					state=uri;
					pr->uri=url-1;
					pr->uri_l=maxlen+1; 
					maxlen=0;
					break;
				}
			} ; break;
			case port:{
				if (c=='/') { //end of port! start of uri
					unsigned long int res;
					if (!trystrtoul(&res,pass_or_port_start,NULL)){
						pr->port=-1;
					} else {
						if (res>0  && res<=65535) pr->port=(unsigned) res;
						else pr->port=-2;
					}
					//DONE!
					state=uri;
					pr->uri=url-1;
					pr->uri_l=maxlen+1; 
					maxlen=0;
					break;
				}
			} ; break;
			case uri:{
				//this hould not be reached anyways
				maxlen=0;break;
			} ; break;
		}//switch (state)
	}//while (maxlen)
	
	if (state==uri){
		if (pr->uri && (pr->uri_l==0 || *(pr->uri)=='0')){
			pr->uri=NULL;
			pr->uri_l=0;
		}
		return true;
	} 
	
	if (state==host){
		pr->host_l=(url-pr->host);
		return true;
	} 
	
	if (state==user_or_host){
		pr->host=user_or_host_start;
		pr->host_l=(url-pr->host);
		return true;
	} 
	
	if (state==port||state==pass_or_port){
		int len=url-pass_or_port_start;
		if (len>5 || len<1) return false;
		char port_chars[6];
		memcpy(port_chars,pass_or_port_start,len);
		port_chars[len]=0;
		unsigned long int res;
		if (!trystrtoul(&res,pass_or_port_start,NULL)){
			return false;
		} else {
			if (res>0  && res<=65535) pr->port=(unsigned) res;
			else pr->port=-2;
		}
		return true;
	};
	
	return false;
}

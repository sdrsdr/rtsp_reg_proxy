/***************************************************************************
 *   Copyright (C) 2007 by Stoian Ivanov                                   *
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

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <errno.h>

#include "epollio.h"


//init in preallocate memory
void epollio_init(epollio_t *ep,int waittimeout,epollio_handleevent_t handle){
	memset(ep,0,sizeof(epollio_t));
	ep->waittimeout=waittimeout; 
	ep->handle=handle;
	ep->epollfd=epoll_create(2);
	
}

//allocate and init 
epollio_t *epollio_alloc(int waittimeout,epollio_handleevent_t handle){
	epollio_t *ep=(epollio_t *)malloc(sizeof(epollio_t));
	if (!ep) return NULL;
	epollio_init(ep,waittimeout,handle);
	return ep;
}

bool epollio_opfh_inepfh (int epfd,int fd,int op,epoll_data_t *epdata,uint32_t events){
	struct epoll_event epev;
	memcpy(&epev.data,epdata,sizeof(epoll_data_t));
	epev.events=events;
	return epoll_ctl(epfd,op, fd ,&epev)==0;
}


bool epollio_run (epollio_t *ep){
	struct epoll_event epev[EPOLLIO_WBUFELCNT], *cev;
	while (true) {
		if (ep->prewait) {
			if (ep->prewait(ep)) break;
		}
		errno=0;
		int cnt=epoll_wait(ep->epollfd, epev, EPOLLIO_WBUFELCNT, ep->waittimeout);
		if (cnt==0) {
			if (ep->timeout && ep->timeout(ep)) break;
			if (ep->postwait && ep->postwait(ep)) break;
		}  else if (cnt<0) {
			if (errno==EINTR) {
				if (ep->postwait && ep->postwait(ep)) break;
				continue;
			} else return false;
		}
		if (ep->postwait && ep->postwait(ep)) break;
		
		bool brk=false;
		for (cev=epev; cnt>0; cnt--,cev++){
			if ( (brk=ep->handle(ep,&cev->data,cev->events))) break;
		}
		if (brk) break;
	}
	return true;
}


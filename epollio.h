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

#ifndef epollio_h_SDR_sjdy2213g8ga_included
#define epollio_h_SDR_sjdy2213g8ga_included
#include <stdbool.h>
#include <stdint.h>
#include <sys/epoll.h>

typedef struct epollio_tag epollio_t;
//all callbacks should return true to stop the loop
typedef bool (*epollio_tick_t) (epollio_t *ep) ;
typedef bool (*epollio_timeout_t) (epollio_t *ep) ;
typedef bool (*epollio_handleevent_t) (epollio_t *ep, epoll_data_t epdata, uint32_t events) ;

struct epollio_tag {
	int epollfd;
	int waittimeout;
	epollio_tick_t prewait;
	epollio_tick_t postwait;
	epollio_timeout_t timeout;
	epollio_handleevent_t handle;
};

//init in preallocate memory
void epollio_init(epollio_t *ep,int waittimeout,epollio_handleevent_t handle);

//allocate and init 
epollio_t *epollio_alloc(int waittimeout,epollio_handleevent_t handle);

bool epollio_opfh_inepfh (int epfd,int fd,int op,epoll_data_t *epdata,uint32_t events);

static inline bool epollio_addfh (int epfd,int fd, epoll_data_t* epdata, uint32_t events) { return epollio_opfh_inepfh(epfd,fd,EPOLL_CTL_ADD,epdata,events);};
static inline bool epollio_add (epollio_t *ep,int fd, epoll_data_t* epdata, uint32_t events) { return epollio_opfh_inepfh(ep->epollfd,fd,EPOLL_CTL_ADD,epdata,events);};
static inline bool epollio_modfh (int epfd,int fd, epoll_data_t* epdata, uint32_t events) { return epollio_opfh_inepfh(epfd,fd,EPOLL_CTL_MOD,epdata,events);};
static inline bool epollio_mod (epollio_t *ep,int fd, epoll_data_t* epdata, uint32_t events) { return epollio_opfh_inepfh(ep->epollfd,fd,EPOLL_CTL_MOD,epdata,events);};
static inline bool epollio_delfh (int epfd,int fd, epoll_data_t* epdata, uint32_t events) { return epollio_opfh_inepfh(epfd,fd,EPOLL_CTL_DEL,epdata,events);};
static inline bool epollio_del (epollio_t *ep,int fd, epoll_data_t* epdata, uint32_t events) { return epollio_opfh_inepfh(ep->epollfd,fd,EPOLL_CTL_DEL,epdata,events);};

bool epollio_run (epollio_t *ep);

#endif
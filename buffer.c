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


#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "buffer.h"

buffer_t *buffer_setupinblock(void *mem,unsigned totmemsz){
	if (totmemsz<sizeof(buffer_t)) return NULL;
	buffer_t *b=(buffer_t *)mem;
	b->datablock_size=totmemsz-sizeof(buffer_t);
	b->datablock_start=b->post_header_memory;
	
	b->freestart=b->datablock_start;
	b->datastart=b->datablock_start;
	return b;
}


buffer_t *buffer_malloc(unsigned datablock_size) {
	void *mem=malloc(datablock_size+sizeof(buffer_t));
	if (!mem) return NULL;
	return buffer_setupinblock(mem,datablock_size+sizeof(buffer_t));
}

void buffer_linkto(buffer_t *b,void *mem,unsigned totmemsz){
	b->datablock_size=totmemsz;
	b->datablock_start=(char *)mem;
	
	b->freestart=b->datablock_start;
	b->datastart=b->datablock_start;
}



void buffer_compact(buffer_t*b){
	if (b->datastart==b->datablock_start) return;
	memmove(b->datablock_start,b->datastart,buffer_datalen(b)); 
	b->freestart-=(b->datastart-b->datablock_start); 
	b->datastart=b->datablock_start; 
}


buffer_iostatus_t buffer_writeout(buffer_t*b,int fd){
	unsigned tosend=buffer_datalen(b);
	if (tosend) {
		while (tosend) {
			errno=0;
			int written= write(fd,b->datastart,tosend);
			if (written<0) {
				if (errno==EINTR) continue; 
				if (errno==EAGAIN || errno==EWOULDBLOCK || errno==EWOULDBLOCK) break;
				return rw_ioerr;
			} else if (written==0) break;
			tosend-=written;
			b->datastart+=written;
		}
		if (b->datastart==b->freestart) { //move to front
			b->datastart=b->datablock_start;
			b->freestart=b->datablock_start;
		}
	}
	if (tosend) return w_someleft;
	else return w_allwritten;
}

buffer_iostatus_t buffer_readin(buffer_t*b,int fd){
	unsigned maxrcv=buffer_freelen(b);
	
	while (maxrcv) {
		errno=0;
		int bread=read(fd,b->freestart,maxrcv);
		if (bread<0) {
			if (errno==EINTR) continue; 
			if (errno==EAGAIN || errno==EWOULDBLOCK || errno==EWOULDBLOCK) break;
			return rw_ioerr;
		} else if (bread==0) break;
		maxrcv-=bread;
		b->freestart+=bread;
	}
	
	if (maxrcv==0) return r_overflow;
	else return r_nomore;
}

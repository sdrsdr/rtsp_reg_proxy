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
#include "buffer.h"

buffer_t *buffer_setupinblock(void *mem,unsigned totmemsz){
	if (totmemsz<sizeof(buffer_t)) return NULL;
	buffer_t *b=(buffer_t *)mem;
	b->datablock_size=totmemsz-sizeof(buffer_t);
	b->datablock_start=b->data;
	
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

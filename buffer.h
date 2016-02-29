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


#ifndef buffer_h_SDR_g9s6a4fge34_included
#define buffer_h_SDR_g9s6a4fge34_included

typedef struct {
	char *datablock_start;//associated data memory 
	unsigned datablock_size; //size of associated data memory 
	
	char *datastart; //start of the data in the data memory
	char *freestart; // end of data in  in data memory
	
	char post_header_memory[0];// offset at te end of the header
} buffer_t;

//initialize start of mem  as buffer_t  containing the rest of it
buffer_t *buffer_setupinblock(void *mem,unsigned totmemsz);

//malloc datablock_size+sizeof(buffer_t) and buffer_setupinblock
buffer_t *buffer_malloc(unsigned datablock_size); 

//setup b to use unrelated memory at mem
void buffer_linkto(buffer_t *b,void *mem,unsigned totmemsz);

static inline unsigned buffer_datalen (buffer_t*b) {return (unsigned)(b->freestart-b->datastart);};
static inline unsigned buffer_freelen (buffer_t*b) {return b->datablock_size - (unsigned)(b->freestart-b->datablock_start);};
static inline unsigned buffer_freemaxlen (buffer_t*b) {return b->datablock_size - (unsigned)(b->freestart-b->datastart);}; //b->datablock_size-buffer_datalen(b)
static inline void buffer_reset (buffer_t*b) {b->freestart=b->datastart=b->datablock_start;}; 

//move data at the start of the data memory
void buffer_compact(buffer_t*b);

typedef enum  {
	rw_ioerr=0,w_allwritten,w_someleft,r_nomore,r_overflow
} buffer_iostatus_t;

buffer_iostatus_t buffer_writeout(buffer_t*b,int fd,unsigned *written_/*=NULL*/);
buffer_iostatus_t buffer_readin(buffer_t*b,int fd,unsigned *dl/*=NULL*/);


#endif

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


#ifndef inettools_h_SDR_3gg234msd2aa_included
#define inettools_h_SDR_3gg234msd2aa_included

#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <stdbool.h>

#define udpsocket()  socket(AF_INET, SOCK_DGRAM|SOCK_CLOEXEC, 0)
#define udpsocket_nb()  socket(AF_INET, SOCK_DGRAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0)
#define tcpsocket()  socket(AF_INET, SOCK_STREAM|SOCK_CLOEXEC, 0)
#define tcpsocket_nb()  socket(AF_INET, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0)

#define bindsock(sok,addr) bind(sok,(struct sockaddr *)addr,sizeof(struct sockaddr_in))

bool bindaddrhl (struct sockaddr_in *sadr,const char *host,unsigned host_l , uint16_t port);
bool bindaddr (struct sockaddr_in *sadr,const char *host, uint16_t port);

int canreuseaddr (int sock);
void setnodelay(int sock, bool nodelay);

//return true on success
bool connsockhl(int sok, const char *host, unsigned host_l, uint16_t port);
static inline bool connsock(int sok, const char *host, uint16_t port) {return  connsockhl(sok, host,0, port);};

#endif

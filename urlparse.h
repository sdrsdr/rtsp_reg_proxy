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


#ifndef urlparse_h_SDR_fc4fg434mdd2a_included
#define urlparse_h_SDR_fc4fg434mdd2a_included
#include <stdint.h>
#include <stdbool.h>

typedef struct {
	const char *proto;
	unsigned proto_l;
	const char *user;
	unsigned user_l;
	const char *pass;
	unsigned pass_l;
	const char *host;
	unsigned host_l;
	int32_t port; //0 unset, -1 err parsing, -2 range err,  >0 actual port
	const char *uri;
	unsigned uri_l;
} parsedurl_t;

//return false on error
bool parseurlml(const char *url, parsedurl_t *pr,unsigned maxlen);
static inline bool parseurl(const char *url, parsedurl_t *pr) {return parseurlml(url,pr,4096);};
#endif

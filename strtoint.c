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

#include <errno.h>
#include <stdlib.h>
#include "strtoint.h"

bool trystrtoulb(unsigned long *res, const char *nptr,char **endptr, int base){
	errno=0;
	*res=strtoul(nptr,endptr,base);
	return errno==0;
}

bool trystrtolb(long *res, const char *nptr,char **endptr, int base){
	errno=0;
	*res=strtol(nptr,endptr,base);
	return errno==0;
}

bool trystrtod (double *res, const char *nptr,char **endptr){
	errno=0;
	*res=strtod(nptr,endptr);
	return errno==0;
}

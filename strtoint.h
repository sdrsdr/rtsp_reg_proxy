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


#ifndef strtoint_h_SDR_9860lhlf429ck4sdffa9g49m2a_included
#define strtoint_h_SDR_9860lhlf429ck4sdffa9g49m2a_included
#include <stdbool.h>

//based on strtoul; return true if conversion was error-free (store result in *res)
bool trystrtoulb(unsigned long *res, const char *nptr,char **endptr, int base);
static inline bool trystrtoul(unsigned long *res, const char *nptr,char **endptr) {return trystrtoulb(res, nptr,endptr, 10);};

//based on strtol; return true if conversion was error-free (store result in *res)
bool trystrtolb(long *res, const char *nptr,char **endptr, int base);
static inline bool trystrtol(long *res, const char *nptr,char **endptr) {return trystrtolb(res, nptr,endptr,10);};

//based on strtod; return true if conversion was error-free (store result in *res)
bool trystrtod (double *res, const char *nptr,char **endptr);

#endif

/*
   Python wrappers for magic functions.

   Copyright (C) Brett Funderburg, Deepfile Corp. Austin, TX, US 2003

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _PY_MAGIC_H
#define _PY_MAGIC_H

typedef struct {
    PyObject_HEAD
    magic_t cookie;
} magic_cookie_hnd;

#endif /* _PY_MAGIC_H */

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

#include <Python.h>
#include <magic.h>
#include "py_magic.h"

/* Exceptions raised by this module */

PyObject* magic_error_obj;

/* Create a new magic_cookie_hnd object */
PyObject* new_magic_cookie_handle(magic_t cookie)
{
    magic_cookie_hnd* mch;

    mch = PyObject_New(magic_cookie_hnd, &magic_cookie_type);

    mch->cookie = cookie;

    return (PyObject*)mch;
}

static char _magic_open__doc__[] =
"Returns a magic cookie on success and None on failure.\n";
static PyObject* py_magic_open(PyObject* self, PyObject* args)
{
    int flags = 0;
    magic_t cookie;

    if(!PyArg_ParseTuple(args, "i", &flags))
        return NULL;

    if(!(cookie = magic_open(flags))) {
        PyErr_SetString(magic_error_obj, "failure initializing magic
cookie");
        return NULL;
    }

    return new_magic_cookie_handle(cookie);
}

static char _magic_close__doc__[] =
"Closes the magic database and deallocates any resources used.\n";
static PyObject* py_magic_close(PyObject* self, PyObject* args)
{
    magic_cookie_hnd* hnd = (magic_cookie_hnd*)self;

    magic_close(hnd->cookie);

    Py_INCREF(Py_None);
    return Py_None;
}

static char _magic_error__doc__[] =
"Returns a textual explanation of the last error or None \
 if there was no error.\n";
static PyObject* py_magic_error(PyObject* self, PyObject* args)
{
    magic_cookie_hnd* hnd = (magic_cookie_hnd*)self;
    const char* message = NULL;
    PyObject* result = NULL;

    message = magic_error(hnd->cookie);

    if(message != NULL)
        result = PyString_FromString(message);
    else {
        Py_INCREF(Py_None);
        result = Py_None;
    }

    return result;
}

static char _magic_file__doc__[] =
"Returns a textual description of the contents of the argument passed \
 as a filename or None if an error occurred.\n";
static PyObject* py_magic_file(PyObject* self, PyObject* args)
{
    magic_cookie_hnd* hnd = (magic_cookie_hnd*)self;
    char* filename = NULL;
    const char* message = NULL;
    PyObject* result = NULL;

    if(!(PyArg_ParseTuple(args, "s", &filename)))
        return NULL;

    message = magic_file(hnd->cookie, filename);

    if(message != NULL)
        result = PyString_FromString(message);
    else
        PyErr_SetString(PyExc_RuntimeError,
                "failure determining file type");

    return result;
}

static char _magic_buffer__doc__[] =
"Returns a textual description of the contents of the argument passed \
 as a buffer or None if an error occurred.\n";
static PyObject* py_magic_buffer(PyObject* self, PyObject* args)
{
    magic_cookie_hnd* hnd = (magic_cookie_hnd*)self;
    void* buffer = NULL;
    int buffer_length = 0;
    const char* message = NULL;
    PyObject* result = NULL;

    if(!(PyArg_ParseTuple(args, "s#", (char**)&buffer, &buffer_length)))
        return NULL;

    message = magic_buffer(hnd->cookie, buffer, buffer_length);

    if(message != NULL)
        result = PyString_FromString(message);
    else
        PyErr_SetString(PyExc_RuntimeError,
                "failure determining buffer type");

    return result;
}

static char _magic_setflags__doc__[] =
"Set flags on the cookie object.\n \
 Returns -1 on systems that don't support utime(2) or utimes(2) \
 when MAGIC_PRESERVE_ATIME is set.\n";
static PyObject* py_magic_setflags(PyObject* self, PyObject* args)
{
    magic_cookie_hnd* hnd = (magic_cookie_hnd*)self;
    int flags;
    int result;

    if(!(PyArg_ParseTuple(args, "i", &flags)))
        return NULL;

    result = magic_setflags(hnd->cookie, flags);

    return PyInt_FromLong(result);
}

static char _magic_check__doc__[] =
"Check the validity of entries in the colon separated list of database
files \
 passed as argument or the default database file if no argument.\n \
 Returns 0 on success and -1 on failure.\n";
static PyObject* py_magic_check(PyObject* self, PyObject* args)
{
    magic_cookie_hnd* hnd = (magic_cookie_hnd*)self;
    char* filename = NULL;
    int result;

    if(!(PyArg_ParseTuple(args, "|s", &filename)))
        return NULL;

    result = magic_check(hnd->cookie, filename);

    return PyInt_FromLong(result);
}

static char _magic_compile__doc__[] =
"Compile entries in the colon separated list of database files \
 passed as argument or the default database file if no argument.\n \
 Returns 0 on success and -1 on failure.\n \
 The compiled files created are named from the basename(1) of each file \
 argument with \".mgc\" appended to it.\n";
static PyObject* py_magic_compile(PyObject* self, PyObject* args)
{
    magic_cookie_hnd* hnd = (magic_cookie_hnd*)self;
    char* filename = NULL;
    int result;

    if(!(PyArg_ParseTuple(args, "|s", &filename)))
        return NULL;

    result = magic_compile(hnd->cookie, filename);

    return PyInt_FromLong(result);
}

static char _magic_load__doc__[] =
"Must be used to load entries in the colon separated list of database files \
 passed as argument or the default database file if no argument before \
 any magic queries can be performed.\n \
 Returns 0 on success and -1 on failure.\n";
static PyObject* py_magic_load(PyObject* self, PyObject* args)
{
    magic_cookie_hnd* hnd = (magic_cookie_hnd*)self;
    char* filename = NULL;
    int result;

    if(!(PyArg_ParseTuple(args, "|s", &filename)))
        return NULL;

    result = magic_load(hnd->cookie, filename);

    return PyInt_FromLong(result);
}

/* object methods */

static PyMethodDef magic_cookie_hnd_methods[] = {
    { "close", (PyCFunction)py_magic_close,
      METH_NOARGS, _magic_close__doc__ },
    { "error", (PyCFunction)py_magic_error,
      METH_NOARGS, _magic_error__doc__ },
    { "file", (PyCFunction)py_magic_file,
      METH_VARARGS, _magic_file__doc__ },
    { "buffer", (PyCFunction)py_magic_buffer,
      METH_VARARGS, _magic_buffer__doc__ },
    { "setflags", (PyCFunction)py_magic_setflags,
      METH_VARARGS, _magic_setflags__doc__ },
    { "check", (PyCFunction)py_magic_check,
      METH_VARARGS, _magic_check__doc__ },
    { "compile", (PyCFunction)py_magic_compile,
      METH_VARARGS, _magic_compile__doc__ },
    { "load", (PyCFunction)py_magic_load,
      METH_VARARGS, _magic_load__doc__ },
    { NULL, NULL }
};

/* module level methods */

static PyMethodDef magic_methods[] = {
    { "open", (PyCFunction)py_magic_open,
      METH_VARARGS, _magic_open__doc__ },
    { NULL, NULL }
};

static void py_magic_dealloc(PyObject* self)
{
    PyObject_Del(self);
}

static PyObject* py_magic_getattr(PyObject* self, char* attrname)
{
    return Py_FindMethod(magic_cookie_hnd_methods, self, attrname);
}

PyTypeObject magic_cookie_type = {
    PyObject_HEAD_INIT(NULL)
    0,
    "Magic cookie",
    sizeof(magic_cookie_hnd),
    0,
    py_magic_dealloc, /* tp_dealloc */
    0,                /* tp_print */
    py_magic_getattr, /* tp_getattr */
    0,                /* tp_setattr */
    0,                /* tp_compare */
    0,                /* tp_repr */
    0,                /* tp_as_number */
    0,                /* tp_as_sequence */
    0,                /* tp_as_mapping */
    0,                /* tp_hash */
};

/* Initialize constants */

static struct const_vals {
    const char* const name;
    unsigned int value;
} module_const_vals[] = {
    { "MAGIC_NONE", MAGIC_NONE },
    { "MAGIC_DEBUG", MAGIC_DEBUG },
    { "MAGIC_SYMLINK", MAGIC_SYMLINK },
    { "MAGIC_COMPRESS", MAGIC_COMPRESS },
    { "MAGIC_DEVICES", MAGIC_DEVICES },
    { "MAGIC_MIME", MAGIC_MIME },
    { "MAGIC_CONTINUE", MAGIC_CONTINUE },
    { "MAGIC_CHECK", MAGIC_CHECK },
    { "MAGIC_PRESERVE_ATIME", MAGIC_PRESERVE_ATIME },
    { NULL }
};

static void const_init(PyObject* dict)
{
    struct const_vals* tmp;
    PyObject *obj;

    for(tmp = module_const_vals; tmp->name; ++tmp) {
        obj = PyInt_FromLong(tmp->value);
        PyDict_SetItemString(dict, tmp->name, obj);
        Py_DECREF(obj);
    }
}

/*
 * Module initialization
 */

void initmagic(void)
{
    PyObject* module;
    PyObject* dict;

    /* Initialize module */

    module = Py_InitModule("magic", magic_methods);
    dict = PyModule_GetDict(module);

    magic_error_obj = PyErr_NewException("magic.error", NULL, NULL);
    PyDict_SetItemString(dict, "error", magic_error_obj);

    magic_cookie_type.ob_type = &PyType_Type;

    /* Initialize constants */

    const_init(dict);

    if(PyErr_Occurred())
        Py_FatalError("can't initialize module magic");
}

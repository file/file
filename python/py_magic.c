/*
   Python wrappers for magic functions.

   Copyright (C) Brett Funderburg, Deepfile Corp. Austin, TX, US 2003

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice immediately at the beginning of the file, without modification,
      this list of conditions, and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. The name of the author may not be used to endorse or promote products
      derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
   OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
   SUCH DAMAGE.
*/

/* The initialisation code was rewritten for Python 2 & 3 compatibility, based on that at
   http://wiki.python.org/moin/PortingExtensionModulesToPy3k */

#include <Python.h>
#include <magic.h>

typedef struct {
	PyObject_HEAD
	magic_t cookie;
} magic_cookie_hnd;

static int py_magic_init(PyObject *self, PyObject *args, PyObject *kwds);
static void py_magic_dealloc(PyObject *self);
static PyObject *py_magic_open(PyObject *self, PyObject *args);
static PyObject *py_magic_close(PyObject *self, PyObject *args);
static PyObject *py_magic_error(PyObject *self, PyObject *args);
static PyObject *py_magic_file(PyObject *self, PyObject *args);
static PyObject *py_magic_buffer(PyObject *self, PyObject *args);
static PyObject *py_magic_setflags(PyObject *self, PyObject *args);
static PyObject *py_magic_check(PyObject *self, PyObject *args);
static PyObject *py_magic_compile(PyObject *self, PyObject *args);
static PyObject *py_magic_load(PyObject *self, PyObject *args);
static PyObject *py_magic_errno(PyObject *self, PyObject *args);


/* Documentation */
static char _magic_close__doc__[] =
"Closes the magic database and deallocates any resources used.\n";
static char _magic_open__doc__[] =
"Returns a magic cookie on success and None on failure.\n";
static char _magic_error__doc__[] =
"Returns a textual explanation of the last error or None \
 if there was no error.\n";
static char _magic_errno__doc__[] =
"Returns a numeric error code. If return value is 0, an internal \
 magic error occurred. If return value is non-zero, the value is \
 an OS error code. Use the errno module or os.strerror() can be used \
 to provide detailed error information.\n";
static char _magic_file__doc__[] =
"Returns a textual description of the contents of the argument passed \
 as a filename or None if an error occurred and the MAGIC_ERROR flag \
 is set. A call to errno() will return the numeric error code.\n";
static char _magic_buffer__doc__[] =
"Returns a textual description of the contents of the argument passed \
 as a buffer or None if an error occurred and the MAGIC_ERROR flag \
 is set. A call to errno() will return the numeric error code.\n";
static char _magic_setflags__doc__[] =
"Set flags on the cookie object.\n \
 Returns -1 on systems that don't support utime(2) or utimes(2) \
 when MAGIC_PRESERVE_ATIME is set.\n";
static char _magic_check__doc__[] =
"Check the validity of entries in the colon separated list of \
 database files passed as argument or the default database file \
 if no argument.\n Returns 0 on success and -1 on failure.\n";
static char _magic_compile__doc__[] =
"Compile entries in the colon separated list of database files \
 passed as argument or the default database file if no argument.\n \
 Returns 0 on success and -1 on failure.\n \
 The compiled files created are named from the basename(1) of each file \
 argument with \".mgc\" appended to it.\n";
static char _magic_load__doc__[] =
"Must be called to load entries in the colon separated list of database files \
 passed as argument or the default database file if no argument before \
 any magic queries can be performed.\n \
 Returns 0 on success and -1 on failure.\n";

/* object methods */

static PyMethodDef magic_cookie_hnd_methods[] = {
	{ "close",    py_magic_close,    METH_NOARGS,  _magic_close__doc__    },
	{ "error",    py_magic_error,    METH_NOARGS,  _magic_error__doc__    },
	{ "file",     py_magic_file,     METH_VARARGS, _magic_file__doc__     },
	{ "buffer",   py_magic_buffer,   METH_VARARGS, _magic_buffer__doc__   },
	{ "setflags", py_magic_setflags, METH_VARARGS, _magic_setflags__doc__ },
	{ "check",    py_magic_check,    METH_VARARGS, _magic_check__doc__    },
	{ "compile",  py_magic_compile,  METH_VARARGS, _magic_compile__doc__  },
	{ "load",     py_magic_load,     METH_VARARGS, _magic_load__doc__     },
	{ "errno",    py_magic_errno,    METH_NOARGS,  _magic_errno__doc__    },
	{ NULL,       NULL,              0,            NULL		      }
};

/* module level methods */

struct module_state {
	PyObject *error;
};

#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif

static PyObject *
error_out(PyObject *m) {
	struct module_state *st = GETSTATE(m);
	PyErr_SetString(st->error, "something bad happened");
	return NULL;
}

static PyMethodDef magic_methods[] = {
	{ "open",     py_magic_open,     METH_VARARGS, _magic_open__doc__     },
	{ "error_out", error_out, 	 METH_NOARGS,  NULL		      },
	{ NULL,       NULL,              0,            NULL		      }
};

/* Initialize constants */

static struct const_vals {
	    const char *name;
	    unsigned int value;
} module_const_vals[] = {
	{ "MAGIC_NONE", MAGIC_NONE },
	{ "MAGIC_DEBUG", MAGIC_DEBUG },
	{ "MAGIC_SYMLINK", MAGIC_SYMLINK },
	{ "MAGIC_COMPRESS", MAGIC_COMPRESS },
	{ "MAGIC_DEVICES", MAGIC_DEVICES },
	{ "MAGIC_MIME_TYPE", MAGIC_MIME_TYPE },
	{ "MAGIC_CONTINUE", MAGIC_CONTINUE },
	{ "MAGIC_CHECK", MAGIC_CHECK },
	{ "MAGIC_PRESERVE_ATIME", MAGIC_PRESERVE_ATIME },
	{ "MAGIC_RAW", MAGIC_RAW},
	{ "MAGIC_ERROR", MAGIC_ERROR},
	{ "MAGIC_MIME_ENCODING", MAGIC_MIME_ENCODING },
	{ "MAGIC_MIME", MAGIC_MIME },
	{ "MAGIC_APPLE", MAGIC_APPLE },
	{ "MAGIC_NO_CHECK_COMPRESS", MAGIC_NO_CHECK_COMPRESS },
	{ "MAGIC_NO_CHECK_TAR", MAGIC_NO_CHECK_TAR },
	{ "MAGIC_NO_CHECK_SOFT", MAGIC_NO_CHECK_SOFT },
	{ "MAGIC_NO_CHECK_APPTYPE", MAGIC_NO_CHECK_APPTYPE },
	{ "MAGIC_NO_CHECK_ELF", MAGIC_NO_CHECK_ELF },
	{ "MAGIC_NO_CHECK_TEXT", MAGIC_NO_CHECK_TEXT },
	{ "MAGIC_NO_CHECK_CDF", MAGIC_NO_CHECK_CDF },
	{ "MAGIC_NO_CHECK_TOKENS", MAGIC_NO_CHECK_TOKENS },
	{ "MAGIC_NO_CHECK_ENCODING", MAGIC_NO_CHECK_ENCODING },
	{ NULL, 0 }
};

// The meta-type for PyQt classes.
static PyTypeObject magic_cookie_type = {
#if PY_VERSION_HEX >= 0x02070000
	PyVarObject_HEAD_INIT(0,0)
#else
	PyObject_HEAD_INIT(NULL)
	0,
#endif
	"Magic cookie",
	sizeof (magic_cookie_hnd), /* tp_basicsize */
	0,                      /* tp_itemsize */
	py_magic_dealloc,       /* tp_dealloc */
	0,                      /* tp_print */
	0,                      /* tp_getattr */
	0,                      /* tp_setattr */
	0,                      /* tp_compare */
	0,                      /* tp_repr */
	0,                      /* tp_as_number */
	0,                      /* tp_as_sequence */
	0,                      /* tp_as_mapping */
	0,                      /* tp_hash */
	0,                      /* tp_call */
	0,                      /* tp_str */
	PyObject_GenericGetAttr,/* tp_getattro */
	0,                      /* tp_setattro */
	0,                      /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	"Magic objects",        /* tp_doc */
	0,                      /* tp_traverse */
	0,                      /* tp_clear */
	0,                      /* tp_richcompare */
	0,                      /* tp_weaklistoffset */
	0,                      /* tp_iter */
	0,                      /* tp_iternext */
	magic_cookie_hnd_methods, /* tp_methods */
	/*0,                       tp_methods */
	0,                      /* tp_members */
	0,                      /* tp_getset */
	0,                      /* tp_base */
	0,                      /* tp_dict */
	0,                      /* tp_descr_get */
	0,                      /* tp_descr_set */
	0,                      /* tp_dictoffset */
	py_magic_init,          /* tp_init */
	0,                      /* tp_alloc */
	0,                      /* tp_new */
	0,                      /* tp_free */
	0,                      /* tp_is_gc */
	0,                      /* tp_bases */
	0,                      /* tp_mro */
	0,                      /* tp_cache */
	0,                      /* tp_subclasses */
	0,                      /* tp_weaklist */
	0,                      /* tp_del */
#if PY_VERSION_HEX >= 0x02070000
	0,                      /* tp_version_tag */
#endif
};

#if PY_VERSION_HEX < 0x02060000
#define PyUnicode_FromString(m)	PyString_FromString(m)
#endif

/* Exceptions raised by this module */

static PyObject *magic_error_obj;

/* Create a new magic_cookie_hnd object */
PyObject *
new_magic_cookie_handle(magic_t cookie)
{
	magic_cookie_hnd *mch = PyObject_New(magic_cookie_hnd,
	    &magic_cookie_type);
	mch->cookie = cookie;
	return (PyObject *)mch;
}

static PyObject *
py_magic_open(PyObject *self, PyObject *args)
{
	int flags;
	magic_t cookie;

	if(!PyArg_ParseTuple(args, "i", &flags))
		return NULL;

	if((cookie = magic_open(flags)) == NULL) {
		PyErr_SetString(magic_error_obj,
		    "failure initializing magic cookie");
		return NULL;
	}

	return new_magic_cookie_handle(cookie);
}

static PyObject *
py_magic_close(PyObject *self, PyObject *args)
{
	magic_cookie_hnd *hnd = (magic_cookie_hnd *)self;
	magic_close(hnd->cookie);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
py_magic_error(PyObject *self, PyObject *args)
{
	magic_cookie_hnd *hnd = (magic_cookie_hnd *)self;
	const char *message = magic_error(hnd->cookie);

	if (message)
		return PyUnicode_FromString(message);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
py_magic_errno(PyObject *self, PyObject *args)
{
	magic_cookie_hnd *hnd = (magic_cookie_hnd*)self;
	return PyLong_FromLong(magic_errno(hnd->cookie));
}

static PyObject *
py_magic_file(PyObject *self, PyObject *args)
{
	magic_cookie_hnd *hnd = (magic_cookie_hnd *)self;
	char *filename;
	const char *msg;

	if (!PyArg_ParseTuple(args, "s", &filename))
		return NULL;

	if ((msg = magic_file(hnd->cookie, filename)) != NULL)
		return PyUnicode_FromString(msg);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
py_magic_buffer(PyObject *self, PyObject *args)
{
	magic_cookie_hnd *hnd = (magic_cookie_hnd *)self;
	void *buf;
	int buflen;
	const char *msg;

	if (!PyArg_ParseTuple(args, "s#", (char **)&buf, &buflen))
		return NULL;

	if ((msg = magic_buffer(hnd->cookie, buf, buflen)) != NULL)
		return PyUnicode_FromString(msg);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
py_magic_setflags(PyObject* self, PyObject* args)
{
	magic_cookie_hnd *hnd = (magic_cookie_hnd *)self;
	int flags;

	if (!PyArg_ParseTuple(args, "i", &flags))
		return 0;

	return PyLong_FromLong(magic_setflags(hnd->cookie, flags));
}

static PyObject *
py_magic_check(PyObject *self, PyObject *args)
{
	magic_cookie_hnd *hnd = (magic_cookie_hnd *)self;
	char *filename = NULL;

	if (!PyArg_ParseTuple(args, "|s", &filename))
		return NULL;

	return PyLong_FromLong(magic_check(hnd->cookie, filename));
}

static PyObject *
py_magic_compile(PyObject *self, PyObject *args)
{
	magic_cookie_hnd *hnd = (magic_cookie_hnd *)self;
	char *filename = NULL;

	if (!PyArg_ParseTuple(args, "|s", &filename))
		return NULL;

	return PyLong_FromLong(magic_compile(hnd->cookie, filename));
}

static PyObject *
py_magic_load(PyObject *self, PyObject *args)
{
	magic_cookie_hnd *hnd = (magic_cookie_hnd *)self;
	char *filename = NULL;

	if (!PyArg_ParseTuple(args, "|s", &filename))
		return NULL;

	return PyLong_FromLong(magic_load(hnd->cookie, filename));
}

static void
py_magic_dealloc(PyObject *self)
{
	PyObject_Del(self);
}

static int
py_magic_init(PyObject *self, PyObject *args, PyObject *kwds)
{
	int rc;

	magic_cookie_type.tp_new = PyType_GenericNew;
	rc = PyType_Ready(&magic_cookie_type);

	if (rc < 0) {
		Py_FatalError("magic: Failed to initialize magic_cookie type");
		return rc;
	}

	Py_INCREF(&magic_cookie_type);

	return 0;
}

static void
const_init(PyObject *dict)
{
	struct const_vals *v = module_const_vals;

	for (; v->name; ++v) {
		PyObject *obj = PyLong_FromLong(v->value);
		PyDict_SetItemString(dict, v->name, obj);
		Py_DECREF(obj);
	}
}

/*
 * Module initialization
 */

#if PY_MAJOR_VERSION >= 3

static int magic_traverse(PyObject *m, visitproc visit, void *arg) {
	Py_VISIT(GETSTATE(m)->error);
	return 0;
}

static int magic_clear(PyObject *m) {
	Py_CLEAR(GETSTATE(m)->error);
	return 0;
}

static struct PyModuleDef moduledef = {
	PyModuleDef_HEAD_INIT,
	"magic",
	NULL,
	sizeof(struct module_state),
	magic_methods,
	NULL,
	magic_traverse,
	magic_clear,
	NULL
};

#define INITERROR return NULL

PyObject *
PyInit_magic(void)

#else
#define INITERROR return

void
initmagic(void)
#endif
{
	PyObject *dict;
#if PY_MAJOR_VERSION >= 3
	PyObject *module = PyModule_Create(&moduledef);
#else
	PyObject *module = Py_InitModule4("magic", magic_methods,
					  "File magic module", (PyObject*)0, PYTHON_API_VERSION);
#endif

	if (module == NULL)
		INITERROR;
	struct module_state *st = GETSTATE(module);

	dict = PyModule_GetDict(module);
	if (dict == NULL)
		Py_FatalError("dict error");

	st->error = PyErr_NewException("magic.Error", NULL, NULL);
	if (st->error == NULL) {
		Py_DECREF(module);
		INITERROR;
	}

	magic_error_obj = PyErr_NewException("magic.error", 0, 0);
	PyDict_SetItemString(dict, "error", magic_error_obj);

	/* Initialize constants */

	const_init(dict);

	if (PyErr_Occurred())
		Py_FatalError("can't initialize module magic");

#if PY_MAJOR_VERSION >= 3
	return module;
#endif
}

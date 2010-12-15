# Python distutils build script for magic extension
import sys, os
from distutils.core import setup, Extension

p = sys.prefix + '/lib64'
if os.path.exists(sys.prefix + '/lib64'):
    libdir = sys.prefix + '/lib64'
elif os.path.exists(sys.prefix + '/lib'):
    libdir = sys.prefix + '/lib'

magic_module = Extension('magic',
    libraries = ['magic'],
    library_dirs = ['./', '../', '../src', libdir],
    include_dirs = ['./', '../', '../src', sys.prefix + '/include' ],
    sources = ['py_magic.c'])

setup (name = 'Magic file extensions',
    version = '0.1',
    author = 'Brett Funderburg',
    author_email = 'brettf@deepfile.com',
    license = 'BSD',
    description = 'libmagic python bindings',
    ext_modules = [magic_module])

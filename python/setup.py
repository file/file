To build, make a directory called 'python' in 'src' and then follow the
instructions in the README.

Unless something got screwed up in the cut and paste, the code should
compile cleanly and without warnings.

You may need to tinker with the library and include paths specified in the
distutils build script (setup.py) to make things work on your machine.

The following python script should give you an idea of how things work.

-- begin --
import magic
c = magic.open(magic.MAGIC_NONE)
c.load()
c.file("/path/to/some/file")

f = file("/path/to/some/file", "r")
buffer = f.read(4096)
f.close()

c.buffer(buffer)

c.close()

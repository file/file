#! /usr/bin/python

import magic

ms = magic.open(magic.MAGIC_NONE)
ms.load()
tp =  ms.file("/bin/ls")
print (tp)

f = open("/bin/ls", "r")
buffer = f.read(4096)
f.close()

tp = ms.buffer(buffer)
print (tp)

ms.close()

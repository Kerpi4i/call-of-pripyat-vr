"""Read selected local CoP archive entries for diagnostics; never edits archives."""
import ctypes as c
import os
from pathlib import Path
import struct
import sys
root=Path(__file__).resolve().parent.parent
directory=os.add_dll_directory(str(root/'engine/bin/x64/Release'))
codec=c.CDLL(str(root/'build/copvr_lz.dll'))
codec.copvr_unpack.argtypes=[c.c_void_p,c.c_size_t,c.POINTER(c.c_void_p),c.POINTER(c.c_size_t)]
core_path=root/'engine/bin/x64/Release/xrCore.dll'
core=c.CDLL(str(core_path))
import re
name=re.search(rb'\?rtc_decompress[^\0]+',core_path.read_bytes()).group().decode('ascii')
decompress=getattr(core,name)
decompress.argtypes=[c.c_void_p,c.c_size_t,c.c_void_p,c.c_size_t]
decompress.restype=c.c_size_t
archive=Path(sys.argv[1]).read_bytes()
offset=0
while offset+8<=len(archive):
    kind,size=struct.unpack_from('<II',archive,offset); offset+=8
    if kind&0x7fffffff==1:
        data=archive[offset:offset+size]
        if kind&0x80000000:
            out=c.c_void_p(); count=c.c_size_t()
            if not codec.copvr_unpack(data,len(data),c.byref(out),c.byref(count)): raise ValueError('Invalid archive index')
            data=c.string_at(out,count.value)
        break
    offset+=size
else: raise ValueError('Missing archive index')
offset=0; patterns=sys.argv[2:]
while offset<len(data):
    size=struct.unpack_from('<H',data,offset)[0]; offset+=2
    real,compressed,crc=struct.unpack_from('<III',data,offset)
    filename=data[offset+12:offset+size-4].decode('cp1251')
    pointer=struct.unpack_from('<I',data,offset+size-4)[0]; offset+=size
    if not real or filename.endswith(("\\", "/")): continue
    if not any(pattern.lower() in filename.lower() for pattern in patterns): continue
    payload=archive[pointer:pointer+compressed]
    if real!=compressed:
        out=c.create_string_buffer(real)
        if decompress(out,real,payload,len(payload))!=real: raise ValueError(filename)
        payload=out.raw
    inspection=(root/'reports/archive-inspection').resolve()
    destination=(inspection/filename.replace('\\','/')).resolve()
    if not destination.is_relative_to(inspection): raise ValueError('Archive entry outside inspection directory')
    destination.parent.mkdir(parents=True,exist_ok=True)
    destination.write_bytes(payload)
    print(filename)

"""Read exception and module-relative stack candidates from a local minidump.
No process is launched and no environment/settings sections are printed.
"""
import struct
import sys
import zipfile

path = sys.argv[1]
if path.lower().endswith('.zip'):
    with zipfile.ZipFile(path) as archive:
        data = archive.read('crashdump.dmp')
else:
    data = open(path, 'rb').read()

def u32(at): return struct.unpack_from('<I', data, at)[0]
def u64(at): return struct.unpack_from('<Q', data, at)[0]
assert data[:4] == b'MDMP', 'Not a minidump'
streams = {}
for index in range(u32(8)):
    kind, size, rva = struct.unpack_from('<III', data, u32(12) + index * 12)
    streams[kind] = (rva, size)

modules = []
at = streams[4][0]
for index in range(u32(at)):
    item = at + 4 + index * 108
    name = u32(item + 20)
    text = data[name+4:name+4+u32(name)].decode('utf-16le')
    modules.append((u64(item), u32(item+8), text.replace('\\', '/').rsplit('/', 1)[-1]))

def location(address):
    for base, size, name in modules:
        if base <= address < base + size:
            return f'{name}+0x{address-base:x}'
    return None

if 6 in streams:
    at = streams[6][0]
    thread = u32(at)
    print(f'Exception thread={thread} code=0x{u32(at+8):08x} address=0x{u64(at+24):x}')
    for index in range(min(u32(at+32), 15)):
        print(f'  parameter[{index}]=0x{u64(at+40+index*8):x}')
else:
    thread = int(sys.argv[2]) if len(sys.argv)>2 else u32(streams[3][0]+4)
    print(f'Snapshot has no exception record; inspecting thread={thread}')
at = streams[3][0]
for index in range(u32(at)):
    item = at + 4 + index * 48
    if u32(item) != thread: continue
    stack_base, stack_size, stack_rva = u64(item+24), u32(item+32), u32(item+36)
    context = u32(item+44)
    rsp, rip = u64(context+152), u64(context+248)
    print(f'RIP=0x{rip:x} ({location(rip)}) RSP=0x{rsp:x}')
    offset = max(0, rsp-stack_base)
    print('Stack candidates (not an unwound call stack):')
    for delta in range(offset, min(stack_size, offset+2048), 8):
        address = u64(stack_rva+delta)
        symbol = location(address)
        if symbol: print(f'  +0x{delta-offset:03x}: {symbol}')
    break

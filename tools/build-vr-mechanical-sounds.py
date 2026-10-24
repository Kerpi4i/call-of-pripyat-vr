"""Cut local CoP reload recordings into independent VR mechanical actions.

Preserves the original binary X-Ray Vorbis sound metadata; never edits archives.
"""
from pathlib import Path
import json, re, struct, subprocess

ROOT=Path(__file__).resolve().parent.parent
SOURCE=ROOT/'reports/archive-inspection/sounds/weapons'
OUTPUT=ROOT/'game-vr/gamedata/sounds/copvr/mechanical'
OUTPUT.mkdir(parents=True,exist_ok=True)

# Seconds, measured from isolated events in the native recordings. Rack includes
# both rearward and forward clicks where both are present in the recording.
CUTS={
 'groza_reload':[(.31,.55),(1.01,1.75),(2.11,2.65)],
 'pistol_reload':[(.16,.34),(.77,1.15),(1.52,1.88)],
 'beretta_reload':[(.16,.34),(.77,1.15),(1.52,1.88)],
 'g36_reload':[(.62,.85),(1.48,2.09),(2.43,2.828)],
 'l85_reload':[(.48,.81),(1.27,1.47),(1.94,2.354)],
 'fn2000_reload':[(.52,.83),(1.25,1.67),(2.07,2.58)],
 'mp5_reload':[(.36,.59),(1.13,1.82),(2.16,2.471)],
 'sig550_reload':[(.33,.56),(1.07,1.77),(2.10,2.489)],
 'svd_reload':[(.28,.54),(1.53,2.32),(2.60,2.95)],
 'svu_reload':[(.48,.74),(1.32,1.69),(2.07,2.55)],
 'pkm_reload':[(1.79,2.03),(3.60,4.44),(5.42,5.93)],
 'gauss_reload':[(.42,.67),(2.13,2.821),None],
}
RACKS={
 'wpn_bm16_hud':('bm16_reload_lr',4.52,4.76),
 'wpn_toz34_hud':('toz34_reload',3.14,3.44),
 'wpn_rpg7_hud':('rpg7_reload',1.76,2.25),
 'wpn_rg-6_hud':('rg6_reload',1.41,1.882),
 'wpn_spas12_hud':('spas12_close',0,.34),
 'wpn_protecta_hud':('spas12_close',0,.34),
 'wpn_winchester1300_hud':('w1300_close',0,.176),
}

def pages(data):
    offset=0
    while offset<len(data):
        assert data[offset:offset+4]==b'OggS'
        count=data[offset+26];laces=data[offset+27:offset+27+count]
        end=offset+27+count+sum(laces)
        yield data[offset:end]
        offset=end

def comments(data):
    packet=bytearray()
    for page in pages(data):
        offset=27+page[26]
        for size in page[27:offset]:
            packet+=page[offset:offset+size];offset+=size
            if size<255:
                if packet.startswith(b'\x03vorbis'):
                    at=7;length=struct.unpack_from('<I',packet,at)[0];at+=4+length
                    count=struct.unpack_from('<I',packet,at)[0];at+=4;result=[]
                    for _ in range(count):
                        length=struct.unpack_from('<I',packet,at)[0];at+=4
                        result.append(bytes(packet[at:at+length]));at+=length
                    return result
                packet.clear()
    raise AssertionError('Missing Vorbis comments')

def crc(data):
    value=0
    for byte in data:
        value^=byte<<24
        for _ in range(8):value=((value<<1)^ (0x04c11db7 if value&0x80000000 else 0))&0xffffffff
    return value

def native_metadata(path,original):
    native=comments(original.read_bytes())[0]
    assert struct.unpack_from('<I',native)[0] in (1,2,3)
    vendor=b'CoPVR local native-audio excerpt'
    packet=b'\x03vorbis'+struct.pack('<I',len(vendor))+vendor+struct.pack('<II',1,len(native))+native+b'\x01'
    output=[];replaced=False
    for page in pages(path.read_bytes()):
        count=page[26];laces=list(page[27:27+count]);body=page[27+count:]
        if body.startswith(b'\x03vorbis'):
            n=0;size=0
            while True:
                size+=laces[n];n+=1
                if laces[n-1]<255:break
            new_laces=[255]*(len(packet)//255)+[len(packet)%255]+laces[n:]
            assert len(new_laces)<256
            header=bytearray(page[:27]);header[22:26]=b'\0'*4;header[26]=len(new_laces)
            page=header+bytes(new_laces)+packet+body[size:]
            page[22:26]=struct.pack('<I',crc(page));page=bytes(page);replaced=True
        output.append(page)
    assert replaced
    path.write_bytes(b''.join(output))
    assert comments(path.read_bytes())[0]==native

def build():
    manifest=[];cache={}
    def cut(source,start,end,action):
        key=(source,start,end,action)
        if key in cache:return cache[key]
        path=OUTPUT/f'{source}_{action}.ogg';duration=end-start
        fade=min(.005,duration/8)
        subprocess.run(['ffmpeg','-v','error','-y','-i',str(SOURCE/(source+'.ogg')),
            '-af',f'atrim=start={start}:end={end},asetpts=PTS-STARTPTS,afade=t=in:d={fade},afade=t=out:st={duration-fade}:d={fade}',
            '-ac','1','-ar','44100','-c:a','libvorbis','-q:a','5','-map_metadata','-1',str(path)],check=True)
        native_metadata(path,SOURCE/(source+'.ogg'))
        cache[key]='copvr\\mechanical\\'+path.stem
        manifest.append(dict(file=str(path.relative_to(ROOT)),source=source,start=start,end=end,action=action))
        return cache[key]

    sections={};parents={}
    for path in (ROOT/'reports/archive-inspection/configs/weapons').glob('*.ltx'):
        current=None
        for line in path.read_text(encoding='cp1251').splitlines():
            line=line.split(';',1)[0].strip();match=re.match(r'\[([^]]+)\](?::(.*))?',line)
            if match:
                current=match[1];sections.setdefault(current,{})
                parents[current]=[p.strip() for p in (match[2] or '').split(',') if p.strip()]
            elif current and '=' in line:
                key,value=line.split('=',1);sections[current][key.strip()]=value.strip().strip('"')

    def resolve(name,seen=()):
        if name in seen:return {}
        data={}
        for parent in parents.get(name,[]):data.update(resolve(parent,seen+(name,)))
        data.update(sections.get(name,{}));return data

    weapons={}
    for name in sorted(sections,key=len):
        data=resolve(name)
        if name.startswith('wpn_') and data.get('class','').startswith('WP_') and 'hud' in data:
            weapons.setdefault(data['hud'],data)

    config=ROOT/'game-vr/gamedata/configs/copvr_equipment.ltx'
    text=re.sub(r'\n\[(?:mechanical_audio_|internal_audio_)[^[]*(?=\n\[|\Z)','',config.read_text(encoding='utf-8'))
    for profile in json.loads((ROOT/'reports/weapon-handle-models.json').read_text()):
        hud=profile['hud'];data=weapons[hud];parts=data['snd_reload'].split(',')
        source=parts[0].strip().split('\\')[-1];volume=float(parts[1]) if len(parts)>1 else 1
        actions={}
        if source in CUTS:
            for action,interval in zip(('remove','insert','rack'),CUTS[source]):
                if interval:actions[action]=cut(source,*interval,action)
        else:
            assert hud in RACKS,(hud,source)
            source,start,end=RACKS[hud];actions['rack']=cut(source,start,end,'rack')
        text+=f'\n[mechanical_audio_{hud}]\nvolume = {volume:.2f}\n'
        for action,file in actions.items():text+=f'{action} = {file}\n'
    internal={
        'wpn_winchester1300_hud':{'remove':('w1300_open',0,.311),'insert':('w1300_load',0,.31),'rack':('w1300_close',0,.176)},
        'wpn_spas12_hud':{'remove':('spas12_open',0,.531),'insert':('spas12_reload',0,.336),'rack':('spas12_close',0,.34)},
        'wpn_bm16_hud':{'remove':('bm16_reload_lr',.38,.78),'insert':('bm16_reload_lr',1.95,2.17),'rack':('bm16_reload_lr',4.52,4.76)},
        'wpn_toz34_hud':{'remove':('toz34_reload',0,.23),'insert':('toz34_reload',1.56,1.73),'rack':('toz34_reload',3.14,3.44)},
        'wpn_rg-6_hud':{'remove':('rg6_reload_start',0,.40),'insert':('rg6_reload',1.41,1.882),'rack':('rg6_reload_end',0,.40)},
        'wpn_protecta_hud':{'insert':('spas12_reload',0,.336),'rack':('spas12_close',0,.34)},
        'wpn_rpg7_hud':{'insert':('rpg7_reload',1.36,2.275)},
    }
    for hud,actions in internal.items():
        text+=f'\n[internal_audio_{hud}]\nvolume = 1.0\n'
        for action,(source,start,end) in actions.items():
            file=cut(source,start,end,'internal_'+action);text+=f'{action} = {file}\n'
    config.write_text(text,encoding='utf-8')
    (ROOT/'reports/mechanical-sound-cuts.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
    print(f'Built {len(manifest)} mono native sound excerpts with original X-Ray metadata.')

if __name__=="__main__":
    build()

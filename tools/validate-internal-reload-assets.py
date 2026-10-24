"""Offline validation of single-round shotgun/RPG assets and profiles."""
from pathlib import Path
import configparser,json,math,struct
ROOT=Path(__file__).resolve().parent.parent
ini=configparser.ConfigParser();ini.read(ROOT/'game-vr/gamedata/configs/copvr_equipment.ltx',encoding='utf-8')

def chunks(data):
    offset=0
    while offset+8<=len(data):
        kind,size=struct.unpack_from('<II',data,offset)
        assert offset+8+size<=len(data)
        yield kind,data[offset+8:offset+8+size];offset+=8+size
    assert offset==len(data)

profiles=json.loads((ROOT/'reports/internal-reload-profiles.json').read_text())
assert len(profiles)==7
assert ini['internal_vr_wpn_spas12_hud'].getboolean('load_requires_open')
for hud,profile in profiles.items():
    section='internal_vr_'+hud;assert ini[section].getint('kind')==profile['kind']
    assert 'internal_audio_'+hud in ini
    native=next((ROOT/'reports/archive-inspection/meshes').rglob(('wpn_rg6_hud' if hud=='wpn_rg-6_hud' else hud)+'.ogf'))
    data=dict(chunks(native.read_bytes()));names=[];offset=4
    for _ in range(struct.unpack_from('<I',data[13])[0]):
        end=data[13].index(0,offset);names.append(data[13][offset:end].decode());offset=data[13].index(0,end+1)+61
    for key in ('pump_bone','bolt_bone','feed_bone','hinge','chamber0','chamber1','drum'):
        if key in profile:assert profile[key] in names
    if profile['kind']==2:
        assert math.dist(profile['port0'],profile['port1'])>.015
        assert 0<ini[section].getfloat('finish_fraction')<1
    if profile['kind']==5:
        motion=next((ROOT/'reports/archive-inspection/meshes').rglob('wpn_rg6_hud_animation.omf'))
        motion_chunks=dict(chunks(motion.read_bytes()))
        assert b'idle\0' in motion_chunks[15]
        assert profile['open_motion'].encode()+b'\0' in motion_chunks[15]
    if profile['kind']==3:
        assert abs(profile['port0'][2]-.216)<.00001
        assert ini[section]['round_profile']=='internal_round_rocket'

for name in ('12g','rocket','vog25'):
    section=ini['internal_round_'+name];relative=section['model'].replace('\\','/')+'.ogf'
    path=ROOT/('game-vr/gamedata/meshes' if name!='rocket' else 'reports/archive-inspection/meshes')/relative
    data=dict(chunks(path.read_bytes()));bounds=struct.unpack_from('<6f',data[1],4)
    length=bounds[5]-bounds[2]
    assert (.035<length<.08) if name=='12g' else (.08<length<.12) if name=='vog25' else (.65<length<.80)
    expected=[(bounds[i]+bounds[i+3])/2 for i in range(3)]
    actual=list(map(float,section['center'].split(',')))
    assert all(abs(a-b)<1e-5 for a,b in zip(expected,actual))
    if name!='rocket':
        assert data[1][1]==10 and struct.unpack_from('<I',data[13])[0]==1
        for _,child in chunks(data[9]):
            mesh=dict(chunks(child));fmt,count=struct.unpack_from('<II',mesh[3]);assert fmt==1
            for i in range(count):
                values=struct.unpack_from('<14fI',mesh[3],8+i*60)
                assert values[-1]==0 and all(math.isfinite(v) for v in values)
print('PASS: 7 internal reload profiles, native bones/ports, single 12g round and full-size 0.706 m rocket.')

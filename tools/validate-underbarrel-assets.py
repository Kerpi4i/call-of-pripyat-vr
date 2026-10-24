from pathlib import Path
import struct,json,configparser,subprocess,importlib.util,re,hashlib
ROOT=Path(__file__).resolve().parent.parent
spec=importlib.util.spec_from_file_location('vr_sound',ROOT/'tools/build-vr-mechanical-sounds.py');sound=importlib.util.module_from_spec(spec);spec.loader.exec_module(sound)
ini=configparser.ConfigParser();ini.read(ROOT/'game-vr/gamedata/configs/copvr_equipment.ltx',encoding='utf-8')
profiles=json.loads((ROOT/'reports/underbarrel-profiles.json').read_text());assert len(profiles)==6
for p in profiles:
 h=p['hud'];section=ini['launcher_vr_'+h];assert len(p['port'])==3 and .25<p['port'][2]<.8
 assert section['bone']==p['bone'];assert 0<section.getfloat('finish_fraction')<1
 assert ini['launcher_grip_'+h]['alias']=='anm_reload_g'
 mesh=ROOT/'game-vr/gamedata/meshes'/('copvr/rounds/launcher_'+h+'.ogf');assert mesh.stat().st_size>100
 audio=ROOT/p['audio'];probe=json.loads(subprocess.check_output(['ffprobe','-v','quiet','-show_streams','-show_format','-of','json',str(audio)]));assert probe['streams'][0]['channels']==1
 assert abs(float(probe['format']['duration'])-(p['end']-p['start']))<.08
 native=ROOT/'reports/archive-inspection/sounds/weapons'/(p['source']+'.ogg');assert sound.comments(audio.read_bytes())[0]==sound.comments(native.read_bytes())[0]
 for page in sound.pages(audio.read_bytes()):
  raw=bytearray(page);expected=struct.unpack_from('<I',raw,22)[0];raw[22:26]=b'\0'*4;assert sound.crc(raw)==expected
for name,save in [('underbarrel_test','copvr_underbarrel_test'),('jam_test','copvr_jam_test')]:assert ini[name]['save_name']==save and ini[name].getboolean('enabled')
manifest=json.loads((ROOT/'reports/weapon-test-save-copies.json').read_text(encoding='utf-8'));source=Path(manifest['source']);digest=hashlib.sha256(source.read_bytes()).hexdigest();assert digest==manifest['source_sha256']
for p in manifest['test_saves']:assert Path(p).is_file() and hashlib.sha256(Path(p).read_bytes()).hexdigest()==digest
sections={};parents={}
for path in (ROOT/'reports/archive-inspection/configs/weapons').glob('*.ltx'):
 current=None
 for line in path.read_text(encoding='cp1251').splitlines():
  line=line.split(';')[0].strip();m=re.match(r'\[([^]]+)\](?::(.*))?',line)
  if m:current=m[1];sections.setdefault(current,{});parents[current]=[x.strip() for x in (m[2] or '').split(',') if x.strip()]
  elif current and '=' in line:k,v=line.split('=',1);sections[current][k.strip()]=v.strip()
def resolve(name,seen=()):
 data={}
 if name in seen:return data
 for p in parents.get(name,[]):data.update(resolve(p,seen+(name,)))
 data.update(sections.get(name,{}));return data
weapons=json.loads((ROOT/'reports/all-weapons-test-manifest.json').read_text())['weapons'];kit=[];ammo={}
for name in weapons:
 data=resolve(name)
 if int(data.get('grenade_launcher_status','0'))>0 or data.get('hud')=='wpn_abakan_hud':
  assert data['hud'] in {p['hud'] for p in profiles},name
  kit.append(dict(weapon=name,hud=data['hud'],upgrade='up_fiftha_abakan' if data['hud']=='wpn_abakan_hud' else None))
  for kind in data['grenade_class'].split(','):kind=kind.strip();ammo[kind]=ammo.get(kind,0)+100
assert len({v['hud'] for v in kit})==6
(ROOT/'reports/underbarrel-test-kit.json').write_text(json.dumps(dict(weapons=kit,ammunition=ammo),indent=2))
print(f'PASS: 6 underbarrel muzzle/round/grip/audio profiles; {len(kit)} test weapons, {len(ammo)} grenade types; 2 isolated save copies, original unchanged.')

"""Portable source/package preflight. Does not launch the game."""
from pathlib import Path
import hashlib,json,configparser,re,xml.etree.ElementTree as ET
root=Path(__file__).resolve().parent.parent
base=root/'release'
manifest=json.loads((base/'asset-manifest.json').read_text())
for entry in manifest['overlay']:
    file=base/'gamedata'/entry['path']
    assert file.is_file(),str(file)
    assert hashlib.sha256(file.read_bytes()).hexdigest()==entry['sha256'],str(file)
for p in (base/'gamedata/configs/ui').rglob('ui_mm_opt*.xml'):ET.fromstring(p.read_bytes())
for lang in ['en','uk']:
    for name in ['USER-GUIDE','FEATURE-STATUS']:
        for ext in ['md','html']:assert (base/'docs'/f'{name}.{lang}.{ext}').is_file()
ini=configparser.ConfigParser(strict=False);ini.read(base/'gamedata/configs/copvr_equipment.ltx',encoding='cp1251')
for section in ['scope_test','integrated_scope_test','all_weapons_test','underbarrel_test','jam_test']:
    assert not ini.getboolean(section,'enabled')
    assert not ini.getboolean(section,'unlimited_weight')
config=(base/'defaults/user.ltx').read_text()
assert 'copvr_show_interaction_zones off' in config
assert 'vid_mode 2560x1440' in config
assert not re.search(r'^(bind|unbind|snd_device|g_unlimited|g_god|vid_monitor)\b',config,re.M)
assert not re.search(r'[A-Z]:\\Users\\',(base/'defaults/fsgame.template.ltx').read_text())
assert not any(p.suffix.lower() in {'.scop','.pdb','.bak'} for p in base.rglob('*') if p.is_file())
print(f"PASS: {len(manifest['overlay'])} resource hashes, XML, bilingual guides, portable paths, no test cheats/private saves.")

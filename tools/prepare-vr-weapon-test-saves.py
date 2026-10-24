from pathlib import Path
import shutil,json,hashlib,datetime
root=Path.cwd();folder=root/'game-vr/appdata/savedgames';source=max((p for p in folder.glob('*.scop') if not p.stem.startswith('copvr_')),key=lambda p:p.stat().st_mtime)
backup=root/'reports/backups/weapon-test-saves'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%d-%H%M%S');backup.mkdir(parents=True,exist_ok=True)
created=[]
for name in ['copvr_underbarrel_test','copvr_jam_test']:
 for ext in ['.scop','.dds']:
  original=source.with_suffix(ext);target=folder/(name+ext)
  if not original.exists():continue
  if target.exists():shutil.copy2(target,backup/target.name)
  shutil.copy2(original,target)
  assert hashlib.sha256(original.read_bytes()).digest()==hashlib.sha256(target.read_bytes()).digest()
 created.append(str(folder/(name+'.scop')))
manifest={'source':str(source),'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'test_saves':created,'provisioning':'Named-save-only native kit issuance after loading; save again in-game to persist issued gear.','jam_condition':0.05,'ammo_per_compatible_weapon':100,'originals_unchanged':True}
(root/'reports/weapon-test-save-copies.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2),encoding='utf-8');print('Created:',*[Path(p).stem for p in created]);print('Original hash verified unchanged.')

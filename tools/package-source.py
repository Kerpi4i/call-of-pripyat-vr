"""Package only Git-visible source files, never ignored engine/game/private state."""
from pathlib import Path
import subprocess,zipfile
root=Path(__file__).resolve().parent.parent
names=subprocess.check_output(['git','ls-files','--cached','--others','--exclude-standard','-z'],cwd=root).decode('utf-8').split('\0')
destination=root/'dist/CoPVR-0.1.0-alpha.2-source.zip'
if destination.exists():raise SystemExit('Source ZIP exists; refusing overwrite')
forbidden={'engine','build','reports','game-vr','game-test','dist','.git'}
with zipfile.ZipFile(destination,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as archive:
    for name in sorted(set(names)):
        if not name:continue
        p=Path(name)
        if p.parts[0] in forbidden:raise ValueError('Private/generated root in source manifest: '+name)
        source=root/p
        if not source.is_file():continue
        archive.write(source,'CoPVR-0.1.0-alpha.2-source/'+p.as_posix())
print(destination.name)

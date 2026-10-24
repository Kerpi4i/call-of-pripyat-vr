"""Check normal PE import closure. Optional Ansel is delay-loaded and omitted."""
from pathlib import Path
import subprocess,re,json,os
root=Path(__file__).resolve().parent.parent
vswhere=Path(os.environ['ProgramFiles(x86)'])/'Microsoft Visual Studio/Installer/vswhere.exe'
vs=subprocess.check_output([str(vswhere),'-latest','-property','installationPath'],text=True).strip()
dump=next((Path(vs)/'VC/Tools/MSVC').glob('*/bin/Hostx64/x64/dumpbin.exe'))
folder=root/'build/release-0.1.0-alpha.1/bin'
packaged={p.name.lower() for p in folder.iterdir()};failures=[];optional=[]
for p in folder.iterdir():
    if p.suffix.lower() not in {'.exe','.dll'}:continue
    text=subprocess.check_output([str(dump),'/dependents',str(p)],text=True)
    main=text.split('Image has the following delay load dependencies:')[0]
    for name in re.findall(r'^    ([\w.\-]+\.dll)\s*$',main,re.M|re.I):
        if name.lower() in packaged or name.lower().startswith(('api-ms-','ext-ms-')) or (Path(os.environ['WINDIR'])/'System32'/name).exists():continue
        failures.append([p.name,name])
    if 'Image has the following delay load dependencies:' in text:
        optional.extend([p.name,n] for n in re.findall(r'^    ([\w.\-]+\.dll)\s*$',text.split('Image has the following delay load dependencies:')[1],re.M|re.I))
result={'missing_required':failures,'delay_loaded':optional,'binary_count':len(packaged)}
(root/'reports/release-dependency-check.json').write_text(json.dumps(result,indent=2))
print(json.dumps(result));raise SystemExit(1 if failures else 0)

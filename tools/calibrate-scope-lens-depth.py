"""Ray-intersect native ocular glass polygons; place VR lenses 5 mm toward player."""
import argparse, configparser, json, re, struct
from pathlib import Path
root=Path(__file__).resolve().parent.parent
parser=argparse.ArgumentParser();parser.add_argument('--apply',action='store_true');args=parser.parse_args()
config=root/'game-vr/gamedata/configs/copvr_equipment.ltx'
ini=configparser.ConfigParser();ini.read(config,encoding='utf-8')
def chunks(data):
    offset=0
    while offset+8<=len(data):
        kind,size=struct.unpack_from('<II',data,offset)
        yield kind,data[offset+8:offset+8+size];offset+=8+size

def cross(a,b):return a[0]*b[1]-a[1]*b[0]
rows=[]
for section in ini.sections():
    if not section.startswith('scope_vr_'):continue
    hud=section.removeprefix('scope_vr_');paths=list((root/'reports/archive-inspection/meshes').rglob(hud+'.ogf'))
    assert len(paths)==1,hud
    data=dict(chunks(paths[0].read_bytes()));names=data[13];offset=4;bones=[]
    for _ in range(struct.unpack_from('<I',names)[0]):
        end=names.index(0,offset);bones.append(names[offset:end].decode());offset=end+1
        end=names.index(0,offset);offset=end+1+60
    wanted=ini[section].get('scope_bone','wpn_scope')
    cx,cy,old_z=map(float,ini[section]['lens_center'].split(','));hits=[]
    for child_id,child in chunks(data[9]):
        mesh=dict(chunks(child));vertices=mesh.get(3);indices=mesh.get(4)
        if vertices is None or indices is None:continue
        fmt,count=struct.unpack_from('<II',vertices);stride=(len(vertices)-8)//count
        if fmt!=1:continue # All native ocular glass surfaces use 1-weight vertices.
        points=[struct.unpack_from('<fff',vertices,8+i*stride) for i in range(count)]
        owners=[bones[struct.unpack_from('<I',vertices,8+i*stride+56)[0]] for i in range(count)]
        count=struct.unpack_from('<I',indices)[0];faces=struct.unpack_from('<'+'H'*count,indices,4)
        for face in range(0,count,3):
            ids=faces[face:face+3]
            if any(owners[i]!=wanted for i in ids):continue
            a,b,c=[points[i] for i in ids]
            # The rim has a hole: its triangles cannot intersect the optical-centre ray.
            # Select the first near-perpendicular cap intersected from the player side.
            if max(p[2] for p in (a,b,c))-min(p[2] for p in (a,b,c))>.001:continue
            ab=(b[0]-a[0],b[1]-a[1]);ac=(c[0]-a[0],c[1]-a[1]);ap=(cx-a[0],cy-a[1])
            area=cross(ab,ac)
            if abs(area)<1e-9:continue
            u=cross(ap,ac)/area;v=cross(ab,ap)/area
            if u<-.001 or v<-.001 or u+v>1.001:continue
            z=a[2]+u*(b[2]-a[2])+v*(c[2]-a[2])
            if not old_z-.015<z<old_z+.09:continue
            hits.append(dict(z=z,mesh=child_id,face=face//3,texture=mesh.get(2,b'').split(b'\0')[0].decode()))
    assert hits,('No central glass polygon found',hud)
    hits.sort(key=lambda h:h['z']);glass_z=hits[0]['z'];desired=round(glass_z-.005,6)
    rows.append(dict(section=section,glass_z=glass_z,old_z=old_z,new_z=desired,
                     offset_mm=(glass_z-desired)*1000,polygon=hits[0],candidate_depths=sorted(set(round(h['z'],6) for h in hits))))
assert len(rows)==15
text=config.read_text(encoding='utf-8')
if args.apply:
    backup=root/'reports/scope-lens-depth-before-glass-5mm.ltx'
    if not backup.exists():backup.write_text(text,encoding='utf-8')
    for row in rows:
        pattern=r'(\['+re.escape(row['section'])+r'\][^\[]*?lens_center\s*=\s*[^,\n]+,\s*[^,\n]+,\s*)[-+0-9.eE]+'
        text,count=re.subn(pattern,lambda m:m[1]+f"{row['new_z']:.6f}",text)
        assert count==1,row['section']
    text=text.replace('measured ocular rim','measured internal glass polygon')
    config.write_text(text,encoding='utf-8')
    (root/'reports/scope-lens-glass-depth-5mm.json').write_text(json.dumps(rows,indent=2),encoding='utf-8')
else:
    for row in rows:assert abs(row['old_z']-row['new_z'])<1e-6,row
for row in rows:print(row['section'],f"glass={row['glass_z']:.6f} lens={row['new_z']:.6f} outward={row['offset_mm']:.3f} mm")
print('PASS:',len(rows),'scope profiles measured from central glass polygons, not housing rims')

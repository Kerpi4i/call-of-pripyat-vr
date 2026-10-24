from pathlib import Path
import struct,math
from PIL import Image,ImageDraw
exec(Path('tools/build-magazine-models.py').read_text().split('profiles=[]')[0])
d=dict(chunks(next(Path('reports/archive-inspection/meshes').rglob('wpn_protecta_hud.ogf')).read_bytes()));tex=Image.open('reports/archive-inspection/textures/wpn/wpn_protecta.dds').convert('RGB');faces=[]
for _,child in chunks(d[9]):
 m=dict(chunks(child));vb=m[3];fmt,n=struct.unpack_from('<II',vb);stride=(len(vb)-8)//n;v=[]
 for i in range(n):
  a=8+i*stride;off=0 if fmt==1 else 4;p=struct.unpack_from('<3f',vb,a+off);uv=struct.unpack_from('<2f',vb,a+(48 if fmt==1 else 56));v.append((p,uv))
 ib=m[4];n=struct.unpack_from('<I',ib)[0];inds=struct.unpack_from('<'+'H'*n,ib,4)
 for i in range(0,n,3):faces.append(([v[j] for j in inds[i:i+3]],b'wpn_protecta' in m[2]))
out=Image.new('RGB',(1400,1050),(65,70,75));draw=ImageDraw.Draw(out)
for row,(depth,sign,vertical) in enumerate([(0,1,1),(0,-1,1),(1,-1,0)]):
 cy=row*350+285;scale=1700
 def proj(p):return (200+p[2]*scale,cy-p[vertical]*scale)
 for f,proper in sorted(faces,key=lambda pair:sum(v[0][depth]*sign for v in pair[0])):
  pts=[proj(v[0]) for v in f];x0=math.floor(min(p[0] for p in pts));y0=math.floor(min(p[1] for p in pts));x1=math.ceil(max(p[0] for p in pts));y1=math.ceil(max(p[1] for p in pts))
  if x1<=x0 or y1<=y0:continue
  uv=[(v[1][0]*tex.width,v[1][1]*tex.height) for v in f];a,b,c=pts;det=(b[0]-a[0])*(c[1]-a[1])-(c[0]-a[0])*(b[1]-a[1])
  if abs(det)<.5:continue
  coeff=[]
  for k in range(2):
   A=((uv[1][k]-uv[0][k])*(c[1]-a[1])-(uv[2][k]-uv[0][k])*(b[1]-a[1]))/det;B=((b[0]-a[0])*(uv[2][k]-uv[0][k])-(c[0]-a[0])*(uv[1][k]-uv[0][k]))/det;C=uv[0][k]-A*a[0]-B*a[1];coeff.extend([A,B,C+A*x0+B*y0])
  patch=tex.transform((x1-x0,y1-y0),Image.Transform.AFFINE,coeff,Image.Resampling.BILINEAR) if proper else Image.new('RGB',(x1-x0,y1-y0),'gray');mask=Image.new('L',patch.size);ImageDraw.Draw(mask).polygon([(p[0]-x0,p[1]-y0) for p in pts],fill=255);out.paste(patch,(x0,y0),mask)
 draw=ImageDraw.Draw(out);draw.text((20,row*350+10),['RIGHT +X','LEFT -X','BOTTOM -Y'][row],fill='white')
 for z in [.1,.15,.2,.25,.3,.35,.4,.5,.6]:
  x=200+z*scale;draw.line((x,row*350+305,x,row*350+320),fill='white');draw.text((x-15,row*350+323),str(z),fill='white')
 x,y=proj((.0342,.125,-.010));draw.ellipse((x-7,y-7,x+7,y+7),outline='red',width=3)
Path('reports/model-inspection').mkdir(exist_ok=True);out.save('reports/model-inspection/protecta-loading-window.png')

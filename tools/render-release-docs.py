"""Build offline browser guides and a simple original launcher icon (build-time only)."""
from pathlib import Path
import markdown
from PIL import Image,ImageDraw
base=Path(__file__).resolve().parent.parent/'release'
style='''body{font:17px/1.65 system-ui,sans-serif;background:#151b18;color:#e7ede8;max-width:1020px;margin:40px auto;padding:0 28px}a{color:#9edd9f}h1,h2,h3{line-height:1.25;color:#c8e4bb}h2{margin-top:2em}table{border-collapse:collapse;width:100%;font-size:15px}td,th{border:1px solid #455147;padding:10px;text-align:left}th{background:#283429}code{background:#29302b;padding:2px 5px;border-radius:4px}li{margin:.45em 0}nav{font-size:14px}'''
for file in (base/'docs').glob('*.md'):
    body=markdown.markdown(file.read_text(encoding='utf-8'),extensions=['tables','fenced_code'])
    body=body.replace('.md"','.html"')
    lang='uk' if '.uk.' in file.name else 'en'
    html='<!doctype html><html lang="'+lang+'"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>CoPVR Alpha guide</title><style>'+style+'</style><nav><a href="USER-GUIDE.en.html">English guide</a> · <a href="USER-GUIDE.uk.html">Українська інструкція</a> · <a href="FEATURE-STATUS.'+lang+'.html">Feature status / Стан функцій</a></nav>'+body+'</html>'
    file.with_suffix('.html').write_text(html,encoding='utf-8')
image=Image.new('RGBA',(256,256),(0,0,0,0));draw=ImageDraw.Draw(image)
draw.rounded_rectangle((8,8,248,248),radius=50,fill='#17251e',outline='#aacd7a',width=9)
draw.rounded_rectangle((40,76,216,168),radius=24,outline='#c7e1a4',width=13)
draw.line((58,120,86,146,111,104),fill='#c7e1a4',width=12)
draw.line((143,146,143,103,181,103,193,118,181,129,143,129),fill='#c7e1a4',width=10)
draw.line((173,130,194,148),fill='#c7e1a4',width=10)
image.save(base/'CoPVR.ico',sizes=[(16,16),(32,32),(48,48),(64,64),(128,128),(256,256)])
print('Offline HTML guides and original VR icon generated.')

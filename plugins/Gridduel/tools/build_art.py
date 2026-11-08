from pathlib import Path
from PIL import Image,ImageDraw
import json
import sys
p=Path(sys.argv[1] if len(sys.argv)>1 else '.').resolve();a=p/'assets/sprites';dest=p/'plugins/Gridduel/data'
paths=['bots/240907-CBOT-GB.png','bots/240907-BBOT-GB.png','bots/240907-DBOT.png','bots/241210-GB-GENBOT.png','bots/241010-MILTONBOTS-FIXD.png','bots/250911-lbot.png','bots/250908-bots-Mbot.png','bots/roebot_00_spritesheet.png']
imgs=[];manifest=[]
for theme in range(2):
 for i in range(8):
  f=a/(paths[i] if theme==0 else '25TAGZ/'+str(11+i).zfill(3)+'.png');im=Image.open(f).convert('RGBA')
  if theme==1:im=im.resize((16,16),Image.Resampling.NEAREST)
  elif i==6:
   im.thumbnail((16,16),Image.Resampling.NEAREST);temp=Image.new('RGBA',(16,16),(0,255,0,255));temp.paste(im,((16-im.width)//2,0));im=temp
  else:
   meta=Path(str(f)+'.gbsres');frame=None
   if meta.exists():
    md=json.loads(meta.read_text());frame=next((fr for st in md.get('states',[]) for an in st.get('animations',[]) for fr in an.get('frames',[]) if fr.get('tiles')),None)
   if frame:
    ts=frame['tiles'];minx=min(t['x'] for t in ts);maxx=max(t['x']+8 for t in ts);miny=min(-t['y'] for t in ts);maxy=max(-t['y']+16 for t in ts)
    canvas=Image.new('RGBA',(maxx-minx,maxy-miny),(0,255,0,255))
    for t in ts:
     sl=im.crop((t['sliceX'],t['sliceY'],t['sliceX']+8,t['sliceY']+16))
     if t.get('flipX'):sl=sl.transpose(Image.Transpose.FLIP_LEFT_RIGHT)
     if t.get('flipY'):sl=sl.transpose(Image.Transpose.FLIP_TOP_BOTTOM)
     canvas.paste(sl,(t['x']-minx,-t['y']-miny))
    canvas.thumbnail((16,16),Image.Resampling.NEAREST);im=Image.new('RGBA',(16,16),(0,255,0,255));im.paste(canvas,((16-canvas.width)//2,(16-canvas.height)//2))
   else:im=im.crop((0,0,16,16))
  # Preserve original pixels and use sprite-green only as background key.
  q=Image.new('P',(16,16));pix=[]
  for r,g,b,alpha in im.getdata():
   if alpha<128 or (g>200 and r<80 and b<80):v=0
   else:
    lum=(r*3+g*6+b)//10;v=1 if lum>190 else 2 if lum>80 else 3
   pix.append(v)
  q.putdata(pix);imgs.append(q);manifest.append({'theme':theme,'card_id':i+1,'source':str(f.relative_to(p)),'adaptation':'Nearest-neighbor16x16; sprite-green keyed transparent;4GB shades. MBOT full portrait fitted.'})
def tiles(im):
 b=[]
 for ty in range(im.height//8):
  for tx in range(im.width//8):
   for y in range(8):
    vals=[im.getpixel((tx*8+x,ty*8+y)) for x in range(8)];b.extend([sum((v&1)<<(7-x) for x,v in enumerate(vals)),sum(((v>>1)&1)<<(7-x) for x,v in enumerate(vals))])
 return b
s=['/* Derived ONLY from existing uploaded BOT/HASHTAGZ pixels. See art_sources.json. */','static const uint8_t gd_token_tiles[1024]={'+','.join(str(v) for im in imgs for v in tiles(im))+'};','static const uint8_t gd_portraits[16][256]={']
for im in imgs:s.append('{'+','.join(map(str,tiles(im.resize((32,32),Image.Resampling.NEAREST))))+'},')
s+=['};'];(p/'assets/engine/include/gridduel_art_data.h').write_text('\n'.join(s)+'\n');(dest/'art_sources.json').write_text(json.dumps(manifest,indent=2)+'\n')

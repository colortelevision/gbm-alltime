import json,os,uuid,copy,glob
from PIL import Image,ImageDraw
R='/workspace/gbm-alltime'; B='/workspace/gbs43/appData/templates/blank'
U=lambda:str(uuid.uuid4())
def W(path,obj):
    os.makedirs(os.path.dirname(path),exist_ok=True); open(path,'w').write(json.dumps(obj,indent=2,ensure_ascii=False))
def gid(f): return json.load(open(f))['id']
S=R+'/assets/sprites/untitled'
SPR={k:gid(f'{S}/{fn}.gbsres') for k,fn in [('mat','0ALLRPG_MAT.png'),('con','0ALLRPG_CON.png'),('par','0ALLRPG_PAR.png'),('ami','0ALLRPG_AMI.png'),('max','112222 Max RPG GB Sprite sheet 16px.png')]}
PAL={k:gid(f'{R}/project/palettes/{k}.gbsres') for k in ['matt','conner','parker','ami']}
MU=lambda n:gid(f'{R}/assets/music/{n}.uge.gbsres')
MUS=dict(title=MU('0song_template 11DREAMON'),curb=MU('00-MM_happysongrock_'),office=MU('00-240627-BACH-INVENTION-2'),house=MU('00JW-MontSpring'),nave=MU('0song_template 8CASTLE'),cellar=MU('00-240911-track-016UNDERWORLD'))
SN=lambda n:gid(f'{R}/assets/sounds/{n}-gb_louder.wav.gbsres')
SFX=dict(talk=SN('IM'),door=SN('door-wood'),doorin=SN('BuddyIn'),doorout=SN('BuddyOut'),bus=SN('door-metal'),locked=SN('invalid'),
 wine=SN('boing'),drunk=SN('aura'),flame=SN('bowser-fire'),bite=SN('chomp'),crash=SN('bowser-die'),mail=SN("You've Got Mail"),pc=SN("File's Done"),
 status=SN('mips-hop'),flag=SN('powerup'),cut_in=SN('descend'),cut_out=SN('airrecover'),bind=SN('bowser-grab'),blind=SN('swoop'),
 rescue=SN('strongpunch'),start=SN('Welcome'),white=SN('warp2'),choice=SN('timer'))
# ---------- prop sprites ----------
PD=R+'/assets/sprites/untitled_props'; os.makedirs(PD,exist_ok=True)
tmpl=json.load(open(B+'/assets/sprites/static.png.gbsres'))
C=[(224,248,208),(136,192,112),(52,104,86),(8,24,32)]
def mk_static(name,frames,draw):
    w=16*frames; im=Image.new('RGB',(w,16),(101,255,0)); d=ImageDraw.Draw(im)  # GBS transparent green
    for i in range(frames): draw(d,i*16,i)
    fn=f'unt_{name}.png'; im.save(f'{PD}/{fn}')
    p=f'{PD}/{fn}.gbsres'; old=json.load(open(p))['id'] if os.path.exists(p) else U()
    g=copy.deepcopy(tmpl); g['id']=old; g['name']=f'untitled_props/unt_{name}'; g['symbol']=f'sprite_unt_{name}'; g['filename']=fn
    g['width']=w; g.pop('checksum',None); g['numTiles']=2*frames
    st=g['states'][0]; st['id']=U()
    for a in st['animations']: a['id']=U(); [f.__setitem__('id',U()) for f in a['frames']]
    fr=st['animations'][0]['frames'][0]; fl=[]
    for i in range(frames):
        f=copy.deepcopy(fr); f['id']=U()
        for j,t in enumerate(f['tiles']): t['id']=U(); t['sliceX']=i*16+j*8
        fl.append(f)
    st['animations'][0]['frames']=fl
    W(p,g); SPR[name]=g['id']
D=lambda c:C[c]
mk_static('controller',1,lambda d,x,i:None)
mk_static('cross',1,lambda d,x,i:(d.rectangle([x+7,0,x+8,15],fill=D(3)),d.rectangle([x+2,4,x+13,5],fill=D(3)),d.rectangle([x+7,1,x+7,14],fill=D(2))))
mk_static('candle',2,lambda d,x,i:(d.rectangle([x+6,9,x+9,15],fill=D(1)),d.rectangle([x+6,9,x+6,15],fill=D(0)),d.ellipse([x+5-i,2+i,x+10+i,8],fill=D(2)),d.ellipse([x+7,4+i,x+8,7],fill=D(0))))
mk_static('bus',1,lambda d,x,i:(d.rectangle([x,2,x+15,12],fill=D(2)),d.rectangle([x+1,4,x+14,7],fill=D(0)),d.line([x+5,4,x+5,7],fill=D(3)),d.line([x+10,4,x+10,7],fill=D(3)),d.ellipse([x+1,11,x+5,15],fill=D(3)),d.ellipse([x+10,11,x+14,15],fill=D(3))))
mk_static('laptop',2,lambda d,x,i:(d.rectangle([x+3,4,x+12,10],fill=D(3)),d.rectangle([x+4,5,x+11,9],fill=D(1 if i else 2)),d.rectangle([x+1,11,x+14,13],fill=D(2))))
mk_static('bottle',1,lambda d,x,i:(d.rectangle([x+7,1,x+8,5],fill=D(3)),d.rectangle([x+5,6,x+10,14],fill=D(3)),d.rectangle([x+6,8,x+7,12],fill=D(1))))
mk_static('machine',2,lambda d,x,i:(d.rectangle([x+2,5,x+13,13],fill=D(3)),d.ellipse([x+4,7,x+7,10],fill=D(1)),d.ellipse([x+9,7,x+12,10],fill=D(1)),d.rectangle([x+11,3,x+12,4],fill=D(0 if i else 2))))
mk_static('flame',2,lambda d,x,i:(d.polygon([(x+8,1+i),(x+14,11),(x+8,15),(x+2,11)],fill=D(2)),d.polygon([(x+8,6-i),(x+11,12),(x+5,12)],fill=D(0))))
# ---------- tile painter / backgrounds ----------
BGD=R+'/assets/backgrounds'
def tile(code):
    im=Image.new('RGB',(8,8),C[0]); d=ImageDraw.Draw(im); p=d.point; r=d.rectangle
    if code=='.': p((3,3),C[1])                                  # floor
    elif code=='w': r([0,0,7,7],C[1]); d.line([0,3,7,3],C[2]); d.line([0,7,7,7],C[2]); p((2,1),C[2]); p((6,5),C[2])  # wood planks
    elif code=='#': r([0,0,7,7],C[2]); d.line([0,3,7,3],C[3]); d.line([0,7,7,7],C[3]); d.line([3,0,3,3],C[3]); d.line([7,4,7,7],C[3])  # brick wall
    elif code=='T': r([0,0,7,7],C[3]); d.line([0,6,7,6],C[2])     # wall cap
    elif code=='g': p((1,2),C[1]); p((5,5),C[1]); p((6,1),C[1])   # grass
    elif code=='b': d.ellipse([0,0,7,7],C[2]); p((2,2),C[1]); p((5,3),C[1]); d.line([1,7,6,7],C[3])  # bush
    elif code=='t': r([0,0,7,7],C[1]); d.ellipse([0,0,7,6],C[2]); r([3,5,4,7],C[3])  # tree
    elif code=='r': r([0,0,7,7],C[3]); p((2,5),C[2])            # road
    elif code=='-': r([0,0,7,7],C[3]); d.line([1,4,6,4],C[0])   # lane line
    elif code=='s': r([0,0,7,7],C[1]); d.line([0,0,7,0],C[0]); d.line([0,0,0,7],C[0])  # sidewalk
    elif code=='W': r([0,0,7,7],C[2]); r([1,1,6,5],C[0]); d.line([1,3,6,3],C[1]); d.line([3,1,3,5],C[1])  # window
    elif code=='D': r([0,0,7,7],C[3]); r([1,1,6,7],C[2]); p((5,4),C[0])  # door
    elif code=='d': r([0,1,7,6],C[2]); d.line([0,1,7,1],C[1]); d.line([0,6,7,6],C[3])  # desk/table
    elif code=='p': r([0,2,7,5],C[2]); d.line([0,2,7,2],C[3]); d.line([0,5,7,5],C[3])  # pew
    elif code=='c': r([0,0,7,7],C[2]); p((2,2),C[1]); p((6,6),C[1]) # carpet
    elif code=='k': r([0,0,7,7],C[1]); r([0,0,3,3],C[2]); r([4,4,7,7],C[2])  # checker
    elif code=='x': r([0,0,7,7],C[2]); d.line([0,0,7,0],C[3]); d.line([0,0,0,7],C[3]); p((4,4),C[1])  # stone
    elif code=='a': r([0,0,7,7],C[1]); d.line([0,1,7,1],C[3]); d.line([0,6,7,6],C[3])  # altar
    elif code=='m': r([0,0,7,7],C[3]); r([2,1,5,3],C[1]); p((3,5),C[2])  # sign/shelf
    elif code=='~': r([0,0,7,7],C[0])                            # white
    elif code=='K': r([0,0,7,7],C[3])                            # black
    return im
SOLID=set('#TbtWDdpamK')
def grid(w,h,fill): return [[fill]*w for _ in range(h)]
def box(g,x0,y0,x1,y1,c):
    for y in range(y0,y1+1):
        for x in range(x0,x1+1): g[y][x]=c
def room(w,h,floor):
    g=grid(w,h,floor); box(g,0,0,w-1,1,'T'); box(g,0,2,w-1,3,'#')
    for y in range(h): g[y][0]=g[y][w-1]='T'
    box(g,0,h-1,w-1,h-1,'T'); return g
BGID={}
def save_bg(name,g,extra=None):
    h=len(g); w=len(g[0]); im=Image.new('RGB',(w*8,h*8))
    cache={}
    for y,row in enumerate(g):
        for x,c in enumerate(row):
            if c not in cache: cache[c]=tile(c)
            im.paste(cache[c],(x*8,y*8))
    if extra: extra(im)
    fn=f'unt_{name}.png'
    if os.environ.get('SKIP_UNT_BG')!='1': im.save(f'{BGD}/{fn}')  # art source of truth: tools/untitled_retile.py
    p=f'{BGD}/{fn}.gbsres'; i=json.load(open(p))['id'] if os.path.exists(p) else U()
    W(p,{"_resourceType":"background","id":i,"name":f"unt_{name}","symbol":f"bg_unt_{name}","tileColors":"","width":w,"height":h,"imageWidth":w*8,"imageHeight":h*8,"filename":fn,"autoColor":False})
    coll=[0x0f if c in SOLID else 0 for row in g for c in row]
    BGID[name]=(i,w,h,coll)
def logo(text,sub):
    def f(im):
        d=ImageDraw.Draw(im); big=Image.new('RGB',(64,10),C[3]); ImageDraw.Draw(big).text((4,0),text,fill=C[0])
        big=big.resize((128,20),Image.NEAREST); im.paste(big,(16,40))
        d.text((44,96),sub,fill=C[1])
    return f
# TITLE 20x18
g=grid(20,18,'K'); box(g,0,15,19,17,'x'); save_bg('title',g,logo('UNTITLED','PRESS START'))
# CURB 40x36 — city block: buildings, sidewalk, road, park
g=grid(40,36,'g')
for x in range(40): g[0][x]=g[35][x]='t'
for y in range(36): g[y][0]=g[y][39]='t'
for bx in (2,12,22,31):                 # four storefronts
    box(g,bx,2,bx+6,3,'T'); box(g,bx,4,bx+6,11,'#')
    for wy in (5,8):
        for wx in (bx+1,bx+3,bx+5): g[wy][wx]='W'
    g[11][bx+3]='D'
box(g,1,12,38,15,'s'); box(g,1,16,38,21,'r')
for x in range(2,38,3): g[18][x]='-'; g[19][x]='-'
box(g,1,22,38,24,'s')
for y,x in [(26,4),(27,9),(29,15),(31,6),(28,24),(31,30),(27,34),(32,20)]: g[y][x]='t'
for x in range(3,37,6): g[25][x]='b'
g[13][35]='m'   # bus stop sign
save_bg('curb',g)
# OFFICE 32x24
g=room(32,24,'w')
for y in (6,10,14):
    for x0 in (4,12,20):
        box(g,x0,y,x0+4,y,'d')
for x in (3,8,24,28): g[2][x]='W'
g[2][15]=g[2][16]='m'
box(g,14,23,17,23,'w'); save_bg('office',g)
# HALL 32x18
g=grid(32,18,'T'); box(g,1,4,30,5,'#')
box(g,1,6,30,12,'k'); box(g,1,13,30,13,'#')
for x in range(4,30,5): g[4][x]='W'
for y in (8,9,10): g[y][0]='k'; g[y][31]='k'
save_bg('hall',g)
# HOUSE 32x24
g=room(32,24,'.'); box(g,8,8,23,16,'c'); box(g,13,11,18,12,'d'); box(g,3,4,8,5,'d'); box(g,24,4,28,5,'d')
for x in (11,20): g[2][x]='W'
box(g,14,23,17,23,'.'); save_bg('house',g)
# NAVE 32x32
g=room(32,32,'k'); box(g,13,4,18,30,'c'); box(g,11,5,20,6,'a')
for y in range(10,28,3):
    box(g,3,y,11,y,'p'); box(g,20,y,28,y,'p')
for x in (4,8,23,27): g[2][x]='W'
for y in (15,16,17): g[y][0]='k'
save_bg('nave',g)
# CELLAR 32x24
g=room(32,24,'x')
for y in (6,7,8): g[y][28]='m'
box(g,22,4,26,5,'d'); save_bg('cellar',g)
# WHITE
save_bg('white',grid(20,18,'~'))
# ---------- variables (append) ----------
VP=R+'/project/variables.gbsres'; VD=json.load(open(VP))
VN=['VAR_VIDEO_OUT','VAR_VIDEO_SEEN','VAR_COVER_STORY','VAR_WARNED','VAR_INTOXICATED','VAR_FLAME_USED','VAR_BOUND','VAR_BITTEN','VAR_RESCUE_LATE','VAR_MATT_DOWN','VAR_CUTSCENE','VAR_SEEN_PARKER_INTRO','VAR_PLAYER_IS_MATT','VAR_WINE_COUNT','VAR_T_CUTSCENE','VAR_T_SPLIT_NIGHT','VAR_T_FLAME','VAR_CHOICE','VAR_COMBAT_READY']
byname={v['name']:v['id'] for v in VD['variables']}
used={int(v['id']) for v in VD['variables'] if v['id'].isdigit()}
nxt=max(used)+1
for n in VN:
    if n not in byname:
        while nxt in used: nxt+=1
        VD['variables'].append({"id":str(nxt),"name":n,"symbol":n.lower()}); byname[n]=str(nxt); used.add(nxt)
W(VP,VD); V={n:byname[n] for n in VN}
# ---------- event helpers ----------
def ev(cmd,args=None,children=None):
    e={"command":cmd,"args":args or {},"id":U()}
    if children is not None: e["children"]=children
    return e
num=lambda v:{"type":"number","value":v}
def cond(var,op,val): return {"type":op,"valueA":{"type":"variable","value":V[var]},"valueB":num(val)}
def IF(var,op,val,t,f=None): return ev("EVENT_IF",{"condition":cond(var,op,val),"__collapseElse":f is None,"__disableElse":f is None},{"true":t,"false":f or []})
def SET(var,val): return ev("EVENT_SET_VALUE",{"variable":V[var],"value":num(val)})
def TEXT(*lines): return ev("EVENT_TEXT",{"text":list(lines),"avatarId":"","minHeight":4,"maxHeight":7,"position":"bottom","clearPrevious":True,"showFrame":"true","speedIn":-1,"speedOut":-1,"closeWhen":"key","closeButton":"a","closeDelayTime":0.5,"closeDelayFrames":30})
def WAIT(t): return ev("EVENT_WAIT",{"time":num(t),"frames":num(int(t*60)),"units":"time"})
def SFXP(k,wait=False): return ev("EVENT_SOUND_PLAY_EFFECT",{"type":SFX[k],"priority":"medium","pitch":4,"frequency":200,"duration":0.5,"wait":wait,"effect":0})
def POS(a,x,y): return ev("EVENT_ACTOR_SET_POSITION",{"actorId":a,"x":num(x),"y":num(y)})
def SHOW(a): return ev("EVENT_ACTOR_SHOW",{"actorId":a})
def HIDE(a): return ev("EVENT_ACTOR_HIDE",{"actorId":a})
def SHAKE(t): return ev("EVENT_CAMERA_SHAKE",{"time":t,"frames":int(t*60),"units":"time","shakeDirection":"horizontal","magnitude":num(4)})
def EMOTE(a,emo): return ev("EVENT_ACTOR_EMOTE",{"actorId":a,"emoteId":emo})
def CALL(sid,**actors):
    a={"customEventId":sid}; a.update({f"$actor[{k}]$":v for k,v in actors.items()}); return ev("EVENT_CALL_CUSTOM_EVENT",a)
def GO(scene,x,y,d="down"): return ev("EVENT_SWITCH_SCENE",{"sceneId":scene,"x":num(x),"y":num(y),"direction":d,"fadeSpeed":"2"})
def DOOR(scene,x,y,d="down",extra=None,snd='door'):
    inner=[SFXP(snd,True),ev("EVENT_FADE_OUT",{"speed":"2"}),GO(scene,x,y,d)]
    if extra: inner=[IF(extra,"eq",1,inner,[SFXP('locked'),SHAKE(0.25),TEXT("The door won't budge.\nNot yet.")])]
    return [IF('VAR_CUTSCENE',"eq",0,inner)]
def COMMENT(t): return ev("EVENT_COMMENT",{"text":t,"__comment":False})
def MPLAY(k): return ev("EVENT_MUSIC_PLAY",{"musicId":MUS[k]})
MSTOP=lambda:ev("EVENT_MUSIC_STOP",{})
# ---------- scripts ----------
SD=R+'/project/scripts'
SNAMES=['SCR_StartCutscene','SCR_EndCutscene','SCR_OfficeTalk','SCR_PlayMessages','SCR_Tour','SCR_Wine','SCR_Flame','SCR_SplitNight','SCR_Rite','SCR_Status']
SID={}
for n in SNAMES:
    p=f'{SD}/{n.lower()}.gbsres'; SID[n]=json.load(open(p))['id'] if os.path.exists(p) else U()
def SCRIPT(name,body,actors=None,desc=""):
    W(f'{SD}/{name.lower()}.gbsres',{"_resourceType":"script","id":SID[name],"name":name,"description":desc,"variables":{},"actors":{k:{"id":k,"name":v} for k,v in (actors or {}).items()},"symbol":"script_"+name.lower(),"script":body})
SCRIPT('SCR_StartCutscene',[SFXP('cut_in'),ev("EVENT_SCENE_UPDATE_PAUSE",{}),SET('VAR_CUTSCENE',1)],desc="UNTITLED: Pause Logic + VAR_CUTSCENE=1. T_CUTSCENE handle reserved.")
SCRIPT('SCR_EndCutscene',[ev("EVENT_THREAD_STOP",{"variable":V['VAR_T_CUTSCENE']}),SET('VAR_CUTSCENE',0),ev("EVENT_SCENE_UPDATE_RESUME",{}),SFXP('cut_out')],desc="UNTITLED: ONLY place Resume Logic is called.")
SCRIPT('SCR_OfficeTalk',[SFXP('pc'),TEXT("A laptop. The video is\nstill queued to upload.","Maxwell: Nobody needs\nto see that. Say you\nwere at the office late."),SET('VAR_COVER_STORY',1),SET('VAR_VIDEO_SEEN',1),SFXP('flag')])
SCRIPT('SCR_PlayMessages',[SFXP('mail',True),TEXT("*BEEP* 2 new messages.","Maxwell: Bring the boy\nto bible study Sunday.","Sister: Don't go to\nthat basement. Please."),SET('VAR_WARNED',1),SFXP('flag')])
SCRIPT('SCR_Tour',[SFXP("talk"),ev("EVENT_ACTOR_MOVE_RELATIVE",{"actorId":"0","x":num(2),"y":num(0),"moveType":"horizontal","collideWith":[]}),TEXT("Maxwell: Bible study is\nupstairs.","The witches meet\ndownstairs. Ha. Kidding."),ev("EVENT_ACTOR_MOVE_RELATIVE",{"actorId":"0","x":num(-2),"y":num(0),"moveType":"horizontal","collideWith":[]})],{"0":"Maxwell"})
SCRIPT('SCR_Wine',[SFXP('wine'),TEXT("Communion wine.\nYou drink."),ev("EVENT_INC_VALUE",{"variable":V['VAR_WINE_COUNT']}),IF('VAR_WINE_COUNT',"gte",2,[SET('VAR_INTOXICATED',1),SFXP('drunk'),SHAKE(0.6),TEXT("The room tilts.")])])
SCRIPT('SCR_Flame',[SFXP('flame'),ev("EVENT_PLAYER_SET_SPRITE",{"spriteSheetId":SPR['flame']}),SHAKE(0.3),WAIT(1),ev("EVENT_PLAYER_SET_SPRITE",{"spriteSheetId":SPR['mat']}),SET('VAR_FLAME_USED',1)],desc="NO Text. COMBAT HOOK: Matt's flame will become an ability here.")
SCRIPT('SCR_SplitNight',[ev("EVENT_LOOP_WHILE",{"condition":{"type":"eq","valueA":{"type":"variable","value":V['VAR_RESCUE_LATE']},"valueB":num(0)}},{"true":[IF('VAR_BOUND',"eq",1,[SET('VAR_RESCUE_LATE',1),ev("EVENT_THREAD_STOP",{"variable":V['VAR_T_SPLIT_NIGHT']})],[WAIT(0.2)])]})],desc="T_SPLIT_NIGHT body. SILENT: no text/music/show/sfx.")
rite=[ev("EVENT_SCENE_UPDATE_PAUSE",{}),SET('VAR_CUTSCENE',1),
 SFXP('blind'),TEXT("Something soft over your\neyes. A blindfold."),
 POS("player",10,8),SFXP('bind',True),SET('VAR_BOUND',1),
 TEXT("Maxwell: Do you give\nyourself freely?"),
 ev("EVENT_CHOICE",{"variable":V['VAR_CHOICE'],"trueText":"Yes","falseText":"No"}),SFXP('choice'),
 IF('VAR_CHOICE',"eq",0,[TEXT("Maxwell: I'll ask once\nmore. Do you consent?"),ev("EVENT_CHOICE",{"variable":V['VAR_CHOICE'],"trueText":"Yes","falseText":"No"}),SFXP('choice')]),
 CALL(SID['SCR_Flame']),
 TEXT("A dagger glints in\ncandlelight."),SFXP('bite',True),SHAKE(0.5),TEXT("Teeth. At your neck."),
 SET('VAR_BITTEN',1),
 ev("EVENT_LOOP_WHILE",{"condition":{"type":"eq","valueA":{"type":"variable","value":V['VAR_RESCUE_LATE']},"valueB":num(0)}},{"true":[WAIT(0.2)]}),
 POS("0",4,10),POS("1",6,10),SHOW("0"),SHOW("1"),SFXP('rescue'),SHAKE(0.3),
 TEXT("Conner: MATT!","Parker: Get him off\nthe cross! Go!"),
 SFXP('crash',True),SET('VAR_MATT_DOWN',1),
 CALL(SID['SCR_EndCutscene']),MSTOP(),SFXP('white'),ev("EVENT_FADE_OUT",{"speed":"1"})]  # GO appended after scenes known
SCRIPT('SCR_Status',[IF('VAR_CUTSCENE',"eq",0,[SFXP('status'),IF('VAR_PLAYER_IS_MATT',"eq",1,[TEXT("WHO: Matt.")],[TEXT("WHO: Conner.")]),TEXT("WHERE: somewhere in\nthe city."),TEXT("FLAG: keep moving."),SFXP('status')])],desc="UNTITLED Start-button status. COMBAT HOOK: abilities menu goes here next pass.")
# ---------- scenes ----------
SC={}
for k in ['TITLE','CURB','OFFICE','HALL','HOUSE','NAVE','CELLAR','WHITE']:
    p=f'{R}/project/scenes/scn_unt_{k.lower()}/scene.gbsres'; SC[k]=json.load(open(p))['id'] if os.path.exists(p) else U()
rite.append(GO(SC['WHITE'],9,9))
SCRIPT('SCR_Rite',rite,{"0":"Conner","1":"Parker"},desc="SOLE cellar speaker. Ends: EndCutscene -> MusicStop -> Fade -> SCN_UNT_WHITE.")
def actor(name,spr,x,y,script=None,start=None,update=None,pal='',d='down',animate=False):
    return {"_resourceType":"actor","id":U(),"_index":0,"symbol":"actor_unt_"+name.lower(),"prefabId":"","name":name,"coordinateType":"tiles","x":x,"y":y,"frame":0,"animate":animate,"spriteSheetId":SPR[spr],"paletteId":PAL.get(pal,''),"direction":d,"moveSpeed":1,"animSpeed":15,"isPinned":False,"persistent":False,"collisionGroup":"","collisionExtraFlags":[],"prefabScriptOverrides":{},"script":script or [],"startScript":start or [],"updateScript":update or [],"hit1Script":[],"hit2Script":[],"hit3Script":[]}
def trig(name,x,y,w,h,script): return {"_resourceType":"trigger","id":U(),"name":name,"prefabId":"","x":x,"y":y,"symbol":"trigger_unt_"+name.lower(),"prefabScriptOverrides":{},"width":w,"height":h,"script":script,"leaveScript":[],"_index":0}
def talk(*lines): return [SFXP('talk'),TEXT(*lines)]
OX,OY=200,2600
def scene(key,typ,bg,script,actors=(),triggers=(),col=0,spal=('matt','conner','parker','ami'),pspr='mat'):
    folder=f'{R}/project/scenes/scn_unt_{key.lower()}'
    if os.path.exists(folder):
        import shutil; shutil.rmtree(folder)
    bid,w,h,coll=BGID[bg]
    W(folder+'/scene.gbsres',{"_resourceType":"scene","id":SC[key],"_index":0,"type":typ,"name":"UNT_"+key,"symbol":"scene_unt_"+key.lower(),"x":OX+col*420,"y":OY,"width":w,"height":h,"backgroundId":bid,"tilesetId":"","colorModeOverride":"none","paletteIds":[],"spritePaletteIds":[PAL[p] for p in spal],"autoFadeSpeed":1,"playerSpriteSheetId":SPR[pspr],"script":script,"playerHit1Script":[],"playerHit2Script":[],"playerHit3Script":[],"collisions":"" if typ=="LOGO" else compress(coll)})
    for i,a in enumerate(actors): a["_index"]=i; W(f'{folder}/actors/{a["symbol"]}.gbsres',a)
    for i,t in enumerate(triggers): t["_index"]=i; W(f'{folder}/triggers/{t["symbol"]}.gbsres',t)
def compress(arr):
    out='';last=-1;cnt=0
    for v in arr:
        if v!=last:
            if cnt==1: out+='!'
            elif cnt>0: out+=f'{cnt:x}+'
            cnt=0;last=v;out+=f'{v%256:02x}'
        cnt+=1
    out+='!' if cnt==1 else f'{cnt:x}+'
    return out
STATUS=lambda:ev("EVENT_SET_INPUT_SCRIPT",{"input":["start"],"override":True,"__scriptTabs":"press"},{"true":[CALL(SID['SCR_Status'])]})
PSPR=lambda k:ev("EVENT_PLAYER_SET_SPRITE",{"spriteSheetId":SPR[k]})
ENTER=lambda:SFXP('doorin')
scene('TITLE','LOGO','title',[MPLAY('title'),WAIT(0.5),ev("EVENT_AWAIT_INPUT",{"input":["a","b","start"]}),SFXP('start',True),ev("EVENT_FADE_OUT",{"speed":"2"}),GO(SC['CURB'],4,13,"right")],col=0)
scene('CURB','TOPDOWN','curb',[PSPR('mat'),SET('VAR_PLAYER_IS_MATT',1),MPLAY('curb'),STATUS(),COMMENT("COMBAT HOOK: overworld encounters / ability input binds go here (VAR_COMBAT_READY)."),
  IF('VAR_SEEN_PARKER_INTRO',"eq",0,[CALL(SID['SCR_StartCutscene']),SFXP('talk'),TEXT("Parker: Matt! Bus is\nhere, you coming?","Conner: He's always\nlate. Let's go."),SET('VAR_SEEN_PARKER_INTRO',1),CALL(SID['SCR_EndCutscene'])])],
  [actor('ACT_Parker','par',30,13,talk("Parker: Hurry up!\nThe bus won't wait."),pal='parker',d='left'),actor('ACT_Conner','con',32,13,talk("Conner: Office first,\nthen my place."),pal='conner',d='left'),
   actor('ACT_Bus','bus',35,17,[SFXP('bus'),TEXT("The 41 bus idles.")]),actor('ACT_Ami','ami',14,23,talk("Ami: Heard about the\nvideo? Everyone has."),pal='ami')],
  [trig('TRG_Board',34,14,4,2,DOOR(SC['OFFICE'],15,21,"up",snd='bus')),trig('TRG_Shop',5,12,1,1,[SFXP('locked'),TEXT("CLOSED.")])],col=1)
scene('OFFICE','TOPDOWN','office',[PSPR('mat'),MPLAY('office'),ENTER(),STATUS()],
  [actor('ACT_Maxwell','max',9,8,[CALL(SID['SCR_Tour'],**{"0":"$self$"})]),actor('ACT_Laptop','laptop',14,9,[CALL(SID['SCR_OfficeTalk'])])],
  [trig('TRG_ToHall',14,22,4,1,DOOR(SC['HALL'],3,9,"right"))],col=2)
scene('HALL','TOPDOWN','hall',[ENTER(),STATUS()],[actor('ACT_Janitor','max',15,7,talk("Janitor: Office west,\nthe Fugly house east."))],
  [trig('TRG_HallToOffice',0,8,1,3,DOOR(SC['OFFICE'],15,20,"up")),trig('TRG_HallToHouse',31,8,1,3,DOOR(SC['HOUSE'],15,21,"up"))],col=3)
scene('HOUSE','TOPDOWN','house',[PSPR('con'),SET('VAR_PLAYER_IS_MATT',0),MPLAY('house'),ENTER(),STATUS()],
  [actor('ACT_Father','max',5,7,talk("Father: Church tonight.\nNo excuses.")),actor('ACT_Machine','machine',26,7,[CALL(SID['SCR_PlayMessages'])],animate=True),actor('ACT_Parker','par',16,14,talk("Parker: Your dad's\nweird about that church."),pal='parker')],
  [trig('TRG_HouseExit',14,22,4,1,DOOR(SC['NAVE'],15,28,"up"))],spal=('conner','matt','parker','ami'),pspr='con',col=4)
scene('NAVE','TOPDOWN','nave',[PSPR('mat'),SET('VAR_PLAYER_IS_MATT',1),MPLAY('nave'),ENTER(),STATUS()],
  [actor('ACT_Maxwell','max',15,8,[CALL(SID['SCR_Tour'],**{"0":"$self$"})]),actor('ACT_Bottle','bottle',21,7,[CALL(SID['SCR_Wine'])])],
  [trig('TRG_LastDoor',0,15,1,3,DOOR(SC['CELLAR'],10,12,"up",extra='VAR_INTOXICATED'))],col=5)
c_con=actor('ACT_Conner','con',4,10,start=[],pal='conner'); c_par=actor('ACT_Parker','par',6,10,pal='parker')
candle=actor('ACT_Candle','candle',8,6,update=[ev("EVENT_ACTOR_SET_FRAME",{"actorId":"$self$","frame":num(1)}),WAIT(0.4),ev("EVENT_ACTOR_SET_FRAME",{"actorId":"$self$","frame":num(0)}),WAIT(0.4)])
scene('CELLAR','TOPDOWN','cellar',[PSPR('mat'),MPLAY('cellar'),ENTER(),ev("EVENT_CAMERA_LOCK",{"speed":0,"axis":["x","y"],"preventScroll":[]}),HIDE(c_con["id"]),HIDE(c_par["id"]),SET('VAR_BOUND',0),SET('VAR_RESCUE_LATE',0),
  ev("EVENT_THREAD_START",{"variable":V['VAR_T_SPLIT_NIGHT'],"__scriptTabs":"true"},{"true":[CALL(SID['SCR_SplitNight'])]}),
  CALL(SID['SCR_Rite'],**{"0":c_con["id"],"1":c_par["id"]})],
  [actor('ACT_Cross','cross',10,8),candle,c_con,c_par,actor('ACT_GameController','controller',1,5),actor('ACT_Maxwell','max',12,8)],[],col=6)
scene('WHITE','LOGO','white',[WAIT(2),COMMENT("hold — end of UNTITLED build. NEXT: grid combat abilities.")],col=7)
print('ok',SC['TITLE'],V)

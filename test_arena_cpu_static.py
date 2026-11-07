"""Static resource-event model, NOT an emulator or hardware test."""
import json,copy,operator,zipfile
from pathlib import Path
W=Path('/workspace/gbm-alltime-work');R=W/'gbm-alltime';F='265aefc8-2dd9-477a-95bc-8c0b7d2630e8';P='player'
A=json.loads((R/'project/scenes/arena/scene.gbsres').read_text());CPU=json.loads((R/'project/scenes/arena/actors/actor_2.gbsres').read_text());B=A['script'][-1]['children']['true']
class Model:
 def __init__(self):
  self.v={str(k):0 for k in range(31,228)};self.v.update({'31':3,'32':5,'215':0,'216':0})
  self.pos={P:[96,72],F:[144,72]};self.dir={P:'right',F:'left'};self.state={P:'',F:''};self.respawn=False
 def val(self,v):
  t=v['type']
  if t=='number':return v['value']
  if t=='variable':return self.v.get(v['value'],0)
  if t=='direction':return v['value']
  ops={'add':operator.add,'sub':operator.sub,'eq':operator.eq,'gt':operator.gt,'gte':operator.ge,'lt':operator.lt,'lte':operator.le,'and':lambda a,b:bool(a and b)}
  return ops[t](self.val(v['valueA']),self.val(v['valueB']))
 def run(self,events):
  for e in events:
   c=e['command'];a=e.get('args',{});kids=e.get('children',{});actor=a.get('actorId')
   if c=='EVENT_IF':self.run(kids['true'] if self.val(a['condition']) else kids['false'])
   elif c=='EVENT_IF_ACTOR_DIRECTION':self.run(kids['true'] if self.dir[actor]==self.val(a['direction']) else kids['false'])
   elif c=='EVENT_SET_VALUE':self.v[a['variable']]=self.val(a['value'])
   elif c=='EVENT_DEC_VALUE':self.v[a['variable']]=self.v.get(a['variable'],0)-1
   elif c=='EVENT_ACTOR_GET_POSITION':self.v[a['vectorX']],self.v[a['vectorY']]=self.pos[actor]
   elif c=='EVENT_ACTOR_SET_POSITION':self.pos[actor]=[self.val(a['x']),self.val(a['y'])]
   elif c=='EVENT_ACTOR_SET_DIRECTION':self.dir[actor]=self.val(a['direction'])
   elif c=='EVENT_ACTOR_SET_STATE':self.state[actor]=a['spriteStateId']
   elif c=='EVENT_SWITCH_SCENE':self.respawn=True
   elif c in ['EVENT_WAIT','EVENT_SOUND_PLAY_EFFECT','EVENT_CAMERA_SHAKE','EVENT_ACTOR_SET_ANIMATE','EVENT_ACTOR_COLLISIONS_DISABLE']:pass
   else:raise AssertionError('Unhandled model event '+c)
 def tick(self,n=1):
  for _ in range(n):self.run(CPU['updateScript'])

def run_tests():
 count=0
 def ok(cond,name):
  nonlocal count
  assert cond,name;count+=1;print('PASS',name)
 m=Model();m.tick();ok(m.pos[F][0]==143 and m.dir[F]=='left','CPU approaches left and faces player')
 m=Model();m.pos[P]=[168,72];m.pos[F]=[120,72];m.tick();ok(m.pos[F][0]==121 and m.dir[F]=='right','CPU approaches right and faces player')
 m=Model();m.pos[P]=[176,72];m.pos[F]=[160,72];m.tick(50);ok(m.pos[F]==[160,72] and m.v['32']==5,'No chase beyond safe platform / no damage from afar')
 m=Model();m.pos[P]=[100,40];m.tick(10);ok(m.pos[F]==[144,72] and m.v['32']==5,'No chase/damage at different height')
 m=Model();m.pos[P]=[120,72];m.tick();ok(m.v['215']==1 and m.v['32']==5 and m.state[F]=='CPU_PUNCH','Attack enters telegraph without instant damage')
 m.tick(5);ok(m.v['32']==5,'No damage before full six-tick windup')
 m.tick();ok(m.v['32']==4 and m.v['217']==10 and m.v['215']==2,'Impact deals exactly one and sets player invulnerability/recovery')
 m.tick(10);ok(m.v['32']==4,'Cooldown prevents rapid HP drain')
 m=Model();m.pos[P]=[120,72];m.tick();m.pos[P][1]=40;m.tick(6);ok(m.v['32']==5,'Jump during telegraph dodges impact')
 m=Model();m.pos[P]=[120,72];m.tick();m.pos[P][0]=160;m.tick(6);ok(m.v['32']==5,'Crossing behind during telegraph dodges fixed facing')
 m=Model();m.pos[P]=[120,72];m.tick();m.v['217']=9;m.tick(6);ok(m.v['32']==5,'Player invulnerability suppresses hit')
 m=Model();m.v['31']=0;pos=copy.deepcopy(m.pos);m.tick(30);ok(m.pos==pos and m.v['32']==5,'KO opponent never moves/attacks')
 m=Model();m.pos[P]=[120,72];m.dir[P]='right';m.run(B);ok(m.v['31']==2 and m.v['218']==8,'B hits in front within range')
 m.run(B);ok(m.v['31']==2,'Repeated B during cooldown does not deal additional damage')
 m=Model();m.pos[P]=[120,72];m.dir[P]='left';m.run(B);ok(m.v['31']==3,'B does not hit behind player')
 m=Model();m.pos[P]=[120,48];m.run(B);ok(m.v['31']==3,'B does not hit across vertical gap')
 m=Model();m.pos[P]=[120,72]
 for _ in range(3):m.v['218']=0;m.v['219']=0;m.run(B)
 ok(m.v['31']==0 and m.state[F]=='DEAD','Three legal B hits KO opponent')
 m=Model();m.pos[P]=[120,72];m.v['32']=1;m.tick(7);ok(m.v['32']==0 and m.respawn,'Player defeat requests local Arena reset')
 ok(CPU['script']==[] and A['playerHit1Script']==[],'Old contact auto-KO and contact HP drain removed')
 # Preservation compared to previous full delivered source.
 with zipfile.ZipFile(W/'gbm-alltime-arena-doorway-full.zip') as z:
  for rel in ['project/scenes/arena/triggers/trigger.gbsres','project/scenes/arena/triggers/gator_pit_respawn.gbsres','project/scenes/arena/actors/actor.gbsres','project/scenes/unt_title/scene.gbsres','project/scenes/scene_39/actors/actor_3.gbsres','project/scenes/scene_40/actors/pic1.gbsres','project/scenes/scene_40/actors/pic2.gbsres']:
   ok((R/rel).read_bytes()==z.read('gbm-alltime/'+rel),'Preserved '+rel)
  for name in z.namelist():
   if name.startswith('gbm-alltime/assets/engine/'):
    assert (R/name[len('gbm-alltime/'):]).read_bytes()==z.read(name),name
 ok(True,'No engine changes')
 sprite=json.loads((R/'assets/sprites/doodies/sdaction.png.gbsres').read_text());old=json.loads((W/'cpu-before/assets/sprites/doodies/sdaction.png.gbsres').read_text())
 ok(sprite['states'][:-1]==old['states'],'Original sprite animation states unmodified')
 ok(sprite['states'][-1]['name']=='CPU_PUNCH' and sprite['states'][-1]['animationType']=='horizontal' and sprite['states'][-1]['flipLeft'],'Existing punch art mirrored for CPU facing')
 print(f'{count} static model/preservation checks passed. NOT runtime/emulator verification.')
if __name__=='__main__':run_tests()

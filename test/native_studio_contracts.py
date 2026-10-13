"""Verify distinct studio APIs, independent learners and real file behavior."""
import datetime,hashlib,importlib.util,json,os,re,shutil,signal,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CC=shutil.which(os.environ.get('CSE_CC','cc'));assert CC
TASK=os.environ.get('CSE_PARENT_TASK_ID','c17-studio-native-contracts')
LOG=Path(os.environ.get('CSE_VALIDATION_LOG',ROOT/'build/native-studio-processes.jsonl'));LOG.parent.mkdir(parents=True,exist_ok=True)
spec=importlib.util.spec_from_file_location('learner',ROOT/'test/studio_learner_completions.py');learner=importlib.util.module_from_spec(spec);spec.loader.exec_module(learner)
PACKS={
'CSE-01-systems-build-14':('cse14_sensor_frame','sensor.c'),
'CSE-02-systems-build-15':('cse15_region_planner','regions.c'),
'CSE-03-systems-build-16':('cse16_stream_decoder','stream.c'),
'CSE-04-systems-build-17':('cse17_window_aggregator','window.c')}
counts=dict(packs=4,sourceRoles=8,completedLearnerRoles=0,debugProfiles=0,releaseProfiles=0,directBuilds=0,unfinishedCasesRejected=0,acceptedContracts=0,cliCases=0,fileCases=0,controlledRegressionsRejected=0)
def run(args,cwd,expected=0,timeout=120,quiet=False):
 args=[str(a) for a in args];started=datetime.datetime.now(datetime.timezone.utc).isoformat();child=subprocess.Popen(args,cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.PIPE,start_new_session=True);cleanup='process exited'
 try:out,err=child.communicate(timeout=timeout)
 except BaseException:
  os.killpg(child.pid,signal.SIGTERM)
  try:child.communicate(timeout=5)
  except subprocess.TimeoutExpired:os.killpg(child.pid,signal.SIGKILL);child.communicate()
  cleanup='child process group terminated';raise
 finally:
  with LOG.open('a') as f:f.write(json.dumps(dict(parentTaskId=TASK,cwd=str(cwd),command=args,pid=child.pid,startTime=started,endTime=datetime.datetime.now(datetime.timezone.utc).isoformat(),exitCode=child.returncode,timeoutSeconds=timeout,childProcessGroupCleanup=cleanup))+'\n')
 assert child.returncode!=0 if expected=='failure' else child.returncode==expected,(args,child.returncode,(out+err).decode(errors='replace'))
 if quiet:assert not out and err,(args,out,err)
 return out,err
def identity():return {str(f.relative_to(ROOT)):hashlib.sha256(f.read_bytes()).hexdigest() for pack in PACKS for f in (ROOT/pack).rglob('*') if f.is_file() and not any(v.startswith('build') for v in f.relative_to(ROOT/pack).parts)}
def completed(pack,path):
 shutil.copytree(ROOT/pack/'starter',path);file=path/PACKS[pack][1];text=file.read_text()
 for marker,body in learner.COMPLETIONS[pack].items():
  pattern=r'[ \t]*/\* TODO\s+'+marker+r':.*?\*/\s*.*?return\s+[A-Z_]+UNFINISHED;'
  match=re.search(pattern,text,re.S);assert match and '\n}' not in match.group(0),(pack,marker)
  text,n=re.subn(pattern,lambda _:body,text,count=1,flags=re.S);assert n==1
 file.write_text(text)
 for other in (ROOT/pack/'starter').iterdir():
  if other.is_file() and other.name!=file.name:assert (path/other.name).read_bytes()==other.read_bytes()
def cli(exe,args,cwd,expected=0,quiet=False):counts['cliCases']+=1;return run([exe,*args],cwd,expected=expected,timeout=15,quiet=quiet)[0]
def crc(data):
 remainder=0
 for byte in data:
  value=(remainder^byte)<<8
  for bit in range(15,7,-1):
   if value&(1<<bit):value^=0x107<<(bit-8)
  remainder=value
 return remainder
def frame(payload):
 data=bytes([0xA5,1,len(payload)])+payload;return data+bytes([crc(data)])
STREAM=bytes.fromhex('A501031000FF53A501009D')
STREAM_OUTPUT=b'frame 1 (3 bytes): 10 00 FF\nframe 2 (0 bytes):\nvalid frames: 2\n'
WINDOW_INPUT=b'timestamp_ms,temp_milli_c\n999,-1\n1000,1000\n1500,2000\n2001,60001\n'
WINDOW_HEADER=b'bucket_ms,count,mean_milli_c,min_milli_c,max_milli_c\n'
WINDOW_OUTPUT=WINDOW_HEADER+b'0,1,-1,-1,-1\n1000,2,1500,1000,2000\n2000,1,60001,60001,60001\n'
def file_checks(pack,exe,path):
 if pack.endswith('16'):
  good=[b'',STREAM,frame(bytes(range(32))),frame(b'')*9,frame(b'ABC')+frame(b'\x00')]
  for i,data in enumerate(good):
   file=path/f'valid stream {i}.bin';file.write_bytes(data);out=cli(exe,[file],path);assert file.read_bytes()==data and b'valid frames:' in out;counts['fileCases']+=1
  bad=[frame(bytes(range(32)))[:cut] for cut in range(1,36)]+[STREAM[:-1],STREAM+bytes([0xA5]),b'X']
  for position in range(len(STREAM)):
   changed=bytearray(STREAM);changed[position]^=1;bad.append(bytes(changed))
  for i,data in enumerate(bad):
   file=path/f'bad stream {i}.bin';file.write_bytes(data);cli(exe,[file],path,expected=1,quiet=True);assert file.read_bytes()==data;counts['fileCases']+=1
  expected_sample=STREAM
 else:
  good=[WINDOW_INPUT,WINDOW_INPUT.replace(b'\n',b'\r\n'),WINDOW_INPUT.rstrip(b'\n'),b'timestamp_ms,temp_milli_c\n',b'timestamp_ms,temp_milli_c\n0,-1\n0,0\n',b'timestamp_ms,temp_milli_c\n4294967295,125000\n',b'timestamp_ms,temp_milli_c\n'+b'0,125000\n'*256]
  for i,data in enumerate(good):
   file=path/f'valid samples {i}.csv';file.write_bytes(data);out=cli(exe,[file],path);assert file.read_bytes()==data and out.startswith(WINDOW_HEADER)
   if i<3:assert out==WINDOW_OUTPUT
   if i==3:assert out==WINDOW_HEADER
   if i==4:assert out==WINDOW_HEADER+b'0,2,0,-1,0\n'
   if i==5:assert out==WINDOW_HEADER+b'4294967000,1,125000,125000,125000\n'
   counts['fileCases']+=1
  bad=[b'',b'wrong\n1,0\n',b'timestamp_ms,temp_milli_c\n1,0\n0,0\n',b'timestamp_ms,temp_milli_c\n1,125001\n',b'timestamp_ms,temp_milli_c\n1,+0\n',b'timestamp_ms,temp_milli_c\n1,0,1\n',b'timestamp_ms,temp_milli_c\n1,0\x00junk\n',b'timestamp_ms,temp_milli_c\x00x\n',b'timestamp_ms,temp_milli_c\n'+b'0'*96+b',0\n',b'timestamp_ms,temp_milli_c\n'+b'0,0\n'*257]
  for i,data in enumerate(bad):
   file=path/f'bad samples {i}.csv';file.write_bytes(data);cli(exe,[file],path,expected=1,quiet=True);assert file.read_bytes()==data;counts['fileCases']+=1
  file=path/'width samples.csv';file.write_bytes(WINDOW_INPUT);out=cli(exe,[file,'2000'],path);assert out==WINDOW_HEADER+b'0,3,999,-1,2000\n2000,1,60001,60001,60001\n';counts['fileCases']+=1
  for width in ['0','-1','+1','1x','4294967296','']:
   cli(exe,[file,width],path,expected=2,quiet=True);assert file.read_bytes()==WINDOW_INPUT;counts['fileCases']+=1
  expected_sample=WINDOW_INPUT
 existing=path/'keep original.input';existing.write_bytes(b'keep original');cli(exe,['--sample',existing],path,expected=1,quiet=True);assert existing.read_bytes()==b'keep original';counts['fileCases']+=1
 new=path/'new sample.input';cli(exe,['--sample',new],path);assert new.read_bytes()==expected_sample;counts['fileCases']+=1
 cli(exe,[path/'missing.input'],path,expected=1,quiet=True);counts['fileCases']+=1
def exercise(pack,exe,path):
 output=cli(exe,[],path)
 if pack.endswith('14'):
  assert output==b'frame: 0xAABC\nbytes (little endian): BC AA\nbits: 1010101010111100\nchannel: 10\nADC count: 2748\nvoltage: 2215 mV\n'
  for word in [0,1,4095,4096,65535]:
   for ref in [1,3300,5000]:
    out=cli(exe,[str(word),str(ref)],path);assert f'channel: {word//4096}\n'.encode() in out and f'voltage: {(word%4096*ref+2047)//4095} mV\n'.encode() in out
  for args in [['65536'],['-1'],['1junk'],[''],['1','0'],['1','5001'],['1','2','3']]:cli(exe,args,path,expected=2,quiet=True)
 elif pack.endswith('15'):
  assert b'total=24 padding=5\n' in output and b'byte 11: padding\n' in output and b'byte 24: outside\n' in output;cli(exe,['extra'],path,expected=2,quiet=True)
 elif pack.endswith('16'):assert output==STREAM_OUTPUT;file_checks(pack,exe,path)
 else:assert output==WINDOW_OUTPUT;file_checks(pack,exe,path)
before=identity()
mutations={
'CSE-01-systems-build-14':(r'\(product\s*\+\s*UINT32_C\(2047\)\)',r'product'),
'CSE-02-systems-build-15':(r'return REGION_PADDING;',r'return REGION_OUTSIDE;'),
'CSE-03-systems-build-16':(r'\? STREAM_OK\s*:\s*STREAM_TRUNCATED',r'? STREAM_OK : STREAM_OK'),
'CSE-04-systems-build-17':(r'samples\[i\]\.timestamp_ms\s*-\s*samples\[i\]\.timestamp_ms\s*%\s*width',r'samples[i].timestamp_ms % width')}
with tempfile.TemporaryDirectory(prefix='c17-studios-',dir=os.environ.get('TMPDIR')) as directory:
 temp=Path(directory)
 for pack,(prefix,primary) in PACKS.items():
  copies={}
  for kind in ['starter','solution','completed']:
   path=temp/pack/kind
   if kind=='completed':completed(pack,path);counts['completedLearnerRoles']+=1
   else:shutil.copytree(ROOT/pack/kind,path)
   copies[kind]=path;role='solution' if kind=='solution' else 'starter';target=prefix+'_'+role
   for profile in ['Debug','Release']:
    build=path/('build-'+profile.lower());run(['cmake','-S',path,'-B',build,'-DCMAKE_BUILD_TYPE='+profile,'-DCMAKE_C_COMPILER='+CC,'-DCSE_SANITIZERS='+('ON' if profile=='Debug' else 'OFF')],path)
    run(['cmake','--build',build,'--parallel','2'],path);counts['debugProfiles' if profile=='Debug' else 'releaseProfiles']+=1
    exe=build/target
    if kind=='starter':
     run(['ctest','--test-dir',build,'--output-on-failure'],path,expected='failure');counts['unfinishedCasesRejected']+=1;cli(exe,[],path,expected=78,quiet=True)
    else:
     run(['ctest','--test-dir',build,'--output-on-failure'],path);counts['acceptedContracts']+=1;cases=path/('cases-'+profile.lower());cases.mkdir();exercise(pack,exe,cases)
   direct=path/'direct';run([CC,'-std=c17','-Wall','-Wextra','-Wpedantic','-Wconversion','-Werror','-I.',primary,'main.c','-o',direct],path);counts['directBuilds']+=1
   if kind=='starter':cli(direct,[],path,expected=78,quiet=True)
   else:cli(direct,[],path)
  reference=copies['solution'];file=reference/primary;original=file.read_text();pattern,replacement=mutations[pack];changed,n=re.subn(pattern,replacement,original,count=1);assert n==1,(pack,pattern);file.write_text(changed)
  build=reference/'build-release';run(['cmake','--build',build,'--clean-first','--parallel','2'],reference);run(['ctest','--test-dir',build,'--output-on-failure'],reference,expected='failure');counts['controlledRegressionsRejected']+=1;file.write_text(original)
 assert before==identity()
assert not Path(directory).exists()
print(json.dumps(dict(event='verified-c17-studio-contracts',compiler=CC,**counts,sourceFilesUnchanged=True,temporaryBuildCleanup=True)))

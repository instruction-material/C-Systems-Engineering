"""Verify six complete native labs, independent learners, real files and regressions."""
import datetime,hashlib,importlib.util,json,os,re,shutil,signal,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CC=shutil.which(os.environ.get('CSE_CC','cc'));assert CC
TASK=os.environ.get('CSE_PARENT_TASK_ID','c17-systems-native-contracts')
LOG=Path(os.environ.get('CSE_VALIDATION_LOG',ROOT/'build/native-systems-processes.jsonl'));LOG.parent.mkdir(parents=True,exist_ok=True)
spec=importlib.util.spec_from_file_location('completions',ROOT/'test/systems_learner_completions.py');completion=importlib.util.module_from_spec(spec);spec.loader.exec_module(completion)
PACKS={
 'CSE4-Memory-Visualizer':('cse4_memory_visualizer','layout.c'),
 'CSE5-Byte-Buffer-Workbench':('cse5_byte_buffer_workbench','buffers.c'),
 'CSE6-Packet-Serializer-Deserializer':('cse6_packet_serializer','packet.c'),
 'CSE7-Dynamic-Ring-Buffer':('cse7_dynamic_ring_buffer','ring.c'),
 'CSE8-Fixed-Size-Log-File-Reader':('cse8_fixed_log_reader','log.c'),
 'CSE9-Capstone-Telemetry-Transform-CLI':('cse9_capstone_transform','telemetry.c')}
counts=dict(packs=6,sourceRoles=12,completedLearnerRoles=0,debugProfiles=0,releaseProfiles=0,abiProfiles=0,directBuilds=0,unfinishedCasesRejected=0,acceptedContracts=0,cliCases=0,fileCases=0,controlledRegressionsRejected=0)
def run(args,cwd,expected=0,timeout=120,quiet_failure=False):
 args=[str(x) for x in args];started=datetime.datetime.now(datetime.timezone.utc).isoformat()
 child=subprocess.Popen(args,cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.PIPE,start_new_session=True)
 cleanup='process exited'
 try:out,err=child.communicate(timeout=timeout)
 except BaseException:
  os.killpg(child.pid,signal.SIGTERM)
  try:child.communicate(timeout=5)
  except subprocess.TimeoutExpired:os.killpg(child.pid,signal.SIGKILL);child.communicate()
  cleanup='child process group terminated';raise
 finally:
  with LOG.open('a') as f:f.write(json.dumps(dict(parentTaskId=TASK,cwd=str(cwd),command=args,pid=child.pid,startTime=started,endTime=datetime.datetime.now(datetime.timezone.utc).isoformat(),exitCode=child.returncode,timeoutSeconds=timeout,childProcessGroupCleanup=cleanup))+'\n')
 text=(out+err).decode(errors='replace')
 assert child.returncode!=0 if expected=='failure' else child.returncode==expected,(args,child.returncode,text)
 if quiet_failure:assert not out and err,(args,out,err)
 return out.decode(errors='replace'),text

def identity():
 return {str(f.relative_to(ROOT)):hashlib.sha256(f.read_bytes()).hexdigest() for pack in PACKS for f in sorted((ROOT/pack).rglob('*')) if f.is_file() and not any(p.startswith('build') for p in f.relative_to(ROOT/pack).parts)}

def completed_copy(pack,destination):
 shutil.copytree(ROOT/pack/'starter',destination)
 primary=destination/PACKS[pack][1];original=primary.read_text();text=original
 for marker,body in completion.COMPLETIONS[pack].items():
  pattern=r'[ \t]*/\* TODO\s+'+marker+r':.*?\*/\s*.*?return\s+(?:[A-Z_]+UNFINISHED|UINT_MAX);'
  match=re.search(pattern,text,re.S);assert match and '\n}' not in match.group(0),(pack,marker)
  text,n=re.subn(pattern,lambda _:body.strip(),text,count=1,flags=re.S);assert n==1,(pack,marker)
 primary.write_text(text)
 for f in (ROOT/pack/'starter').iterdir():
  if f.is_file() and f.name!=primary.name:assert (destination/f.name).read_bytes()==f.read_bytes()
 assert text!=original

INPUT_HEADER='sample_id,temp_milli_c,voltage_mv,flags'
OUTPUT_HEADER='sample_id,temp_milli_c,temp_milli_f,voltage_mv,flags_hex,status\n'
SAMPLE_INPUT=(INPUT_HEADER+'\n7,21500,3300,5\n8,19875,3250,1\n9,25000,3400,129\n').encode()
SAMPLE_OUTPUT=OUTPUT_HEADER+'7,21500,70700,3300,0x05,normal\n8,19875,67775,3250,0x01,normal\n9,25000,77000,3400,0x81,normal\n'
SAMPLE_LOG=bytes.fromhex('E903000001072AC6F003000002072CDA')
def checksum(data):return sum((sum((b>>bit)&1 for b in data)%2)*(1<<bit) for bit in range(8))
def log_record(timestamp,level,sensor,reading):
 content=timestamp.to_bytes(4,'little')+bytes([level,sensor,reading]);return content+bytes([checksum(content)])
def telemetry_output(rows):
 result=OUTPUT_HEADER
 for identity,celsius,voltage,flags in rows:
  magnitude=abs(celsius)*9//5;fahrenheit=(-magnitude if celsius<0 else magnitude)+32000
  status='cold' if celsius<0 else 'hot' if celsius>60000 else 'normal'
  result+=f'{identity},{celsius},{fahrenheit},{voltage},0x{flags:02X},{status}\n'
 return result

def cli(exe,args,cwd,expected=0,quiet=False):
 counts['cliCases']+=1;return run([exe,*args],cwd,expected=expected,timeout=15,quiet_failure=quiet)

def file_checks(pack,exe,cwd):
 if pack.startswith('CSE8'):
  valid=[b'',SAMPLE_LOG,log_record(0,0,0,0),b''.join(log_record(t,l,s,r) for t,l,s,r in [(255,255,0,1),(256,0,255,2),(0x80000000,7,7,7),(0xFFFFFFFF,255,255,255)])]
  for n,data in enumerate(valid):
   path=cwd/f'input {n}.logbin';path.write_bytes(data)
   out,_=cli(exe,[path],cwd);assert path.read_bytes()==data and f'valid records: {len(data)//8}\n' in out;counts['fileCases']+=1
  bad=[SAMPLE_LOG[:n] for n in range(1,8)]+[SAMPLE_LOG[:8]+b'X']
  for position in range(8):
   data=bytearray(SAMPLE_LOG);data[position]^=1;bad.append(bytes(data))
  for n,data in enumerate(bad):
   path=cwd/f'bad-{n}.logbin';path.write_bytes(data);cli(exe,[path],cwd,1,True);assert path.read_bytes()==data;counts['fileCases']+=1
  new=cwd/'sample log.bin';cli(exe,['--sample',new],cwd);assert new.read_bytes()==SAMPLE_LOG
  cli(exe,['--sample',new],cwd,1,True);assert new.read_bytes()==SAMPLE_LOG;counts['fileCases']+=2
  cli(exe,[cwd/'missing.bin'],cwd,1,True);cli(exe,[cwd],cwd,1,True)
 elif pack.startswith('CSE9'):
  cases=[([], '\n',True),([(0,-40000,0,0),(255,125000,5000,255)],'\n',True),([(1,-1,0,255),(2,60000,5000,0),(3,60001,1,1)],'\r\n',False)]
  for n,(rows,ending,final) in enumerate(cases):
   lines=[INPUT_HEADER,*[','.join(str(v) for v in row) for row in rows]];data=(ending.join(lines)+(ending if final else '')).encode()
   source=cwd/f'input {n}.csv';out=cwd/f'output {n}.csv';source.write_bytes(data)
   cli(exe,[source,out],cwd);assert source.read_bytes()==data and out.read_text()==telemetry_output(rows);counts['fileCases']+=1
   cli(exe,[source,out],cwd,1,True);assert source.read_bytes()==data and out.read_text()==telemetry_output(rows);counts['fileCases']+=1
   cli(exe,[source,source],cwd,1,True);assert source.read_bytes()==data;counts['fileCases']+=1
  malformed=[b'',b'wrong\n', (INPUT_HEADER+'\n999,0,0,0\n').encode(),(INPUT_HEADER+'\n1,0,0,0\n2,125001,0,0\n').encode(),(INPUT_HEADER+'\n1,0,0,0trailing\n').encode(),(INPUT_HEADER+'\n1,0,0,0,5\n').encode(),(INPUT_HEADER+'\n\n').encode(),(INPUT_HEADER+'\n1,+1,0,0\n').encode(),(INPUT_HEADER+'\n-1,0,0,0\n').encode(),(INPUT_HEADER+'\n1,0,5001,0\n').encode(),(INPUT_HEADER+'\n1,0,0,256\n').encode(),(INPUT_HEADER+'\n'+('0'*128)+'\n').encode(),(INPUT_HEADER+'\n1,0,0,0').encode()+b'\x00hidden\n']
  for n,data in enumerate(malformed):
   source=cwd/f'bad-{n}.csv';out=cwd/f'absent-{n}.csv';source.write_bytes(data)
   cli(exe,[source,out],cwd,1,True);assert source.read_bytes()==data and not out.exists();counts['fileCases']+=1
  new=cwd/'sample input.csv';cli(exe,['--sample',new],cwd);assert new.read_bytes()==SAMPLE_INPUT
  cli(exe,['--sample',new],cwd,1,True);assert new.read_bytes()==SAMPLE_INPUT;counts['fileCases']+=2
  cli(exe,[cwd/'missing.csv',cwd/'not-created.csv'],cwd,1,True);assert not (cwd/'not-created.csv').exists()

before=identity();assert len(before)==74,len(before)
references={}
with tempfile.TemporaryDirectory(prefix='c17-systems-') as directory:
 temporary=Path(directory)
 for pack,(target,primary) in PACKS.items():
  for role,finished in [('starter',False),('solution',False),('starter',True)]:
   folder=temporary/(pack+'-'+role+('-completed' if finished else ''))
   if finished:completed_copy(pack,folder);counts['completedLearnerRoles']+=1
   else:shutil.copytree(ROOT/pack/role,folder)
   name=target+'_'+role
   for profile in ['Debug','Release']:
    build=folder/('build-'+profile)
    args=['cmake','-S',folder,'-B',build,'-DCMAKE_BUILD_TYPE='+profile,'-DCMAKE_C_COMPILER='+CC]
    if profile=='Debug':args+=['-DCSE_SANITIZERS=ON']
    run(args,ROOT);run(['cmake','--build',build],ROOT)
    exe=build/name
    if role=='starter' and not finished:
     run(['ctest','--test-dir',build,'--output-on-failure'],ROOT,'failure')
     cli(exe,[],folder,78,True);counts['unfinishedCasesRejected']+=1
    else:
     run(['ctest','--test-dir',build,'--output-on-failure'],ROOT);counts['acceptedContracts']+=1
     initial=set(folder.iterdir());out,_=cli(exe,[],folder);assert set(folder.iterdir())==initial
     if pack.startswith('CSE4'):assert 'array index gap   : 1\n' in out and 'FrameRecord:' in out and 'ExperimentRecord:' in out
     elif pack.startswith('CSE5'):assert 'label text: sensor-\n' in out and 'terminated packet prefix: 2 bytes; complete packet: 8 bytes\n' in out
     elif pack.startswith('CSE6'):assert 'C3 01 02 05 E4 0C 2D ' in out
     elif pack.startswith('CSE7'):assert 'contents: 20 30 40 50 60 70 \n' in out and 'capacity: 8\n' in out
     elif pack.startswith('CSE8'):assert 'valid records: 2\n' in out
     else:assert out==SAMPLE_OUTPUT
     cases=folder/('cases-'+profile);cases.mkdir();file_checks(pack,exe,cases)
    counts['debugProfiles' if profile=='Debug' else 'releaseProfiles']+=1
   # A direct compiler command uses the same supplied sources and no CMake state.
   direct=folder/'direct-demo';sources=['main.c',primary]+(['allocation.c'] if pack.startswith('CSE7') else [])
   run([CC,'-std=c17','-Wall','-Wextra','-Wpedantic','-Wconversion','-Werror',*sources,'-o',direct],folder)
   cli(direct,[],folder,78 if role=='starter' and not finished else 0,role=='starter' and not finished);counts['directBuilds']+=1
   if role=='solution':references[pack]=folder
   if pack.startswith('CSE4') and (role=='solution' or finished):
    packed=folder/'build-packed';run(['cmake','-S',folder,'-B',packed,'-DCMAKE_C_COMPILER='+CC,'-DCMAKE_C_FLAGS=-fpack-struct=1'],ROOT);run(['cmake','--build',packed],ROOT);run(['ctest','--test-dir',packed,'--output-on-failure'],ROOT);counts['abiProfiles']+=1
 for pack,folder in references.items():
  source=folder/PACKS[pack][1];original=source.read_text()
  changes={
   'CSE4-Memory-Visualizer':(r'report\.padding_bytes\s*\+=\s*report\.tail;', '/* regression: omitted tail padding */'),
   'CSE5-Byte-Buffer-Workbench':(r'copied\s*<\s*capacity\s*-\s*1','copied < capacity'),
   'CSE6-Packet-Serializer-Deserializer':(r'length\s*!=\s*PACKET_SIZE','length < PACKET_SIZE'),
   'CSE7-Dynamic-Ring-Buffer':(r'reverse\(\s*grown\s*,\s*0\s*,\s*buffer->capacity\s*\);','/* regression: omitted final rotation */'),
   'CSE8-Fixed-Size-Log-File-Reader':(r'if\s*\(log_checksum\(bytes,\s*7\)\s*!=\s*\(unsigned int\)bytes\[7\]\)\s*return LOG_INVALID;','/* regression: skipped checksum validation */'),
   'CSE9-Capstone-Telemetry-Transform-CLI':(r'>\s*60000\s*\?','> 60001 ?')}
  pattern,new=changes[pack];mutated,n=re.subn(pattern,new,original,count=1);assert n==1,(pack,pattern)
  source.write_text(mutated);build=folder/'build-Release';run(['cmake','--build',build,'--clean-first'],ROOT);run(['ctest','--test-dir',build,'--output-on-failure'],ROOT,'failure');counts['controlledRegressionsRejected']+=1
  source.write_text(original)
assert identity()==before
print(json.dumps(dict(event='verified-c17-systems-contracts',compiler=CC,**counts,sourceFilesUnchanged=True,temporaryBuildCleanup=True)))

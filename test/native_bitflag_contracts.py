"""Verify every byte, independent learner completion, CLI grammar and mutations."""
import datetime,hashlib,json,os,re,shutil,signal,subprocess,tempfile,time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
PACK=ROOT/"CSE2-Bitflag-Configuration-Parser"
CC=shutil.which(os.environ.get("CSE_CC","cc"));assert CC
TASK=os.environ.get("CSE_PARENT_TASK_ID","c17-bitflag-native-contracts")
LOG=Path(os.environ.get("CSE_VALIDATION_LOG",ROOT/"build/native-bitflag-processes.jsonl"));LOG.parent.mkdir(parents=True,exist_ok=True)

def run(args,cwd,expected=0,contains=None,timeout=120):
 args=[str(v) for v in args];start=datetime.datetime.now(datetime.timezone.utc).isoformat()
 child=subprocess.Popen(args,cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.PIPE,start_new_session=True)
 cleanup="process exited"
 try:out,err=child.communicate(timeout=timeout)
 except BaseException:
  os.killpg(child.pid,signal.SIGTERM)
  try:child.communicate(timeout=5)
  except subprocess.TimeoutExpired:os.killpg(child.pid,signal.SIGKILL);child.communicate()
  cleanup="child process group terminated";raise
 finally:
  with LOG.open("a") as f:f.write(json.dumps(dict(parentTaskId=TASK,cwd=str(cwd),command=args,pid=child.pid,startTime=start,endTime=datetime.datetime.now(datetime.timezone.utc).isoformat(),exitCode=child.returncode,timeoutSeconds=timeout,childProcessGroupCleanup=cleanup))+"\n")
 text=(out+err).decode(errors="replace")
 assert (child.returncode!=0 if expected=="failure" else child.returncode==expected),(args,child.returncode,text)
 if contains:assert contains in text,(args,contains,text)
 return out.decode(errors="replace"),text

SCAFFOLDS=[
"""    /* TODO: guard bit 0..3, then return its 0/1 state; invalid bits return -1. */
    (void)config;
    (void)bit;
    return -2;""",
"""    /* TODO: extract exactly the mode bits at positions 4 and 5. */
    (void)config;
    return UINT_MAX;""",
"""    /* TODO: validate before shifting or writing, then set/clear only the flag. */
    (void)original;
    (void)bit;
    (void)enabled;
    (void)updated;
    return false;"""]
COMPLETIONS=[
"""    if (bit >= 4u) return -1;
    unsigned int divisor = 1;
    for (unsigned int index = 0; index < bit; ++index) divisor *= 2u;
    return (int)(((unsigned int)config / divisor) % 2u);""",
"""    return ((unsigned int)config / 16u) % 4u;""",
"""    if (bit >= 4u || updated == NULL) return false;
    unsigned int divisor = 1;
    for (unsigned int index = 0; index < bit; ++index) divisor *= 2u;
    const int existing = (int)(((unsigned int)original / divisor) % 2u);
    const int changed = (int)original + ((enabled ? 1 : 0) - existing) * (int)divisor;
    *updated = (uint8_t)changed;
    return true;"""]
INVALID=[""," ","\t","-1","+1","1 "," 1","1\t","1\n","garbage","12junk","0x","0X","0xG","0xFFjunk","256","0x100","0b1","0o77","1.0","1e2","FF","0x-1","0x+1","999999999999999999999999","18446744073709551615","18446744073709551616","0xFFFFFFFFFFFFFFFF"]
VALID=[(str(value),value) for value in range(256)]+[(None,39),("077",77),("0x00FF",255),("0X0f",15),("000",0)]
def expected_output(value):
 flags=[value//(2**bit)%2 for bit in range(4)];mode=value//16%4;reserved=value//64*64
 bits=f"{value:08b}";off=value-flags[0];on=value+(1-flags[0])
 text=("bitflag configuration parser\n"+f"config byte : 0x{value:02X}\n"+f"binary      : {bits[:4]}_{bits[4:]}\n"+
       f"logging     : {'on' if flags[0] else 'off'}\n"+
       f"checksum    : {'enabled' if flags[1] else 'disabled'}\n"+
       f"ack         : {'required' if flags[2] else 'optional'}\n"+
       f"encryption  : {'set' if flags[3] else 'clear'} (flag only)\n"+
       f"mode        : {['safe','balanced','fast','diagnostic'][mode]} ({mode})\n"+
       f"reserved    : 0x{reserved:02X}\n"+f"logging off : 0x{off:02X}\n"+f"logging on  : 0x{on:02X}\n")
 return text+("warning     : reserved bits are set; no behavior is assigned to them.\n" if reserved else "")
def identity():
 return {str(path.relative_to(PACK)):hashlib.sha256(path.read_bytes()).hexdigest()
         for path in sorted(PACK.rglob("*")) if path.is_file() and not any(part.startswith("build") for part in path.relative_to(PACK).parts)}
before=identity();assert len(before)==15
counts=dict(roles=0,bytePatterns=0,modelChecks=0,argumentChecks=0,validCliCases=0,invalidCliCases=0,unfinishedCasesRejected=0,completedLearnerRoles=0,controlledRegressionsRejected=0)
with tempfile.TemporaryDirectory(prefix="c17-bitflag-") as directory:
 temp=Path(directory);reference=None
 for role,completed in [("starter",False),("solution",False),("starter",True)]:
  folder=temp/(role+("-completed" if completed else ""));shutil.copytree(PACK/role,folder,ignore=shutil.ignore_patterns("build","build-*"))
  if completed:
   target=folder/"configuration.c";text=target.read_text()
   for scaffold,completion in zip(SCAFFOLDS,COMPLETIONS):assert text.count(scaffold)==1;text=text.replace(scaffold,completion)
   target.write_text(text)
   assert all((folder/name).read_bytes()==(PACK/role/name).read_bytes() for name in ["main.c","input.c","input.h","configuration.h","tests.c","CMakeLists.txt"])
  name="cse2_bitflag_parser_"+role;build=folder/"build"
  run(["cmake","-S",folder,"-B",build,"-DCMAKE_BUILD_TYPE=Debug","-DCMAKE_C_COMPILER="+CC,"-DCSE_SANITIZERS=ON"],ROOT)
  run(["cmake","--build",build,"--parallel","2"],ROOT)
  unfinished=role=="starter" and not completed
  run(["ctest","--test-dir",build,"--output-on-failure"],ROOT,expected="failure" if unfinished else 0)
  if unfinished:
   for argument in [None,"0","255"]:
    out,_=run([build/name]+([] if argument is None else [argument]),folder,expected=78,contains="Unfinished");assert out==""
    counts["unfinishedCasesRejected"]+=1
  else:
   out,_=run([build/(name+"_tests")],folder)
   assert out=="256 byte patterns, 5376 model checks and 98 argument checks passed\n",out
   counts["bytePatterns"]+=256;counts["modelChecks"]+=5376;counts["argumentChecks"]+=98
   for argument,value in VALID:
    out,_=run([build/name]+([] if argument is None else [argument]),folder);assert out==expected_output(value),(argument,out)
    counts["validCliCases"]+=1
  for argument in INVALID:
   out,_=run([build/name,argument],folder,expected=2,contains="Input must be");assert out=="";counts["invalidCliCases"]+=1
  out,_=run([build/name,"39","extra"],folder,expected=2,contains="Usage:");assert out=="";counts["invalidCliCases"]+=1
  if completed:counts["completedLearnerRoles"]+=1
  else:counts["roles"]+=1
  if role=="solution":reference=(folder,build)
 folder,build=reference
 mutations=[
  ("configuration.c","(UINT32_C(1) << bit)","(UINT32_C(1) << (bit + 1u))","flag decoding"),
  ("configuration.c",">> 4u",">> 3u","mode extraction"),
  ("configuration.c","value & ~mask","value & mask","flag update preserves other fields"),
  ("configuration.c","if (bit >= 4u || updated == NULL)","if (bit >= 5u || updated == NULL)","flag update guard"),
  ("input.c","value > (UINT8_MAX - digit) / base","value > (UINT32_C(1000000) - digit) / base","invalid input accepted"),
  ("input.c","uint32_t base = 10;","uint32_t base = 8;","valid input rejected or misread")]
 for filename,correct,regression,failure in mutations:
  path=folder/filename;original=path.read_text();assert original.count(correct)==1
  path.write_text(original.replace(correct,regression))
  run(["cmake","--build",build,"--clean-first","--parallel","2"],ROOT)
  run(["ctest","--test-dir",build,"--output-on-failure"],ROOT,expected="failure",contains=failure)
  counts["controlledRegressionsRejected"]+=1;path.write_text(original)
 run(["cmake","--build",build,"--clean-first","--parallel","2"],ROOT)
 run(["ctest","--test-dir",build,"--output-on-failure"],ROOT)
assert identity()==before
print(json.dumps(dict(event="verified-c17-bitflag-contracts",compiler=CC,**counts,sourceFilesUnchanged=True,temporaryBuildCleanup=True,log=str(LOG))))

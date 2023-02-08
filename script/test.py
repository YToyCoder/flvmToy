#!E:/python-/python
import os
import sys
import re
import share
# dir,file = os.path.split(os.path.abspath(__file__))
root = share.root #os.path.dirname(dir)
testFolder = share.testFolder #os.path.join(root, "test")
test_build = os.path.join(share.build, "test")

def log(msg):
  print(f"[flvm-test]: {msg}")

cc = "g++"

testFiles = [] 

def list_all_test_files():
  global testFiles
  testFiles = os.listdir(testFolder)

# files that built by cmd
builtExe = [] 

def build_and_run_test_files(file):
  source = os.path.join(testFolder, file)
  log(f"build file {source}")
  if not os.path.isfile(source):
    return
  target = os.path.join(test_build, file).replace(".cc",".exe")
  builtExe.append(target)
  cmd = f"{cc} {source} -o {target}"
  log(f"execute cmd : {cmd}")
  os.system(cmd)
  # run
  log(f"running test file : {target}")
  log(f"begin ${target}:>>> ")
  os.system(target)
  log(f"end   ${target}:<<< ")

def mksurePath():
  if not os.path.exists(test_build):
    os.makedirs(test_build)

def run_cmd():
  if len(sys.argv) > 1:
    mstr = sys.argv[1]
    if mstr == "clean":
      os.system(f"rm -r {root}/build")
      exit(0)
    else :
      temp = []
      global testFiles
      for f in testFiles:
        if re.match(f".*{mstr}.*", f) != None:
          temp.append(f)
      testFiles = temp

def build_cc_files():
  for el in testFiles:
    if el.endswith(".cc"):
      build_and_run_test_files(el)

def run():
  # init log
  log(f"root path {root}")
  log(f"test path {testFolder}")
  log(f"build path {test_build}")
  log(f"compiler cmd is {cc}")

  # make sure all the test path created
  mksurePath()

  # list test files
  list_all_test_files()

  # execute option -> like, clean
  run_cmd()

  # build and run all the test files
  build_cc_files()

if __name__ == "__main__":
  # run
  run()

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

log(f"root path {root}")
log(f"test path {testFolder}")
log(f"build path {test_build}")

testFiles = os.listdir(testFolder)

builtExe = []
def build_file(file):
  source = os.path.join(testFolder, file)
  log(f"build file {source}")
  if not os.path.isfile(source):
    return
  target = os.path.join(test_build, file).replace(".cc",".exe")
  builtExe.append(target)
  cmd = f"{cc} {source} -o {target}"
  log(f"execute cmd : {cmd}")
  os.system(cmd)

def mksurePath():
  if not os.path.exists(test_build):
    os.makedirs(test_build)

mksurePath()

if len(sys.argv) > 1:
  # testFiles = 
  mstr = sys.argv[1]
  if mstr == "clean":
    os.system(f"rm -r {root}/build")
    exit(0)
  else :
    temp = []
    for f in testFiles:
      if re.match(f".*{mstr}.*", f) != None:
        temp.append(f)
    testFiles = temp


for el in testFiles:
  if el.endswith(".cc"):
    build_file(el)

for t in builtExe:
  log(f"testing {t} ...")
  os.system(t)

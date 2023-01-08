#!E:/python-/python
import os
import sys
import re

dir,file = os.path.split(os.path.abspath(__file__))
root = os.path.dirname(dir)
testFolder = os.path.join(root, "test")
build = os.path.join(root, "build/test")

def log(msg):
  print(f"[flvm-test]: {msg}")

cc = "g++"

log(f"root path {root}")
log(f"test path {testFolder}")
log(f"build path {build}")

testFiles = os.listdir(testFolder)

builtExe = []
def build_file(file):
  log(f"build file {file}")
  source = os.path.join(testFolder, file)
  target = os.path.join(build, file).replace(".cc",".exe")
  builtExe.append(target)
  cmd = f"{cc} {source} -o {target}"
  log(f"execute cmd : {cmd}")
  os.system(cmd)

def mksurePath():
  if not os.path.exists(build):
    os.makedirs(build)

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

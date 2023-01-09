import os

dir,file = os.path.split(os.path.abspath(__file__))
root = os.path.dirname(dir)
testFolder = os.path.join(root, "test")
build = os.path.join(root, "build")

def log(msg):
  print(f"[flvm-test]: {msg}")

def folder_ensure(folder:str):
  if not os.path.exists(folder):
    os.makedirs(folder)


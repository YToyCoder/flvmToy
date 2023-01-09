#!E:/python-/python
import os
import share

bin = os.path.join(share.testFolder, "bin")

share.folder_ensure(bin)
order="little"

TagInt = 1
TagStr = 2

def write(filename: str):
  with open(filename,'wb') as file:
    a = 0xf1
    k = a.to_bytes(1, 'little')
    file.write(k)
    file.write((2).to_bytes(2,'little'))
    write_int(3, file)
    write_int(4, file)

def write_int(val: int, file):
  file.write(TagInt.to_bytes(1, order))
  file.write((val).to_bytes(8, order))

# space: each file is a space , you can define it , or default as it file name
# binary
# magic number | const pool | space | method | codes
'''
magic number : u1 f1

>>>> const pool
size of const pool: u2

uint8: 
  u1 tag 
  u2 len 
  u? content

int : 
u1 tag 
u8 bytes

double: 
u1 tag 
u8 bytes

string:
u1 tag 
u2 index 

method_info: 
u1 tag 
u2 space_name(string_index) 
u2 method_index(name&type)

name&type:
u1 tag
u2 string_index
u2 type_index(string_index)

>>>> space
space_info:
u2 name_index(string_index)

>>>> globals / static
size of globals: u2
u2 * ? index (name&type)

>>>> exports
size of globals: u2
u2 * ? index(name&type/method_info)

>>>> methods
size of methods: u2
method:
u2: const_index(method_info)
u2: code_index

>>>> codes
len u2
u1 * len

'''

class ConstWithName:
  def __init__(self, name, value, tag) -> None:
    self.name = name
    self.value = value 
    self.tag = tag 
  def createInt(name, value):
    return ConstWithName(name,value, TagInt)
  def createStr(name, value):
    return ConstWithName(name, value,)

def full_name(filename: str):
  return os.path.join(bin, filename)

class FileMethod:
  def __init__(self, name, t) -> None:
    self.codes = []
    self.name = name
    self.t = t

class FileWriter:
  def __init__(self) -> None:
    self.consts = []
    self.methods = []
  
  def write(self):
    pass

# filename = full_name("hello")
# write(filename)

# if os.path.exists(filename):
#   print("#" * 10)
#   os.system(f"hexdump {filename}")
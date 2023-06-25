## flvm - f-language vm

a virtual machine

### 1. instruction 字节码
flvm is a stack based vm, it support instruction as follow:
```
    nop      = 0x00, // do nothing
    iconst_0 = 0x01,
    iconst_1 = 0x02,
    iconst_2 = 0x03,
    iconst_3 = 0x04,
    iconst_4 = 0x05,
    dconst_1 = 0x06,
    dconst_2 = 0x07,
    ipush    = 0x08,
    dpush    = 0x09,
    iload    = 0x0A,
    dload    = 0x0B,
    aload    = 0x0C,
    istore   = 0x0D,
    dstore   = 0x0E,
    bstore   = 0x0F,
    ostore   = 0x11, // store string in local value
    sstore   = 0x11, // store string in local value
    ldci     = 0x15, // load int from consts
    ldcd     = 0x16, // load double from consts
    ldcs     = 0x17, // load string from consts

    // operator
    iadd     = 0x21,
    dadd     = 0x22,
    isub     = 0x23,
    dsub     = 0x24,
    imul     = 0x25,
    dmul     = 0x26,
    idiv     = 0x27,
    ddiv     = 0x28,

    // int comp
    ifeq     = 0x29,
    iflt     = 0x2A,
    ifle     = 0x2B,
    ifgt     = 0x2C,
    ifge     = 0x2D,
    go       = 0x2E,
    dcmp     = 0x2F,

    // cast
    i2d      = 0x30, // int to double
    i2b      = 0x31, // int to boolean
    d2i      = 0x32, // double to int
```

the runtime instruction code example:

![runtime-example](https://github.com/YToyCoder/flvmToy/blob/main/doc/runtime_instruction.png)

### 2. sytax 语法支持

#### 2.1 语法

1. 字面量: 数字(number)
2. 基础的数学计算: + - * /
3. 变量声明: let id = init
4. 变量引用: let a = b + c

#### 2.2 example 示例

```
let id = 1.2 + 3 * 2.1 - 4 + 2.1 
let a = 0 + id * ( 2 * 7 - 4.4 )
let b = 1 - 2 > 3 - 9
let c = 1.24 < 2.34
if 3 < 2
{
  let m = 1
}
else if 3 == 3
{
  let f = 2
  let ss = 1.2 + a
}

let e = "hello, string-world"
```
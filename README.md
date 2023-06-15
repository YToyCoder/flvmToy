## flvm - f-language vm

a virtual machine

### 1. instruction 字节码
flvm is a stack based vm, it support instruction as follow:
```
  iconst_0 = 0x00,
  iconst_1 = 0x01,
  iconst_2 = 0x02,
  iconst_3 = 0x03,
  iconst_4 = 0x04,
  dconst_1 = 0x05,
  dconst_2 = 0x06,
  ipush    = 0x07,
  dpush    = 0x08,
  iload    = 0x09,
  dload    = 0x0A,
  aload    = 0x0B,
  istore   = 0x0C,
  dstore   = 0x0D,
  ldci     = 0x11, // load int from consts
  ldcd     = 0x12, // load double from consts

  // operator
  iadd     = 0x21,
  dadd     = 0x22,
  isub     = 0x23,
  dsub     = 0x24,
  imul     = 0x25,
  dmul     = 0x26,
  idiv     = 0x27,
  ddiv     = 0x28,

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
let a = 2.2 * 3 - (5  * 10.0 / 2)
let b = a - 8 * 0.8
```
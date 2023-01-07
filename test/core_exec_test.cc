#define FlvmDebug
#include "../flvm.cc"

instr_t* int2code(int16_t vl_){
  instr_t * ret = (instr_t *) malloc(sizeof(instr_t) * 2);
  ret[0] = vl_ >> 8;
  ret[1] = vl_ & 0x00FF;
  return ret;
}

void push(int16_t val, FlMethodBuilder *builder){
  instr_t *v = int2code(val);
  builder->append(v[0]);
  builder->append(v[1]);
}

int main(int argc, char const *argv[])
{
  FlMethodBuilder mbuilder;
  mbuilder.append(Instruction::iconst_1);
  mbuilder.append(Instruction::iconst_2);
  mbuilder.append(Instruction::iconst_3);
  mbuilder.append(Instruction::iconst_4);
  mbuilder.append(Instruction::dconst_1);
  mbuilder.append(Instruction::dconst_2);
  mbuilder.append(Instruction::dload);
  mbuilder.append(1);
  mbuilder.append(Instruction::ipush);
  push(10, &mbuilder);
  mbuilder.append(Instruction::iload);
  mbuilder.append(0);
  mbuilder.set_max_stk(7);
  mbuilder.set_max_locals(2);
  FlFrame frame(nullptr, mbuilder.build());
  FlExec exec;
  exec.setBase(&frame);
  exec.run();
  return 0;
}

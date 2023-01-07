#define FlvmDebug
#include "../flvm.cc"

int main(int argc, char const *argv[])
{
  FlMethodBuilder mbuilder;
  mbuilder.append(Instruction::iconst_1);
  mbuilder.append(Instruction::iconst_2);
  mbuilder.append(Instruction::iconst_3);
  mbuilder.append(Instruction::iconst_4);
  mbuilder.set_max_stk(6);
  mbuilder.set_max_locals(2);
  FlFrame frame(nullptr, mbuilder.build());
  FlExec exec;
  exec.setBase(&frame);
  exec.run();
  return 0;
}

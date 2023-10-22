#include "emulator.h"

void Emulator::dispatch_instruction(instr_t instr) {
  switch(instr) {
    case Instruction::nop:                  break;
    case Instruction::iconst_0: iconst_0(); break;
    case Instruction::iconst_1: iconst_1(); break;
    case Instruction::iconst_2: iconst_2(); break;
    case Instruction::iconst_3: iconst_3(); break;
    case Instruction::iconst_4: iconst_4(); break;
    case Instruction::dconst_1: dconst_1(); break;
    case Instruction::dconst_2: dconst_2(); break;
    case Instruction::ipush:    pushi();    break;
    case Instruction::dpush:    pushd();    break;
    case Instruction::iload:    loadi();    break;
    case Instruction::dload:    loadd();    break;
    case Instruction::istore:   storei();   break;
    case Instruction::dstore:   stored();   break;
    case Instruction::bstore:   storeb();   break;
    case Instruction::sstore:   stores();   break;
    case Instruction::iadd:     addi();     break;
    case Instruction::dadd:     addd();     break;
    case Instruction::dsub:     subd();     break;
    case Instruction::isub:     subi();     break;
    case Instruction::imul:     muli();     break;
    case Instruction::dmul:     muld();     break;
    case Instruction::idiv:     divi();     break;
    case Instruction::ddiv:     divd();     break;
    case Instruction::ldci:     ldci();     break;
    case Instruction::ldcd:     ldcd();     break;
    case Instruction::ldcs:     ldcs();     break;
    case Instruction::i2d:      i2d();      break;
    case Instruction::i2b:      i2b();      break;
    case Instruction::d2i:      d2i();      break;
// ******************** int cmp **********************
#define CaseIntergalCmp(instruction, op) \
  case Instruction:: instruction : { \
    FlInt top_int = popi();              \
    uint8_t jmp_offset = read_instr();   \
    if(top_int op 0) set_pc(jmp_offset); \
  }                                      \
  break
// ***************************************************

    CaseIntergalCmp(ifeq, ==);
    CaseIntergalCmp(iflt, <);
    CaseIntergalCmp(ifle, <=);
    CaseIntergalCmp(ifgt, >);
    CaseIntergalCmp(ifge, >=);
    case Instruction::go: set_pc(read_instr()); break;
    case Instruction::dcmp: {
      FlDouble d1 = popd();
      FlDouble d2 = popd();
      FlDouble sub = d2 - d1;
      if(sub == 0) topFrame->pushi(0);
      else if(sub < 0) topFrame->pushi(-1);
      else topFrame->pushi(1);
    }
    break;

    default:
      printf("not support instruction : %s \n", instr_name((Instruction::Code)instr));
      throw_exception(("not support instruction : " + std::to_string(instr)).c_str());
  };
}
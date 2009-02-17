#include <libasm.h>
#include <libasm-arm.h>

char *asm_arm_get_register(int reg)
{
  switch (reg)
    {
    case ASM_ARM_REG_R0: return "r0";
    case ASM_ARM_REG_R1: return "r1";
    case ASM_ARM_REG_R2: return "r2";
    case ASM_ARM_REG_R3: return "r3";
    case ASM_ARM_REG_R4: return "r4";
    case ASM_ARM_REG_R5: return "r5";
    case ASM_ARM_REG_R6: return "r6";
    case ASM_ARM_REG_R7: return "r7";
    case ASM_ARM_REG_R8: return "r8";
    case ASM_ARM_REG_R9: return "r9";
    case ASM_ARM_REG_R10: return "r10";
    case ASM_ARM_REG_R11: return "r11";
    case ASM_ARM_REG_R12: return "r12";
    case ASM_ARM_REG_R13: return "sp";
    case ASM_ARM_REG_R14: return "lr";
    case ASM_ARM_REG_PC: return "pc";
    case ASM_ARM_REG_CPSR: return "cpsr";
    case ASM_ARM_REG_SPSR: return "spsr";
    default: return "bad";
    }
}

char *asm_arm_get_shift_type(u_int shift_type)
{
  switch (shift_type)
    {
    case ASM_ARM_SHIFT_LSL: return "lsl";
    case ASM_ARM_SHIFT_LSR: return "lsr";
    case ASM_ARM_SHIFT_ASR: return "asr";
    case ASM_ARM_SHIFT_ROR: return "ror";
    case ASM_ARM_SHIFT_RRX: return "rrx";
    default: return "bad";
    }
}

/**
 * Dump a ARM operand to a buffer.
 * @param ins Pointer to instruction structure.
 * @param num Number of the operand to dump
 * @param addr Address of the instruction
 * @param buf Buffer to store operand ascii representation.
 */

void	asm_arm_dump_operand(asm_instr *ins, int num, 
			       eresi_Addr addr, char *buf)
{
  asm_operand *op;

  if (num > ins->nb_op)
    return;

  op = &ins->op[num-1];
  
  switch (op->content) 
    {
    case ASM_ARM_OTYPE_REGISTER:
      sprintf(buf, "%s", asm_arm_get_register(op->baser));
      break;
    case ASM_ARM_OTYPE_IMMEDIATE:
      sprintf(buf, "#%i", op->imm);
      break;
    case ASM_ARM_OTYPE_REG_SCALED:
      sprintf(buf, "%s, %s",
              asm_arm_get_register(op->baser),
              asm_arm_get_shift_type(op->shift_type));
      if (op->indexr != ASM_ARM_REG_NUM)
        /* Scaled by register */
        sprintf(buf + strlen(buf), " %s",
                asm_arm_get_register(op->indexr));
      else if (op->shift_type !=  ASM_ARM_SHIFT_RRX)
        /* Scaled by immediate */
        sprintf(buf + strlen(buf), " #%i", op->imm);
      break;
    case ASM_ARM_OTYPE_REG_OFFSET:
      sprintf(buf, "[%s", asm_arm_get_register(op->baser));

      if (op->addressing_type == ASM_ARM_ADDRESSING_POST)
        sprintf(buf + strlen(buf), "]");

      if (op->indexr == ASM_ARM_REG_NUM)
        /* Immediate offset */
        sprintf(buf + strlen(buf), ", #%c%i",
                (op->offset_added ? '+' : '-'),
                op->imm);
      else
        {
          sprintf(buf + strlen(buf), ", %c%s",
                  (op->offset_added ? '+' : '-'),
                  asm_arm_get_register(op->indexr));
          if (op->shift_type != ASM_ARM_SHIFT_NUM)
            /* Scaled register offset */
            {
              sprintf(buf + strlen(buf), ", %s",
                      asm_arm_get_shift_type(op->shift_type));
              if (op->shift_type != ASM_ARM_SHIFT_RRX)
                sprintf(buf + strlen(buf), " #%i",
                        op->imm);
            }
        }

      if (op->addressing_type == ASM_ARM_ADDRESSING_OFFSET)
        sprintf(buf + strlen(buf), "]");
      else if (op->addressing_type == ASM_ARM_ADDRESSING_PRE)
        sprintf(buf + strlen(buf), "]!");

      break;
    default:
      sprintf(buf, "err");
      break;
  }
}

char		*asm_arm_display_instr(asm_instr *instr, eresi_Addr addr)
{
  u_int32_t	i;
  static char 	buffer[1024];

  memset(buffer,0,sizeof(buffer));
  sprintf(buffer,"%s",instr->name);

  strcat(buffer, " ");
  for (i = 0; i < instr->nb_op; i++)
    {
      asm_arm_dump_operand(instr, i+1, addr, buffer + strlen(buffer));
      if (i < instr->nb_op -1)
        strcat(buffer, ", ");
    }

  return buffer;
}

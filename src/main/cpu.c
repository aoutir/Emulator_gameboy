/**
 * @file cpu.c
 * @brief Game Boy CPU simulation
 *
 * @date 2019
 */

#include "error.h"
#include "opcode.h"
#include "cpu.h"
#include "cpu-alu.h"
#include "cpu-registers.h"
#include "cpu-storage.h"
#include "util.h"

#include <inttypes.h> // PRIX8
#include <stdio.h>    // fprintf

/**
 * see cpu.h
 * */
int cpu_init(cpu_t *cpu)
{
    M_REQUIRE_NON_NULL(cpu);
    M_EXIT_IF_ERR(component_create(&cpu->high_ram, HIGH_RAM_SIZE));
    cpu->alu.flags = 0;
    cpu->alu.value = 0;
    cpu->AF = 0;
    cpu->BC = 0;
    cpu->DE = 0;
    cpu->HL = 0;
    cpu->PC = 0;
    cpu->SP = 0;
    cpu->idle_time = 0;
    cpu->IME = 0;
    cpu->IE = 0;
    cpu->IF = 0;
    cpu->HALT = 0;
    cpu->write_listener = 0;
    return ERR_NONE;
}
/**
 * @brief verification de la condition
 *
 * @param cpu cpu pointer
 * @param lu instruction pointer
 * @return 1 si la codition est verifie sinon 0
 */
int verify_cc(cpu_t *cpu, const instruction_t *lu)
{
    switch (extract_cc(lu->opcode))
    {
    case 0:
        return get_Z(cpu->F) == 0 ? 1 : 0;
        break;
    case 1:
        return get_Z(cpu->F) == FLAG_Z ? 1 : 0;
        break;
    case 2:
        return get_C(cpu->F) == 0 ? 1 : 0;
        break;
    case 3:
        return get_C(cpu->F) == FLAG_C ? 1 : 0;
        break;
    default:
        return 0;
    }
}

/**
 * determiner le numero d'interrruption a gerer dans le cas ou il y a aucune interruption 
 * retourne JOYPAD + 1
 */
int nb_interrupt(cpu_t *cpu)
{
    for (int i = 0; i <= JOYPAD; i++)
    {
        if (bit_get(cpu->IE, i) == bit_get(cpu->IF, i) && bit_get(cpu->IE, i) == 1)
        {
            return i;
        }
    }
    return JOYPAD + 1;
}
/**
 * see cpu.h
 * */
int cpu_plug(cpu_t *cpu, bus_t *bus)
{
    M_REQUIRE_NON_NULL(cpu);
    M_REQUIRE_NON_NULL(bus);
    cpu->bus = bus;
    M_EXIT_IF_ERR(bus_plug(*(cpu->bus), &cpu->high_ram, HIGH_RAM_START, HIGH_RAM_END));
    (*cpu->bus)[REG_IE] = &cpu->IE;
    (*cpu->bus)[REG_IF] = &cpu->IF;

    return ERR_NONE;
}

/**
 * see cpu.h
 * */
void cpu_free(cpu_t *cpu)
{
    if (cpu != NULL)
    {   if(cpu->bus != NULL){
        (*cpu->bus)[REG_IE] = NULL;
        (*cpu->bus)[REG_IF] = NULL;
        bus_unplug(*(cpu->bus), &cpu->high_ram);
         }
        cpu->bus = NULL;
        component_free(&cpu->high_ram);
        
    }
}

/**
 * @brief Executes an instruction
 * @param lu instruction
 * @param cpu, the CPU which shall execute
 * @return error code
 *
 * See opcode.h and cpu.h
 */
static int cpu_dispatch(const instruction_t *lu, cpu_t *cpu)
{
    M_REQUIRE_NON_NULL(lu);
    M_REQUIRE_NON_NULL(cpu);
    cpu->alu.value = 0;
    cpu->alu.flags = 0;

    switch (lu->family)
    {

    // ALU
    case ADD_A_HLR:
    case ADD_A_N8:
    case ADD_A_R8:
    case INC_HLR:
    case INC_R8:
    case ADD_HL_R16SP:
    case INC_R16SP:
    case SUB_A_HLR:
    case SUB_A_N8:
    case SUB_A_R8:
    case DEC_HLR:
    case DEC_R8:
    case DEC_R16SP:
    case AND_A_HLR:
    case AND_A_N8:
    case AND_A_R8:
    case OR_A_HLR:
    case OR_A_N8:
    case OR_A_R8:
    case XOR_A_HLR:
    case XOR_A_N8:
    case XOR_A_R8:
    case CPL:
    case CP_A_HLR:
    case CP_A_N8:
    case CP_A_R8:
    case SLA_HLR:
    case SLA_R8:
    case SRA_HLR:
    case SRA_R8:
    case SRL_HLR:
    case SRL_R8:
    case ROTCA:
    case ROTA:
    case ROTC_HLR:
    case ROT_HLR:
    case ROTC_R8:
    case ROT_R8:
    case SWAP_HLR:
    case SWAP_R8:
    case BIT_U3_HLR:
    case BIT_U3_R8:
    case CHG_U3_HLR:
    case CHG_U3_R8:
    case LD_HLSP_S8:
    case DAA:
    case SCCF:
        M_EXIT_IF_ERR(cpu_dispatch_alu(lu, cpu));

        break;

    // STORAGE
    case LD_A_BCR:
    case LD_A_CR:
    case LD_A_DER:
    case LD_A_HLRU:
    case LD_A_N16R:
    case LD_A_N8R:
    case LD_BCR_A:
    case LD_CR_A:
    case LD_DER_A:
    case LD_HLRU_A:
    case LD_HLR_N8:
    case LD_HLR_R8:
    case LD_N16R_A:
    case LD_N16R_SP:
    case LD_N8R_A:
    case LD_R16SP_N16:
    case LD_R8_HLR:
    case LD_R8_N8:
    case LD_R8_R8:
    case LD_SP_HL:
    case POP_R16:
    case PUSH_R16:
        M_EXIT_IF_ERR(cpu_dispatch_storage(lu, cpu));

        break;

    // JUMP
    case JP_CC_N16:
    {
        if (verify_cc(cpu, lu) == 1)
        {
            cpu->PC = cpu_read_addr_after_opcode(cpu) - lu->bytes;
            cpu->idle_time += lu->xtra_cycles;
        }
    }
    break;
    case JP_HL:
        cpu->PC = cpu_HL_get(cpu) - lu->bytes;
        break;

    case JP_N16:
        cpu->PC = cpu_read_addr_after_opcode(cpu) - lu->bytes;
        break;

    case JR_CC_E8:
    {
        if (verify_cc(cpu, lu) == 1)
        {
            cpu->PC += (int8_t)cpu_read_data_after_opcode(cpu);
            cpu->idle_time += lu->xtra_cycles;
        }
    }
    break;

    case JR_E8:
        cpu->PC += (int8_t)cpu_read_data_after_opcode(cpu);
        break;

    // CALLS
    case CALL_CC_N16:
    {
        if (verify_cc(cpu, lu))
        {
            M_EXIT_IF_ERR(cpu_SP_push(cpu, cpu->PC + lu->bytes));
            cpu->PC = cpu_read_addr_after_opcode(cpu) - lu->bytes;
            cpu->idle_time += lu->xtra_cycles;
        }
    }
    break;

    case CALL_N16:
    {
        M_EXIT_IF_ERR(cpu_SP_push(cpu, cpu->PC + lu->bytes));
        cpu->PC = cpu_read_addr_after_opcode(cpu) - lu->bytes;
    }
    break;

    // RETURN (from call)
    case RET:
        cpu->PC = cpu_SP_pop(cpu) - lu->bytes;
        break;

    case RET_CC:
    {
        if (verify_cc(cpu, lu))
        {
            cpu->PC = cpu_SP_pop(cpu) - lu->bytes;
            cpu->idle_time += lu->xtra_cycles;
        }
    }
    break;

    case RST_U3:
    {
        M_EXIT_IF_ERR(cpu_SP_push(cpu, cpu->PC + lu->bytes));
        cpu->PC = (extract_n3(lu->opcode) << 3) - lu->bytes;
    }
    break;

    // INTERRUPT & MISC.
    case EDI:
        cpu->IME = extract_ime(lu->opcode);
        break;

    case RETI:
    {
        cpu->IME = 1;
        cpu->PC = cpu_SP_pop(cpu) - lu->bytes;
    }
    break;

    case HALT:
        cpu->HALT = 1;
        break;

    case STOP:
    case NOP:
        // ne rien faire
        break;

    default:
    {
        fprintf(stderr, "Unknown instruction, Code: 0x%" PRIX8 "\n", cpu_read_at_idx(cpu, cpu->PC));
        return ERR_INSTR;
    }
    break;

    } // switch

    cpu->idle_time += lu->cycles - 1;
    cpu->PC += lu->bytes;

    return ERR_NONE;
}

static int cpu_do_cycle(cpu_t *cpu)
{
    M_REQUIRE_NON_NULL(cpu);
    int nbInterrupt = nb_interrupt(cpu); //numero de l'instruction a gerer
    if (cpu->IME == 1 && nbInterrupt <= JOYPAD)
    {
        cpu->IME = 0;
        bit_unset(&cpu->IF, nbInterrupt);
        cpu_SP_push(cpu, cpu->PC);
        cpu->PC = 0x40 + (nbInterrupt << 3);
        cpu->idle_time += 5;
    }else{
    data_t first_byte = cpu_read_at_idx(cpu, cpu->PC);
    if (first_byte == PREFIXED)
    {
        instruction_t prefixed_instruction = instruction_prefixed[cpu_read_data_after_opcode(cpu)];
        M_EXIT_IF_ERR(cpu_dispatch(&prefixed_instruction, cpu));
    }
    else
    {
        instruction_t instruction = instruction_direct[first_byte];
        M_EXIT_IF_ERR(cpu_dispatch(&instruction, cpu));
    }
    }
    return ERR_NONE;
}

/**
 * See cpu.h
 */
int cpu_cycle(cpu_t *cpu)
{
    M_REQUIRE_NON_NULL(cpu);
    M_REQUIRE_NON_NULL(cpu->bus);
    cpu->write_listener = 0;

    if (cpu->HALT == 1 && nb_interrupt(cpu) <= JOYPAD && cpu->idle_time == 0)
    { 
        cpu->HALT = 0;
        M_EXIT_IF_ERR(cpu_do_cycle(cpu));
        return ERR_NONE;
    }
    else if (cpu->idle_time != 0)
    {
        --cpu->idle_time;
        return ERR_NONE;

    }
    else if (cpu->idle_time == 0 && cpu->HALT == 0)
    {
        M_EXIT_IF_ERR(cpu_do_cycle(cpu));
    }

    return ERR_NONE;
}

/**
 * See cpu.h
 */
void cpu_request_interrupt(cpu_t *cpu, interrupt_t i)
{
    if (cpu != NULL)
    {
        bit_set(&cpu->IF, i);
    }
}

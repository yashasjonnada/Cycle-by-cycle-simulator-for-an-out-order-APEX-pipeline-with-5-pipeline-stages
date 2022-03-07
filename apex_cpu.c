/*
 * apex_cpu.c
 * Contains APEX cpu pipeline implementation
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "apex_cpu.h"
#include "apex_macros.h"

int scoreBoard[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int collection[16] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
bool stall = FALSE;

/* Converts the PC(4000 series) into array index for code memory
 *
 * Note: You are not supposed to edit this function
 */
static int
get_code_memory_index_from_pc(const int pc)
{
    return (pc - 4000) / 4;
}

static void
print_instruction(const CPU_Stage *stage)
{
    switch (stage->opcode)
    {
        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_MUL:
        case OPCODE_DIV:
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_XOR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }

        case OPCODE_ADDL:
        case OPCODE_SUBL:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1, stage->imm);
            break;
        }

        case OPCODE_NOP: 
        {
            printf("%s ", stage->opcode_str);
            break;
        }

        case OPCODE_MOVC:
        {
            printf("%s,R%d,#%d ", stage->opcode_str, stage->rd, stage->imm);
            break;
        }

        case OPCODE_LOAD:
        case OPCODE_LDI:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }

        case OPCODE_STORE:
        case OPCODE_STI:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2,
                   stage->imm);
            break;
        }

        case OPCODE_BZ:
        case OPCODE_BNZ:
        case OPCODE_BP:
        case OPCODE_BNP:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }

        case OPCODE_HALT:
        {
            printf("%s", stage->opcode_str);
            break;
        }

        case OPCODE_CMP:
        {
            printf("%s,R%d,R%d ", stage->opcode_str, stage->rs1, stage->rs2);
            break;
        }

        case OPCODE_JUMP:
        {
            printf("%s,R%d,R%d ", stage->opcode_str, stage->rs1, stage->imm);
            break;
        }
    }
}

/* Debug function which prints the CPU stage content
 *
 * Note: You can edit this function to print in more detail
 */
static void
print_stage_content(const char *name, const CPU_Stage *stage)
{
    printf("%-15s: pc(%d) ", name, stage->pc);
    print_instruction(stage);
    printf("\n");
}

/* Debug function which prints the register file
 *
 * Note: You are not supposed to edit this function
 */
static void
print_reg_file(const APEX_CPU *cpu)
{
    int i;

    printf("----------\n%s\n----------\n", "Registers:");

    for (int i = 0; i < REG_FILE_SIZE / 2; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");

    for (i = (REG_FILE_SIZE / 2); i < REG_FILE_SIZE; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");
}

int state_of_arch_reg_file(APEX_CPU* cpu) {
    int start = 0;
    printf("\n==================================== STATE OF ARCHITECTURAL REGISTER FILE ====================================\n");
    int total_number_of_registers = (int)(sizeof(cpu->regs)/4);

    while(start < total_number_of_registers){
        int rd = 0;
        printf("| \t REG[%d] \t | \t Value = %d \t | \t Status = %s \t \n", start, cpu->regs[start], (scoreBoard[cpu->writeback.rd]? "INVALID" : "VALID" ));
        start++;
        rd++;
        
    }
    return 0;
}

int state_of_data_memory(APEX_CPU* cpu) {
    int start = 0;
    printf("\n========================================== STATE OF DATA MEMORY ================================================\n");
  
    while(start < 100){
        printf("| \t MEM[%d] \t | \t Data Value = %d \t |\n", start, cpu->data_memory[start]);
        start++;
    }
  return 0;
}

/*
 * Fetch Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_fetch(APEX_CPU *cpu)
{
    APEX_Instruction *current_ins;

    if (cpu->fetch.has_insn)
    {
        /* This fetches new branch target instruction from next cycle */
        if (cpu->fetch_from_next_cycle == TRUE)
        {
            cpu->fetch_from_next_cycle = FALSE;

            /* Skip this cycle*/
            return;
        }

        /* Store current PC in fetch latch */
        cpu->fetch.pc = cpu->pc;
        /* Index into code memory using this pc and copy all instruction fields * into fetch latch  */
        current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
        strcpy(cpu->fetch.opcode_str, current_ins->opcode_str);
        cpu->fetch.opcode = current_ins->opcode;
        cpu->fetch.rd = current_ins->rd;
        cpu->fetch.rs1 = current_ins->rs1;
        cpu->fetch.rs2 = current_ins->rs2;
        cpu->fetch.imm = current_ins->imm;

        /* Update PC for next instruction */
        if(stall == FALSE){
            cpu->pc += 4;
            /* Copy data from fetch latch to decode latch*/
            cpu->decode = cpu->fetch;
        }else{
            current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
            strcpy(cpu->fetch.opcode_str, current_ins->opcode_str);
            cpu->fetch.opcode = current_ins->opcode;
            cpu->fetch.rd = current_ins->rd;
            cpu->fetch.rs1 = current_ins->rs1;
            cpu->fetch.rs2 = current_ins->rs2;
            cpu->fetch.imm = current_ins->imm;
        }
        
        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Fetch", &cpu->fetch);
        }

        /* Stop fetching new instructions if HALT is fetched */
        if (cpu->fetch.opcode == OPCODE_HALT && stall == FALSE)
        {
            cpu->fetch.has_insn = FALSE;
        }
    }
}

/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_decode(APEX_CPU *cpu)
{
    if (cpu->decode.has_insn)
    {
        /* Read operands from register file based on the instruction type */
        switch (cpu->decode.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_MUL:
            case OPCODE_DIV:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            {

                if(collection[cpu->decode.rs1] != -1 && collection[cpu->decode.rs2] != -1){
                    scoreBoard[cpu->decode.rd] = 1;
                    cpu->decode.rs1_value = collection[cpu->decode.rs1];
                    cpu->decode.rs2_value = collection[cpu->decode.rs2];
                    cpu->execute = cpu->decode;
                    cpu->decode.has_insn = FALSE;
                    stall = FALSE;

                }else if(scoreBoard[cpu->decode.rs1] == 0 && scoreBoard[cpu->decode.rs2] == 0){
                    scoreBoard[cpu->decode.rd] = 1;

                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];

                    cpu->execute = cpu->decode;
                    cpu->decode.has_insn = FALSE;
                    stall = FALSE;
                }else{
                    cpu->decode.has_insn = TRUE;
                    stall = TRUE;
                }
                
                break;
            }

            case OPCODE_ADDL:
            case OPCODE_SUBL:
            {
                if(collection[cpu->decode.rs1] != -1){
                    scoreBoard[cpu->decode.rd] = 1;
                    cpu->decode.rs1_value = collection[cpu->decode.rs1];
                    cpu->execute = cpu->decode;
                    cpu->decode.has_insn = FALSE;
                    stall = FALSE;

                }else if(scoreBoard[cpu->decode.rs1] == 0){
                    scoreBoard[cpu->decode.rd] = 1;
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->execute = cpu->decode;
                    cpu->decode.has_insn = FALSE;
                    stall = FALSE;
                }else{
                    cpu->decode.has_insn = TRUE;
                    stall = TRUE;
                }
                break;
            }

            case OPCODE_LOAD:
            {
                if(collection[cpu->decode.rs1] != -1){
                    scoreBoard[cpu->decode.rd] = 1;
                    cpu->decode.rs1_value = collection[cpu->decode.rs1];
                    cpu->execute = cpu->decode;
                    cpu->decode.has_insn = FALSE;
                    stall = FALSE;
                }else if(scoreBoard[cpu->decode.rs1] == 0){
                    scoreBoard[cpu->decode.rd] = 1;
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->execute = cpu->decode;
                    cpu->decode.has_insn = FALSE;
                    stall = FALSE;
                }else{
                    cpu->decode.has_insn = TRUE;
                    stall = TRUE;
                }
                break;
            }

            case OPCODE_LDI:
            {
                if(collection[cpu->decode.rs1] != -1){
                    scoreBoard[cpu->decode.rd] = 1;
                    scoreBoard[cpu->decode.rs1] = 1;
                    
                    cpu->decode.rs1_value = collection[cpu->decode.rs1];
                    cpu->execute = cpu->decode;
                    cpu->decode.has_insn = FALSE;
                    stall = FALSE;

                }else if(scoreBoard[cpu->decode.rs1] == 0){
                    scoreBoard[cpu->decode.rd] = 1;
                    scoreBoard[cpu->decode.rs1] = 1;
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->execute = cpu->decode;
                    cpu->decode.has_insn = FALSE;
                    stall = FALSE;
                }else{
                    cpu->decode.has_insn = TRUE;
                    stall = TRUE;
                }
                break;
            }

            case OPCODE_STI:
            {
                if(collection[cpu->decode.rs1] != -1){
                    cpu->decode.rs1_value = collection[cpu->decode.rs1];
                    cpu->decode.rs2_value = collection[cpu->decode.rs2];
                    cpu->execute = cpu->decode;
                    cpu->decode.has_insn = FALSE;
                    stall = FALSE;
                    
                }else if(scoreBoard[cpu->decode.rs2] == 0){
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                scoreBoard[cpu->decode.rs2] = 1;
                cpu->execute = cpu->decode;
                cpu->decode.has_insn = FALSE;
                stall = FALSE;
                }else{
                    cpu->decode.has_insn = TRUE;
                    stall = TRUE;
                }
                break;
            }

           /* case OPCODE_STORE:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                cpu->execute = cpu->decode;
                cpu->decode.has_insn = FALSE;
                stall = FALSE;
                break;
            }*/

            case OPCODE_STORE:
            {
                if(collection[cpu->decode.rs1] != -1){
                    cpu->decode.rs1_value = collection[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    cpu->execute = cpu->decode;
                    cpu->decode.has_insn = FALSE;
                    stall = FALSE;
                    
                }else if(scoreBoard[cpu->decode.rs2] == 0){
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    scoreBoard[cpu->decode.rs2] = 1;
                    cpu->execute = cpu->decode;
                    cpu->decode.has_insn = FALSE;
                    stall = FALSE;
                }else{
                    cpu->decode.has_insn = TRUE;
                    stall = TRUE;
                }
                break;
            }


            case OPCODE_MOVC:
            {
                scoreBoard[cpu->decode.rd] = 1;
                cpu->execute = cpu->decode;
                cpu->decode.has_insn = FALSE;
                stall = FALSE;
                /* MOVC doesn't have register operands */
                break;
            }

            case OPCODE_CMP:
            {
                if(collection[cpu->decode.rs1] != -1 && collection[cpu->decode.rs2] != -1){
                    cpu->decode.rs1_value = collection[cpu->decode.rs1];
                    cpu->decode.rs2_value = collection[cpu->decode.rs2];

                    cpu->execute = cpu->decode;
                    cpu->decode.has_insn = FALSE;
                    stall = FALSE;
                }else if(scoreBoard[cpu->decode.rs1] == 0 && scoreBoard[cpu->decode.rs2] == 0){

                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];

                    cpu->execute = cpu->decode;
                    cpu->decode.has_insn = FALSE;
                    stall = FALSE;
                }else{
                    cpu->decode.has_insn = TRUE;
                    stall = TRUE;
                }
                
                break;
            }

            case OPCODE_JUMP:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];

                cpu->execute = cpu->decode;
                cpu->decode.has_insn = FALSE;
                stall = FALSE;
                break;
            }

            case OPCODE_HALT:
            {
                cpu->execute = cpu->decode;
                cpu->decode.has_insn = FALSE;
                stall = FALSE;
                break;
            }

            case OPCODE_BP:
            case OPCODE_BNP:
            case OPCODE_BZ:
            case OPCODE_BNZ:
            {
                cpu->execute = cpu->decode;
                cpu->decode.has_insn = FALSE;
                stall = FALSE;
                break;
            }

            case OPCODE_NOP:
            {
                break;
            }
        }

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Decode/RF", &cpu->decode);
        }
    }
}

/*
 * Execute Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_execute(APEX_CPU *cpu)
{
    if (cpu->execute.has_insn)
    {
        /* Execute logic based on instruction type */
        switch (cpu->execute.opcode)
        {
            case OPCODE_ADD:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value + cpu->execute.rs2_value;
                
                collection[cpu->execute.rd] = cpu->execute.result_buffer;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }

                if (cpu->execute.result_buffer > 0)
                {
                    cpu->positive_flag = TRUE;
                }
                else{
                    cpu->positive_flag = FALSE;
                }
                break;
            }

            case OPCODE_ADDL:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value + cpu->execute.imm;
                
                collection[cpu->execute.rd] = cpu->execute.result_buffer;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }

                if (cpu->execute.result_buffer > 0)
                {
                    cpu->positive_flag = TRUE;
                }
                else{
                    cpu->positive_flag = FALSE;
                }
                break;
            }

              case OPCODE_SUBL:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value - cpu->execute.imm;

                collection[cpu->execute.rd] = cpu->execute.result_buffer;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }

                if (cpu->execute.result_buffer > 0)
                {
                    cpu->positive_flag = TRUE;
                }
                else{
                    cpu->positive_flag = FALSE;
                }
                break;
            }

            case OPCODE_SUB:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value - cpu->execute.rs2_value;
                
                collection[cpu->execute.rd] = cpu->execute.result_buffer;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }

                if (cpu->execute.result_buffer > 0)
                {
                    cpu->positive_flag = TRUE;
                }
                else{
                    cpu->positive_flag = FALSE;
                }
                break;
            }

            case OPCODE_MUL:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value * cpu->execute.rs2_value;

                collection[cpu->execute.rd] = cpu->execute.result_buffer;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }

                if (cpu->execute.result_buffer > 0)
                {
                    cpu->positive_flag = TRUE;
                }
                else{
                    cpu->positive_flag = FALSE;
                }
                break;
            }

            case OPCODE_DIV:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value / cpu->execute.rs2_value;

                collection[cpu->execute.rd] = cpu->execute.result_buffer;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }

                if (cpu->execute.result_buffer > 0)
                {
                    cpu->positive_flag = TRUE;
                }
                else{
                    cpu->positive_flag = FALSE;
                }
                break;
            }

             case OPCODE_AND:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value & cpu->execute.rs2_value;
                collection[cpu->execute.rd] = cpu->execute.result_buffer;
                    break;
            }

             case OPCODE_OR:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value | cpu->execute.rs2_value;
                collection[cpu->execute.rd] = cpu->execute.result_buffer;
                    break;
            }

             case OPCODE_XOR:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value ^ cpu->execute.rs2_value;
                collection[cpu->execute.rd] = cpu->execute.result_buffer;
                    break;
            }
            
            case OPCODE_LOAD:
            {
                cpu->execute.memory_address
                    = cpu->execute.rs1_value + cpu->execute.imm;
                break;
            }

            case OPCODE_STORE:           
            {
                cpu->execute.memory_address
                    = cpu->execute.rs2_value + cpu->execute.imm;
                
                //collection[cpu->execute.rd] = cpu->execute.new_result_buffer;
                break;
            }

            case OPCODE_JUMP:   
            {
                cpu->pc = cpu->execute.rs1_value + cpu->execute.imm;
                /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                break;
            }

            case OPCODE_LDI:
            {
                cpu->execute.memory_address
                    = cpu->execute.rs1_value + cpu->execute.imm;
                
                cpu->execute.new_result_buffer
                    = cpu->execute.rs1_value + 4;
                
                collection[cpu->execute.rd] = cpu->execute.new_result_buffer;
                break;
            }

            case OPCODE_STI:           
            {
                cpu->execute.memory_address
                    = cpu->execute.rs2_value + cpu->execute.imm;

                cpu->execute.new_result_buffer
                    = cpu->execute.rs2_value + 4;
                
                collection[cpu->execute.rs2] = cpu->execute.new_result_buffer;
                break;
            }

            case OPCODE_CMP:
            {
                if(cpu->execute.rs1_value == cpu->execute.rs2_value){
                    cpu->zero_flag = TRUE;
                    cpu->positive_flag = FALSE;
                }
                
                if(cpu->execute.rs1_value > cpu->execute.rs2_value){
                    cpu->positive_flag = TRUE;
                    cpu->zero_flag = FALSE;
                }
                break;
            }

            case OPCODE_NOP:
            {
                break;
            }

            case OPCODE_BZ:
            {
                if (cpu->zero_flag == TRUE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_BP:
            {
                if (cpu->positive_flag == TRUE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_BNP:
            {
                if (cpu->positive_flag == FALSE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_BNZ:
            {
                if (cpu->zero_flag == FALSE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_MOVC: 
            {
                cpu->execute.result_buffer = cpu->execute.imm;
                collection[cpu->execute.rd] = cpu->execute.result_buffer;
                break;
            }

            case OPCODE_HALT:
            {
                break;
            }
        }

        /* Copy data from execute latch to memory latch*/
        cpu->memory = cpu->execute;
        cpu->execute.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Execute", &cpu->execute);
        }
    }
}

/*
 * Memory Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_memory(APEX_CPU *cpu)
{
    if (cpu->memory.has_insn)
    {
        switch (cpu->memory.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_DIV:
            case OPCODE_MUL:
            case OPCODE_SUB:
            case OPCODE_NOP:
            case OPCODE_CMP:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            case OPCODE_SUBL:
            case OPCODE_ADDL:
            {
                /* No work */
                break;
            }

            case OPCODE_LOAD:
            {
                /* Read from data memory */
                cpu->memory.result_buffer
                    = cpu->data_memory[cpu->memory.memory_address];
                break;
            }

            case OPCODE_LDI:
            {
                cpu->memory.result_buffer   
                    = cpu->data_memory[cpu->memory.memory_address];
                break;
            }

            case OPCODE_STORE: 
            {
                /* Read from data memory */
                cpu->data_memory[cpu->memory.memory_address]
                    = cpu->memory.rs1_value;
                break;
            }

            case OPCODE_STI: 
            {
                /* Read from data memory */
                cpu->data_memory[cpu->memory.memory_address]
                    = cpu->memory.rs1_value;
                
                break;
            }

            case OPCODE_HALT:
            case OPCODE_JUMP:
            {
                break;
            }
        }

        /* Copy data from memory latch to writeback latch*/
        cpu->writeback = cpu->memory;
        cpu->memory.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Memory", &cpu->memory);
        }
    }
}

/*
 * Writeback Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static int
APEX_writeback(APEX_CPU *cpu)
{
    if (cpu->writeback.has_insn)
    {
        /* Write result to register file based on instruction type */
        switch (cpu->writeback.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_MUL:
            case OPCODE_DIV:
            case OPCODE_SUB:
            case OPCODE_ADDL:
            case OPCODE_SUBL:
            case OPCODE_LOAD:
            case OPCODE_MOVC:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                scoreBoard[cpu->writeback.rd] = 0;
                //collection[cpu->writeback.rd] = -1;
                break;
            }

            case OPCODE_LDI:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                cpu->regs[cpu->writeback.rs1] = cpu->writeback.new_result_buffer;
                scoreBoard[cpu->writeback.rd] = 0;
                scoreBoard[cpu->writeback.rs1] = 0;
                break;
            }

            case OPCODE_STORE:
            case OPCODE_NOP:
            case OPCODE_BP:
            case OPCODE_BNP:
            case OPCODE_BZ:
            case OPCODE_BNZ:
            case OPCODE_JUMP:
            {
                //no work to be done
                break;
            }

            case OPCODE_STI:
            {
                cpu->regs[cpu->writeback.rs2] = cpu->writeback.new_result_buffer;
                scoreBoard[cpu->writeback.rs2] = 0;
                break;
            }

        }

        cpu->insn_completed++;
        cpu->writeback.has_insn = FALSE;
        
        
        if(stall == TRUE)
        {
            stall = FALSE;
        }

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Writeback", &cpu->writeback);
        }

        if (cpu->writeback.opcode == OPCODE_HALT)
        {
            /* Stop the APEX simulator */
            state_of_arch_reg_file(cpu);
            state_of_data_memory(cpu);
            return TRUE;
        }
    }

    /* Default */
    return 0;
}

/*
 * This function creates and initializes APEX cpu.
 *
 * Note: You are free to edit this function according to your implementation
 */
APEX_CPU *
APEX_cpu_init(const char *filename)
{
    int i;
    APEX_CPU *cpu;

    if (!filename)
    {
        return NULL;
    }

    cpu = calloc(1, sizeof(APEX_CPU));

    if (!cpu)
    {
        return NULL;
    }

    /* Initialize PC, Registers and all pipeline stages */
    cpu->pc = 4000;
    memset(cpu->regs, 0, sizeof(int) * REG_FILE_SIZE);
    memset(cpu->data_memory, 0, sizeof(int) * DATA_MEMORY_SIZE);
    cpu->single_step = ENABLE_SINGLE_STEP;

    /* Parse input file and create code memory */
    cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);
    if (!cpu->code_memory)
    {
        free(cpu);
        return NULL;
    }

    if (ENABLE_DEBUG_MESSAGES)
    {
        fprintf(stderr,
                "APEX_CPU: Initialized APEX CPU, loaded %d instructions\n",
                cpu->code_memory_size);
        fprintf(stderr, "APEX_CPU: PC initialized to %d\n", cpu->pc);
        fprintf(stderr, "APEX_CPU: Printing Code Memory\n");
        printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode_str", "rd", "rs1", "rs2",
               "imm");

        for (i = 0; i < cpu->code_memory_size; ++i)
        {
            printf("%-9s %-9d %-9d %-9d %-9d\n", cpu->code_memory[i].opcode_str,
                   cpu->code_memory[i].rd, cpu->code_memory[i].rs1,
                   cpu->code_memory[i].rs2, cpu->code_memory[i].imm);
        }
    }

    /* To start fetch stage */
    cpu->fetch.has_insn = TRUE;
    return cpu;
}

/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_run(APEX_CPU *cpu)
{
    char user_prompt_val;

    while (TRUE)
    {
        if (ENABLE_DEBUG_MESSAGES)
        {
            printf("--------------------------------------------\n");
            printf("Clock Cycle #: %d\n", cpu->clock);
            printf("--------------------------------------------\n");
        }

        if (APEX_writeback(cpu))
        {
            /* Halt in writeback stage */
            printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
            break;
        }

        APEX_memory(cpu);
        APEX_execute(cpu);
        APEX_decode(cpu);
        APEX_fetch(cpu);

        print_reg_file(cpu);

        if (cpu->single_step)
        {
            printf("Press any key to advance CPU Clock or <q> to quit:\n");
            scanf("%c", &user_prompt_val);

            if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
            {
                printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                break;
            }
        }

        cpu->clock++;
    }
}

/*
 * This function deallocates APEX CPU.
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_stop(APEX_CPU *cpu)
{
    free(cpu->code_memory);
    free(cpu);
}
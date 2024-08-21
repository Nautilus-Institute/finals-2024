//////////////////////////////////////////////////////////////////
//                                                              //
//  Decode stage of Amber 2 Core                                //
//                                                              //
//  This file is part of the Amber project                      //
//  http://www.opencores.org/project,amber                      //
//                                                              //
//  Description                                                 //
//  This module is the most complex part of the Amber core      //
//  It decodes and sequences all instructions and handles all   //
//  interrupts                                                  //
//                                                              //
//  Author(s):                                                  //
//      - Conor Santifort, csantifort.amber@gmail.com           //
//                                                              //
//////////////////////////////////////////////////////////////////
//                                                              //
// Copyright (C) 2010 Authors and OPENCORES.ORG                 //
//                                                              //
// This source file may be used and distributed without         //
// restriction provided that this copyright statement is not    //
// removed from the file and that any derivative work contains  //
// the original copyright notice and the associated disclaimer. //
//                                                              //
// This source file is free software; you can redistribute it   //
// and/or modify it under the terms of the GNU Lesser General   //
// Public License as published by the Free Software Foundation; //
// either version 2.1 of the License, or (at your option) any   //
// later version.                                               //
//                                                              //
// This source is distributed in the hope that it will be       //
// useful, but WITHOUT ANY WARRANTY; without even the implied   //
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR      //
// PURPOSE.  See the GNU Lesser General Public License for more //
// details.                                                     //
//                                                              //
// You should have received a copy of the GNU Lesser General    //
// Public License along with this source; if not, download it   //
// from http://www.opencores.org/lgpl.shtml                     //
//                                                              //
//////////////////////////////////////////////////////////////////


module a23_decode
(
input                       i_clk,
input       [31:0]          i_read_data,
input                       i_fetch_stall,                  // stall all stages of the cpu at the same time
input                       i_irq,                          // interrupt request
input                       i_firq,                         // Fast interrupt request
input                       i_dabt,                         // data abort interrupt request
input                       i_iabt,                         // instruction pre-fetch abort flag
input                       i_adex,                         // Address Exception
input       [31:0]          i_execute_address,              // Registered address output by execute stage
                                                            // 2 LSBs of read address used for calculating
                                                            // shift in LDRB ops
input       [7:0]           i_abt_status,                   // Abort status
input       [31:0]          i_execute_status_bits,          // current status bits values in execute stage
input                       i_multiply_done,                // multiply unit is nearly done
input                       i_lua_mode,                     // whether we should decode as lua or as arm
input                       i_prefetch_instruction,         // whether or not we should prefetch our instruction
input       [4:0]           i_lua_inst_stage,               // current lua inst stage

// --------------------------------------------------
// Control signals to execute stage
// --------------------------------------------------
output reg  [31:0]          o_read_data = 'd0,
output reg  [4:0]           o_read_data_alignment = 'd0,  // 2 LSBs of read address used for calculating shift in LDRB ops

output reg  [31:0]          o_imm32 = 'd0,
output reg  [4:0]           o_imm_shift_amount = 'd0,
output reg                  o_shift_imm_zero = 'd0,
output reg  [3:0]           o_condition = 4'he,             // 4'he = al
output reg                  o_exclusive_exec = 'd0,         // exclusive access request ( swap instruction )
output reg                  o_data_access_exec = 'd0,       // high means the memory access is a read
                                                            // read or write, low for instruction
output reg  [1:0]           o_status_bits_mode = 2'b11,     // SVC
output reg                  o_status_bits_irq_mask = 1'd1,
output reg                  o_status_bits_firq_mask = 1'd1,

output reg  [3:0]           o_rm_sel = 'd0,
output reg  [3:0]           o_rds_sel = 'd0,
output reg  [3:0]           o_rn_sel = 'd0,
output reg  [1:0]           o_barrel_shift_amount_sel = 'd0,
output reg  [1:0]           o_barrel_shift_data_sel = 'd0,
output reg  [1:0]           o_barrel_shift_function = 'd0,
output reg  [8:0]           o_alu_function = 'd0,
output reg  [1:0]           o_multiply_function = 'd0,
output reg  [2:0]           o_interrupt_vector_sel = 'd0,
output reg  [3:0]           o_address_sel = 4'd2,
output reg  [2:0]           o_pc_sel = 3'd2,
output reg  [1:0]           o_byte_enable_sel = 'd0,        // byte, halfword or word write
output reg  [2:0]           o_status_bits_sel = 'd0,
output reg  [3:0]           o_reg_write_sel,
output reg                  o_user_mode_regs_load,
output reg                  o_user_mode_regs_store_nxt,
output reg                  o_firq_not_user_mode,

output reg                  o_write_data_update_wen = 'd0,
output reg                  o_write_data_wen = 'd0,
output reg                  o_base_address_wen = 'd0,       // save LDM base address register
                                                            // in case of data abort
output reg                  o_pc_wen = 1'd1,
output reg  [14:0]          o_reg_bank_wen = 'd0,
output reg                  o_status_bits_flags_wen = 'd0,
output reg                  o_status_bits_mode_wen = 'd0,
output reg                  o_status_bits_irq_mask_wen = 'd0,
output reg                  o_status_bits_firq_mask_wen = 'd0,
output reg                  o_lua_mode = 'd0,
output reg                  o_prefetch_instruction = 'd0,
output reg [4:0]            o_lua_inst_stage_nxt = 'd0,
output [31:0] o_dbg_instruction_address,
output [5:0] o_control_state,

// --------------------------------------------------
// Co-Processor interface
// --------------------------------------------------
output reg  [2:0]           o_copro_opcode1 = 'd0,
output reg  [2:0]           o_copro_opcode2 = 'd0,
output reg  [3:0]           o_copro_crn = 'd0,
output reg  [3:0]           o_copro_crm = 'd0,
output reg  [3:0]           o_copro_num = 'd0,
output reg  [1:0]           o_copro_operation = 'd0, // 0 = no operation,
                                                     // 1 = Move to Amber Core Register from Coprocessor
                                                     // 2 = Move to Coprocessor from Amber Core Register
output reg                  o_copro_write_data_wen = 'd0,
output                      o_iabt_trigger,
output      [31:0]          o_iabt_address,
output      [7:0]           o_iabt_status,
output                      o_dabt_trigger,
output      [31:0]          o_dabt_address,
output      [7:0]           o_dabt_status


);

`include "a23_localparams.v"
`include "a23_functions.v"

localparam [5:0] RST_WAIT1      = 'd0,
                 RST_WAIT2      = 'd1,
                 INT_WAIT1      = 'd2,
                 INT_WAIT2      = 'd3,
                 EXECUTE        = 'd4,
                 PRE_FETCH_EXEC = 'd5,  // Execute the Pre-Fetched Instruction
                 MEM_WAIT1      = 'd6,  // conditionally decode current instruction, in case
                                         // previous instruction does not execute in S2
                 MEM_WAIT2      = 'd7,
                 PC_STALL1      = 'd8,  // Program Counter altered
                                         // conditionally decude current instruction, in case
                                         // previous instruction does not execute in S2
                 PC_STALL2      = 'd9,
                 MTRANS_EXEC1   = 'd10,
                 MTRANS_EXEC2   = 'd11,
                 MTRANS_EXEC3   = 'd12,
                 MTRANS_EXEC3B  = 'd13,
                 MTRANS_EXEC4   = 'd14,
                 MTRANS5_ABORT  = 'd15,
                 MULT_PROC1     = 'd16,  // first cycle, save pre fetch instruction
                 MULT_PROC2     = 'd17,  // do multiplication
                 MULT_STORE     = 'd18,  // save RdLo
                 MULT_ACCUMU    = 'd19,  // Accumulate add lower 32 bits
                 SWAP_WRITE     = 'd20,
                 SWAP_WAIT1     = 'd21,
                 SWAP_WAIT2     = 'd22,
                 COPRO_WAIT     = 'd23,

                 LUA_DECODE     = 'd24,
                 LUA_LOAD1      = 'd25, // load remaining arg
                 LUA_LOAD1F     = 'd26, // load remaining arg, cycle 2
                 LUA_LOAD2      = 'd27, // load 2 remaining args
                 LUA_LOAD2F     = 'd28, // load 2 remaining args, cycle 2
                 LUA_EXEC       = 'd29,
                 LUA_STOREA     = 'd30,
                 LUA_STOREA2    = 'd31,
                 LUA_PC_STALL   = 'd32,
                 LUA_PC_STALL2  = 'd33;



// ========================================================
// Internal signals
// ========================================================
wire    [31:0]         instruction;
wire                   instruction_iabt;        // abort flag, follows the instruction
wire                   instruction_adex;        // address exception flag, follows the instruction
wire    [31:0]         instruction_address;     // instruction virtual address, follows
                                                // the instruction
wire    [7:0]          instruction_iabt_status; // abort status, follows the instruction
wire    [1:0]          instruction_sel;
reg     [4:0]          itype;
wire    [5:0]          opcode;
wire    [7:0]          imm8;
wire    [31:0]         offset12;
wire    [31:0]         offsetbxl;
wire    [31:0]         offset24;
wire    [31:0]         branch_bxl_offset;
wire    [31:0]         lua_shift;
wire    [31:0]         offseta;
wire                   is_bx;
wire    [31:0]         offsetb;
wire    [31:0]         offsetc;
wire    [4:0]          shift_imm;

wire                   opcode_compare;
wire                   mem_op;
wire                   load_op;
wire                   store_op;
wire                   write_pc;
wire                   immediate_shifter_operand;
wire                   rds_use_rs;
wire                   branch;
wire                   is_bxl;
wire                   mem_op_pre_indexed;
wire                   mem_op_post_indexed;

// Flop inputs
wire    [31:0]         imm32_nxt;
wire    [4:0]          imm_shift_amount_nxt;
wire                   shift_imm_zero_nxt;
wire    [3:0]          condition_nxt;
reg                    exclusive_exec_nxt;
reg                    data_access_exec_nxt;

reg     [1:0]          barrel_shift_function_nxt;
wire    [8:0]          alu_function_nxt;
reg     [1:0]          multiply_function_nxt;
reg     [1:0]          status_bits_mode_nxt;
reg                    status_bits_irq_mask_nxt;
reg                    status_bits_firq_mask_nxt;

wire    [8:0]          lua_b_sel_nxt;
wire    [8:0]          lua_c_sel_nxt;
wire    [7:0]          lua_bk_sel_nxt;
wire    [7:0]          lua_ck_sel_nxt;
wire    [3:0]          lua_bk_base;
wire    [3:0]          lua_ck_base;
wire    [17:0]         lua_bx_sel_nxt;
wire    [17:0]         lua_sbx_sel_nxt;
reg     [4:0]          lua_inst_stage_reg = 'd0;

reg     [31:0]         lua_custom_imm32 = 'd0;
reg     [31:0]         lua_scratch_reg = 'd0;
wire                   lua_scratch_reg_wen;

wire    [3:0]          rm_sel_nxt;
wire    [3:0]          rds_sel_nxt;
wire    [3:0]          rn_sel_nxt;
reg     [1:0]          barrel_shift_amount_sel_nxt;
reg     [1:0]          barrel_shift_data_sel_nxt;
reg     [3:0]          address_sel_nxt;
reg     [2:0]          pc_sel_nxt;
reg     [1:0]          byte_enable_sel_nxt;
reg     [2:0]          status_bits_sel_nxt;
reg     [3:0]          reg_write_sel_nxt;
reg                    user_mode_regs_load_nxt;
wire                   firq_not_user_mode_nxt;

// ALU Function signals
reg                    alu_swap_sel_nxt;
reg                    alu_not_sel_nxt;
reg     [1:0]          alu_cin_sel_nxt;
reg                    alu_cout_sel_nxt;
reg     [3:0]          alu_out_sel_nxt;

reg                    write_data_update_wen_nxt;
reg                    write_data_wen_nxt;
reg                    copro_write_data_wen_nxt;
reg                    base_address_wen_nxt;
reg                    pc_wen_nxt;
reg     [14:0]         reg_bank_wen_nxt;
reg                    status_bits_flags_wen_nxt;
reg                    status_bits_mode_wen_nxt;
reg                    status_bits_irq_mask_wen_nxt;
reg                    status_bits_firq_mask_wen_nxt;

reg                    saved_current_instruction_wen;   // saved load instruction
reg                    pre_fetch_instruction_wen;       // pre-fetch instruction

reg     [5:0]          control_state = RST_WAIT1;
reg     [5:0]          control_state_nxt;

reg                    lua_fetch_1 = 'd0;
reg                    lua_fetch_2 = 'd0;
reg                    lua_store_a = 'd0;
reg                    lua_prefetch_1 = 'd0;
reg                    lua_pc_stall = 'd0;

wire    [31:0]         lua_fetch_arg1;

wire                   dabt;
reg                    dabt_reg = 'd0;
reg                    dabt_reg_d1;
reg                    iabt_reg = 'd0;
reg                    adex_reg = 'd0;
reg     [31:0]         abt_address_reg = 'd0;
reg     [7:0]          abt_status_reg = 'd0;
reg     [31:0]         saved_current_instruction = 'd0;
reg                    saved_current_instruction_iabt = 'd0;          // access abort flag
reg                    saved_current_instruction_adex = 'd0;          // address exception
reg     [31:0]         saved_current_instruction_address = 'd0;       // virtual address of abort instruction
reg     [7:0]          saved_current_instruction_iabt_status = 'd0;   // status of abort instruction
reg     [31:0]         pre_fetch_instruction = 'd0;
reg                    pre_fetch_instruction_iabt = 'd0;              // access abort flag
reg                    pre_fetch_instruction_adex = 'd0;              // address exception
reg     [31:0]         pre_fetch_instruction_address = 'd0;           // virtual address of abort instruction
reg     [7:0]          pre_fetch_instruction_iabt_status = 'd0;       // status of abort instruction

wire                   instruction_valid;
wire                   instruction_execute;

reg     [3:0]          lua_rn_reg = 'd0;
reg     [3:0]          lua_rm_reg = 'd0;
reg     [3:0]          lua_rd_reg = 'd0;
reg     [3:0]          mtrans_reg;              // the current register being accessed as part of STM/LDM
reg     [3:0]          mtrans_reg_d1 = 'd0;     // delayed by 1 period
reg     [3:0]          mtrans_reg_d2 = 'd0;     // delayed by 2 periods
reg     [31:0]         mtrans_instruction_nxt;

wire   [31:0]          mtrans_base_reg_change;
wire   [4:0]           mtrans_num_registers;
wire                   use_saved_current_instruction;
wire                   use_pre_fetch_instruction;
wire                   interrupt;
wire   [1:0]           interrupt_mode;
wire   [2:0]           next_interrupt;
reg                    irq = 'd0;
reg                    firq = 'd0;
wire                   firq_request;
wire                   irq_request;
wire                   swi_request;
wire                   und_request;
wire                   dabt_request;
reg    [1:0]           copro_operation_nxt;
reg                    mtrans_r15 = 'd0;
reg                    mtrans_r15_nxt;
reg                    restore_base_address = 'd0;
reg                    restore_base_address_nxt;

reg                    lua_mode_nxt;

wire                   regop_set_flags;


// ========================================================
// Instruction Abort and Data Abort outputs
// ========================================================

assign o_iabt_trigger     = instruction_iabt && o_status_bits_mode == SVC && control_state == INT_WAIT1;
assign o_iabt_address     = instruction_address;
assign o_iabt_status      = instruction_iabt_status;

assign o_dabt_trigger     = dabt_reg && !dabt_reg_d1;
assign o_dabt_address     = abt_address_reg;
assign o_dabt_status      = abt_status_reg;


// ========================================================
// Instruction Decode
// ========================================================

// for instructions that take more than one cycle
// the instruction is saved in the 'saved_mem_instruction'
// register and then that register is used for the rest of
// the execution of the instruction.
// But if the instruction does not execute because of the
// condition, then need to select the next instruction to
// decode
assign use_saved_current_instruction =  instruction_execute &&
                          ( control_state == MEM_WAIT1     ||
                            control_state == MEM_WAIT2     ||
                            control_state == MTRANS_EXEC1  ||
                            control_state == MTRANS_EXEC2  ||
                            control_state == MTRANS_EXEC3  ||
                            control_state == MTRANS_EXEC3B ||
                            control_state == MTRANS_EXEC4  ||
                            control_state == MTRANS5_ABORT ||
                            control_state == MULT_PROC1    ||
                            control_state == MULT_PROC2    ||
                            control_state == MULT_ACCUMU   ||
                            control_state == MULT_STORE    ||
                            control_state == INT_WAIT1     ||
                            control_state == INT_WAIT2     ||
                            control_state == SWAP_WRITE    ||
                            control_state == SWAP_WAIT1    ||
                            control_state == SWAP_WAIT2    ||
                            control_state == COPRO_WAIT    ||
                            control_state == LUA_LOAD1     ||
                            control_state == LUA_LOAD1F    ||
                            control_state == LUA_LOAD2     ||
                            control_state == LUA_LOAD2F    ||
                            control_state == LUA_EXEC      ||
                            control_state == LUA_STOREA    ||
                            control_state == LUA_STOREA2 );

assign use_pre_fetch_instruction = control_state == PRE_FETCH_EXEC;


assign instruction_sel  =         use_saved_current_instruction  ? 2'd1 :  // saved_current_instruction
                                  use_pre_fetch_instruction      ? 2'd2 :  // pre_fetch_instruction
                                                                   2'd0 ;  // o_read_data

assign instruction      =         instruction_sel == 2'd0 ? o_read_data               :
                                  instruction_sel == 2'd1 ? saved_current_instruction :
                                                            pre_fetch_instruction     ;

// abort flag
assign instruction_iabt =         instruction_sel == 2'd0 ? iabt_reg                       :
                                  instruction_sel == 2'd1 ? saved_current_instruction_iabt :
                                                            pre_fetch_instruction_iabt     ;

assign instruction_address =      instruction_sel == 2'd0 ? abt_address_reg                   :
                                  instruction_sel == 2'd1 ? saved_current_instruction_address :
                                                            pre_fetch_instruction_address     ;

assign instruction_iabt_status =  instruction_sel == 2'd0 ? abt_status_reg                        :
                                  instruction_sel == 2'd1 ? saved_current_instruction_iabt_status :
                                                            pre_fetch_instruction_iabt_status     ;

// instruction address exception
assign instruction_adex =         instruction_sel == 2'd0 ? adex_reg                       :
                                  instruction_sel == 2'd1 ? saved_current_instruction_adex :
                                                            pre_fetch_instruction_adex     ;

// Instruction Decode - Order is important!
always @*
    casez ({i_lua_mode, instruction[27:20], instruction[7:0]})
        17'b000010?001001???? : itype = SWAP;
        17'b0000000??1001???? : itype = MULT;
        17'b000??????0??0???? : itype = REGOP;
        17'b000??????0??1???? : itype = REGOP;
        17'b000??????1??0???? : itype = REGOP;
        17'b0001?????1??1???? : itype = REGOP;
        17'b001?????????????? : itype = TRANS;
        17'b0100????????????? : itype = MTRANS;
        17'b?101????????????? : itype = BRANCH;
        17'b0110????????????? : itype = CODTRANS;
        17'b01110???????0???? : itype = COREGOP;
        17'b01110???????1???? : itype = CORTRANS;

        // these are a little more annoying to categorize
        17'b1??????????000000 : itype = LUA_MOVE;  // OP_MOVE
        17'b1??????????000001 : itype = LUA_MOVE;  // OP_LOADK
        17'b1??????????000010 : itype = SWI;  // OP_LOADKX
        17'b1??????????000011 : itype = LUA_LOADBOOL;  // OP_LOADBOOL
        17'b1??????????000100 : itype = LUA_LOADNIL;  // OP_LOADNIL
        17'b1??????????000101 : itype = LUA_GET;  // OP_GETUPVAL

        17'b1??????????000110 : itype = LUA_GET;  // OP_GETTABUP
        17'b1??????????000111 : itype = LUA_GET;  // OP_GETTABLE

        17'b1??????????001000 : itype = LUA_SET2;  // OP_SETTABUP
        17'b1??????????001001 : itype = LUA_SET; // OP_SETUPVAL
        17'b1??????????001010 : itype = LUA_SET2; // OP_SETTABLE

        17'b1??????????001011 : itype = LUA_TABLE;  // OP_NEWTABLE

        17'b1??????????001100 : itype = LUA_SELF;  // OP_SELF

        17'b1??????????001101 : itype = LUA_ALU2;  // OP_ADD
        17'b1??????????001110 : itype = LUA_ALU2; // OP_SUB
        17'b1??????????001111 : itype = SWI; // OP_MUL
        17'b1??????????010000 : itype = SWI; // OP_MOD
        17'b1??????????010001 : itype = SWI; // OP_POW
        17'b1??????????010010 : itype = SWI; // OP_DIV
        17'b1??????????010011 : itype = SWI; // OP_IDIV
        17'b1??????????010100 : itype = LUA_ALU2; // OP_BAND
        17'b1??????????010101 : itype = LUA_ALU2; // OP_BOR
        17'b1??????????010110 : itype = LUA_ALU2; // OP_BXOR
        17'b1??????????010111 : itype = LUA_ALU2; // OP_SHL
        17'b1??????????011000 : itype = LUA_ALU2; // OP_SHR
        17'b1??????????011001 : itype = LUA_ALU2; // OP_UNM
        17'b1??????????011010 : itype = LUA_ALU2; // OP_BNOT
        17'b1??????????011011 : itype = LUA_ALU2; // OP_NOT
        17'b1??????????011100 : itype = LUA_LEN; // OP_LEN

        17'b1??????????011101 : itype = LUA_CONCAT; // OP_CONCAT

        17'b1??????????011110 : itype = LUA_BRANCH; // OP_JMP
        17'b1??????????011111 : itype = LUA_CMP; // OP_EQ
        17'b1??????????100000 : itype = LUA_CMP; // OP_LT
        17'b1??????????100001 : itype = LUA_CMP; // OP_LE

        17'b1??????????100010 : itype = LUA_TEST; // OP_TEST
        17'b1??????????100011 : itype = LUA_TEST; // OP_TESTSET

        17'b1??????????100100 : itype = LUA_CALL; // OP_CALL
        17'b1??????????100101 : itype = LUA_UNKNOWN; // OP_TAILCALL
        17'b1??????????100110 : itype = LUA_RETURN; // OP_RETURN

        17'b1??????????100111 : itype = LUA_FORLOOP; // OP_FORLOOP
        17'b1??????????101000 : itype = LUA_FORPREP; // OP_FORPREP
        17'b1??????????101001 : itype = LUA_UNKNOWN; // OP_TFORCALL
        17'b1??????????101010 : itype = LUA_UNKNOWN; // OP_TFORLOOP

        17'b1??????????101011 : itype = LUA_SETLIST; // OP_SETLIST

        17'b1??????????101100 : itype = LUA_CLOSURE; // OP_CLOSURE

        17'b1??????????101101 : itype = LUA_UNKNOWN; // OP_VARARG

        17'b1??????????101110 : itype = LUA_UNKNOWN; // OP_EXTRAARG
        default:           itype = SWI; // tons of ignored bits here, so just make it default
    endcase

// ========================================================
// Fixed fields within the instruction
// ========================================================

assign opcode        = i_lua_mode ? instruction[5:0] : ({2'b0, instruction[24:21]});
assign condition_nxt = i_lua_mode ? AL : instruction[31:28];

assign rm_sel_nxt    = lua_mode_nxt ? lua_rm_reg : instruction[3:0];

assign rn_sel_nxt    = lua_mode_nxt ? lua_rn_reg :
                       branch  ? 4'd15             : // Use PC to calculate branch destination
                                 instruction[19:16] ;

assign rds_sel_nxt   = lua_mode_nxt ? lua_rd_reg :
                       control_state == SWAP_WRITE  ? instruction[3:0]   : // Rm gets written out to memory
                       itype == MTRANS               ? mtrans_reg      :
                       branch                       ? 4'd15              : // Update the PC
                       rds_use_rs                   ? instruction[11:8]  :
                                                      instruction[15:12] ;

assign lua_b_sel_nxt = instruction[31:23];
assign lua_c_sel_nxt = instruction[22:14];
assign lua_bk_sel_nxt = instruction[30:23];
assign lua_bk_base    = instruction[31] ? CONSTANTS : BASE;
assign lua_ck_sel_nxt = instruction[21:14];
assign lua_ck_base    = instruction[22] ? CONSTANTS : BASE;
assign lua_bx_sel_nxt = instruction[31:14];
assign lua_sbx_sel_nxt = instruction[31:14];

assign shift_imm     = lua_mode_nxt ? 5'd2 : instruction[11:7];
assign offset12      = { 20'h0, instruction[11:0]};
assign offsetbxl      = {{21{instruction[10]}}, instruction[10:8], instruction[6:5], instruction[3:0], 2'd0 }; // sign extend
assign offset24      = {{6{instruction[23]}}, instruction[23:0], 2'd0 }; // sign extend
assign offseta       = {24'd0, instruction[13:6]};
assign is_bx         = instruction[5:0] == LUA_OP_CLOSURE || instruction[5:0] == LUA_OP_LOADK;
assign offsetb       = is_bx ? {15'd0, instruction[31:15]} : {24'd0, instruction[22:15]};
assign offsetc       = {24'd0, instruction[31:24]};
assign imm8          = instruction[7:0];

assign immediate_shifter_operand = instruction[25];
assign rds_use_rs                = (itype == REGOP && !instruction[25] && instruction[4]) ||
                                   (itype == MULT &&
                                    (control_state == MULT_PROC1  ||
                                     control_state == MULT_PROC2  ||
                                     instruction_valid && !interrupt )) ;
assign branch                    = itype == BRANCH;
assign is_bxl                    = offset24 == 32'hcccc;
assign branch_bxl_offset         = is_bxl ? 32'h0 : offset24;
assign opcode_compare =
            opcode == CMP ||
            opcode == CMN ||
            opcode == TEQ ||
            opcode == TST ||
            (i_lua_mode && itype == LUA_CMP);


assign mem_op               = (!i_lua_mode) && itype == TRANS;
assign load_op              = mem_op && instruction[20];
assign store_op             = mem_op && !instruction[20];
assign write_pc             = pc_wen_nxt && pc_sel_nxt != 3'd0;
assign regop_set_flags      = itype == REGOP && instruction[20];

assign mem_op_pre_indexed   =  instruction[24] && instruction[21];
assign mem_op_post_indexed  = !instruction[24];

assign lua_fetch_arg1 = 
    itype == LUA_MOVE && is_bx ? {14'd0, lua_bx_sel_nxt} :
    itype == LUA_MOVE ? {23'd0, lua_b_sel_nxt} :
    itype == LUA_CLOSURE ? {14'd0, lua_bx_sel_nxt} :
                        {32'd0} ;

assign lua_scratch_reg_wen = control_state == LUA_LOAD2F;

assign imm32_nxt            =  // add 0 to Rm
                               itype == MULT               ? {  32'd0                      } :

                               // 4 x number of registers
                               itype == MTRANS             ? {  mtrans_base_reg_change    } :
                               itype == BRANCH             ? {  branch_bxl_offset         } :
                               itype == TRANS              ? {  offset12                  } :
                               lua_inst_stage_reg != 'd0   ? {  lua_custom_imm32          } :
                               lua_store_a                 ? {  offseta                   } :
                               lua_fetch_1                 ? {  lua_fetch_arg1            } :

                               control_state == LUA_EXEC && opcode == LUA_OP_UNM
                                                          ? {  32'd1                      } :
                               control_state == LUA_EXEC && opcode == LUA_OP_NOT
                                                          ? {  32'hffffffff               } :
                               instruction[11:8] == 4'h0  ? {            24'h0, imm8[7:0] } :
                               instruction[11:8] == 4'h1  ? { imm8[1:0], 24'h0, imm8[7:2] } :
                               instruction[11:8] == 4'h2  ? { imm8[3:0], 24'h0, imm8[7:4] } :
                               instruction[11:8] == 4'h3  ? { imm8[5:0], 24'h0, imm8[7:6] } :
                               instruction[11:8] == 4'h4  ? { imm8[7:0], 24'h0            } :
                               instruction[11:8] == 4'h5  ? { 2'h0,  imm8[7:0], 22'h0     } :
                               instruction[11:8] == 4'h6  ? { 4'h0,  imm8[7:0], 20'h0     } :
                               instruction[11:8] == 4'h7  ? { 6'h0,  imm8[7:0], 18'h0     } :
                               instruction[11:8] == 4'h8  ? { 8'h0,  imm8[7:0], 16'h0     } :
                               instruction[11:8] == 4'h9  ? { 10'h0, imm8[7:0], 14'h0     } :
                               instruction[11:8] == 4'ha  ? { 12'h0, imm8[7:0], 12'h0     } :
                               instruction[11:8] == 4'hb  ? { 14'h0, imm8[7:0], 10'h0     } :
                               instruction[11:8] == 4'hc  ? { 16'h0, imm8[7:0], 8'h0      } :
                               instruction[11:8] == 4'hd  ? { 18'h0, imm8[7:0], 6'h0      } :
                               instruction[11:8] == 4'he  ? { 20'h0, imm8[7:0], 4'h0      } :
                                                            { 22'h0, imm8[7:0], 2'h0      } ;

assign imm_shift_amount_nxt = shift_imm ;

       // This signal is encoded in the decode stage because
       // it is on the critical path in the execute stage
assign shift_imm_zero_nxt   = imm_shift_amount_nxt == 5'd0 &&       // immediate amount = 0
                              barrel_shift_amount_sel_nxt == 2'd2;  // shift immediate amount

assign alu_function_nxt     = { alu_swap_sel_nxt,
                                alu_not_sel_nxt,
                                alu_cin_sel_nxt,
                                alu_cout_sel_nxt,
                                alu_out_sel_nxt  };

assign lua_mode_nxt         = !(next_interrupt != 'd0) && (is_bxl || (itype != BRANCH && i_lua_mode));

// ========================================================
// MTRANS Operations
// ========================================================

   // Bit 15 = r15
   // Bit 0  = R0
   // In LDM and STM instructions R0 is loaded or stored first
always @*
    casez (instruction[15:0])
    16'b???????????????1 : mtrans_reg = 4'h0 ;
    16'b??????????????10 : mtrans_reg = 4'h1 ;
    16'b?????????????100 : mtrans_reg = 4'h2 ;
    16'b????????????1000 : mtrans_reg = 4'h3 ;
    16'b???????????10000 : mtrans_reg = 4'h4 ;
    16'b??????????100000 : mtrans_reg = 4'h5 ;
    16'b?????????1000000 : mtrans_reg = 4'h6 ;
    16'b????????10000000 : mtrans_reg = 4'h7 ;
    16'b???????100000000 : mtrans_reg = 4'h8 ;
    16'b??????1000000000 : mtrans_reg = 4'h9 ;
    16'b?????10000000000 : mtrans_reg = 4'ha ;
    16'b????100000000000 : mtrans_reg = 4'hb ;
    16'b???1000000000000 : mtrans_reg = 4'hc ;
    16'b??10000000000000 : mtrans_reg = 4'hd ;
    16'b?100000000000000 : mtrans_reg = 4'he ;
    default              : mtrans_reg = 4'hf ;
    endcase

always @*
    casez (instruction[15:0])
    16'b???????????????1 : mtrans_instruction_nxt = {instruction[31:16], instruction[15: 1],  1'd0};
    16'b??????????????10 : mtrans_instruction_nxt = {instruction[31:16], instruction[15: 2],  2'd0};
    16'b?????????????100 : mtrans_instruction_nxt = {instruction[31:16], instruction[15: 3],  3'd0};
    16'b????????????1000 : mtrans_instruction_nxt = {instruction[31:16], instruction[15: 4],  4'd0};
    16'b???????????10000 : mtrans_instruction_nxt = {instruction[31:16], instruction[15: 5],  5'd0};
    16'b??????????100000 : mtrans_instruction_nxt = {instruction[31:16], instruction[15: 6],  6'd0};
    16'b?????????1000000 : mtrans_instruction_nxt = {instruction[31:16], instruction[15: 7],  7'd0};
    16'b????????10000000 : mtrans_instruction_nxt = {instruction[31:16], instruction[15: 8],  8'd0};
    16'b???????100000000 : mtrans_instruction_nxt = {instruction[31:16], instruction[15: 9],  9'd0};
    16'b??????1000000000 : mtrans_instruction_nxt = {instruction[31:16], instruction[15:10], 10'd0};
    16'b?????10000000000 : mtrans_instruction_nxt = {instruction[31:16], instruction[15:11], 11'd0};
    16'b????100000000000 : mtrans_instruction_nxt = {instruction[31:16], instruction[15:12], 12'd0};
    16'b???1000000000000 : mtrans_instruction_nxt = {instruction[31:16], instruction[15:13], 13'd0};
    16'b??10000000000000 : mtrans_instruction_nxt = {instruction[31:16], instruction[15:14], 14'd0};
    16'b?100000000000000 : mtrans_instruction_nxt = {instruction[31:16], instruction[15   ], 15'd0};
    default              : mtrans_instruction_nxt = {instruction[31:16],                     16'd0};
    endcase


// number of registers to be stored
assign mtrans_num_registers =   {4'd0, instruction[15]} +
                                {4'd0, instruction[14]} +
                                {4'd0, instruction[13]} +
                                {4'd0, instruction[12]} +
                                {4'd0, instruction[11]} +
                                {4'd0, instruction[10]} +
                                {4'd0, instruction[ 9]} +
                                {4'd0, instruction[ 8]} +
                                {4'd0, instruction[ 7]} +
                                {4'd0, instruction[ 6]} +
                                {4'd0, instruction[ 5]} +
                                {4'd0, instruction[ 4]} +
                                {4'd0, instruction[ 3]} +
                                {4'd0, instruction[ 2]} +
                                {4'd0, instruction[ 1]} +
                                {4'd0, instruction[ 0]} ;

// 4 x number of registers to be stored
assign mtrans_base_reg_change = {25'd0, mtrans_num_registers, 2'd0};

// ========================================================
// Interrupts
// ========================================================

assign firq_request = firq && !i_execute_status_bits[26];
assign irq_request  = irq  && !i_execute_status_bits[27];
assign swi_request  = itype == SWI;
assign dabt_request = dabt_reg;

// copro15 and copro13 only supports reg trans opcodes
// all other opcodes involving co-processors cause an
// undefined instrution interrupt
// assign und_request  =   itype == CODTRANS ||
//                         itype == COREGOP  ||
//                       ( itype == CORTRANS && instruction[11:8] != 4'd15 );
assign und_request = 1'd0;

  // in order of priority !!
  // Highest
  // 1 Reset
  // 2 Data Abort (including data TLB miss)
  // 3 FIRQ
  // 4 IRQ
  // 5 Prefetch Abort (including prefetch TLB miss)
  // 6 Undefined instruction, SWI
  // Lowest
assign next_interrupt = dabt_request     ? 3'd1 :  // Data Abort
                        firq_request     ? 3'd2 :  // FIRQ
                        irq_request      ? 3'd3 :  // IRQ
                        instruction_adex ? 3'd4 :  // Address Exception
                        instruction_iabt ? 3'd5 :  // PreFetch Abort, only triggered
                                                   // if the instruction is used
                        und_request      ? 3'd6 :  // Undefined Instruction
                        swi_request      ? 3'd7 :  // SWI
                                           3'd0 ;  // none

        // SWI and undefined instructions do not cause an interrupt in the decode
        // stage. They only trigger interrupts if they arfe executed, so the
        // interrupt is triggered if the execute condition is met in the execute stage
assign interrupt      = next_interrupt != 3'd0 &&
                        next_interrupt != 3'd7 &&  // SWI
                        next_interrupt != 3'd6 ;   // undefined interrupt


assign interrupt_mode = next_interrupt == 3'd2 ? FIRQ :
                        next_interrupt == 3'd3 ? IRQ  :
                        next_interrupt == 3'd4 ? SVC  :
                        next_interrupt == 3'd5 ? SVC  :
                        next_interrupt == 3'd6 ? SVC  :
                        next_interrupt == 3'd7 ? SVC  :
                        next_interrupt == 3'd1 ? SVC  :
                                                 USR  ;




// ========================================================
// Generate control signals
// ========================================================
always @*
    begin
    // default mode
    status_bits_mode_nxt            = i_execute_status_bits[1:0];   // change to mode in execute stage get reflected
                                                                    // back to this stage automatically
    status_bits_irq_mask_nxt        = o_status_bits_irq_mask;
    status_bits_firq_mask_nxt       = o_status_bits_firq_mask;
    exclusive_exec_nxt              = 1'd0;
    data_access_exec_nxt            = 1'd0;
    copro_operation_nxt             = 'd0;

    // Save an instruction to use later
    saved_current_instruction_wen   = 1'd0;
    pre_fetch_instruction_wen       = 1'd0;
    mtrans_r15_nxt                  = mtrans_r15;
    restore_base_address_nxt        = restore_base_address;

    // default Mux Select values
    barrel_shift_amount_sel_nxt     = 'd0;  // don't shift the input
    barrel_shift_data_sel_nxt       = 'd0;  // immediate value
    barrel_shift_function_nxt       = 'd0;
    multiply_function_nxt           = 'd0;
    address_sel_nxt                 = 'd0;
    pc_sel_nxt                      = 'd0;
    byte_enable_sel_nxt             = 'd0;
    status_bits_sel_nxt             = 'd0;
    reg_write_sel_nxt               = 'd0;
    user_mode_regs_load_nxt         = 'd0;
    o_user_mode_regs_store_nxt      = 'd0;

    // ALU Muxes
    alu_swap_sel_nxt                = 'd0;
    alu_not_sel_nxt                 = 'd0;
    alu_cin_sel_nxt                 = 'd0;
    alu_cout_sel_nxt                = 'd0;
    alu_out_sel_nxt                 = 'd0;

    // default Flop Write Enable values
    write_data_update_wen_nxt       = 'd1;
    write_data_wen_nxt              = 'd0;
    copro_write_data_wen_nxt        = 'd0;
    base_address_wen_nxt            = 'd0;
    pc_wen_nxt                      = o_lua_inst_stage_nxt == 'd0;
    reg_bank_wen_nxt                = 'd0;  // Don't select any
    status_bits_flags_wen_nxt       = 'd0;
    status_bits_mode_wen_nxt        = 'd0;
    status_bits_irq_mask_wen_nxt    = 'd0;
    status_bits_firq_mask_wen_nxt   = 'd0;

    lua_rm_reg                      = 'd0;
    lua_rn_reg                      = 'd0;
    lua_rd_reg                      = 'd0;
    lua_fetch_1                     = 'd0;
    lua_fetch_2                     = 'd0;
    lua_store_a                     = 'd0;
    lua_prefetch_1                  = 'd0;
    lua_pc_stall                    = 'd0;
    lua_custom_imm32                = 'd0;
    lua_inst_stage_reg              = i_lua_inst_stage;

    if ( instruction_valid && !interrupt )
        begin

        if ( i_prefetch_instruction )
            begin
                pre_fetch_instruction_wen   = 1'd1;
            end

        if ( itype == REGOP )
            begin
            if ( !opcode_compare )
                begin
                // Check is the load destination is the PC
                if (instruction[15:12]  == 4'd15)
                    begin
                    pc_sel_nxt      = 3'd1; // alu_out
                    address_sel_nxt = 4'd1; // alu_out
                    end
                else
                    reg_bank_wen_nxt = decode (instruction[15:12]);
                end

            if ( !immediate_shifter_operand )
                barrel_shift_function_nxt  = instruction[6:5];

            if ( !immediate_shifter_operand )
                barrel_shift_data_sel_nxt = 2'd2; // Shift value from Rm register

            if ( !immediate_shifter_operand && instruction[4] )
                barrel_shift_amount_sel_nxt = 2'd1; // Shift amount from Rs registter

            if ( !immediate_shifter_operand && !instruction[4] )
                barrel_shift_amount_sel_nxt = 2'd2; // Shift immediate amount

            if ( opcode == ADD || opcode == CMN )   // CMN is just like an ADD
                begin
                alu_out_sel_nxt  = 4'd1; // Add
                end

            if ( opcode == ADC ) // Add with Carry
                begin
                alu_out_sel_nxt  = 4'd1; // Add
                alu_cin_sel_nxt  = 2'd2; // carry in from status_bits
                end

            if ( opcode == SUB || opcode == CMP ) // Subtract
                begin
                alu_out_sel_nxt  = 4'd1; // Add
                alu_cin_sel_nxt  = 2'd1; // cin = 1
                alu_not_sel_nxt  = 1'd1; // invert B
                end

            // SBC (Subtract with Carry) subtracts the value of its
            // second operand and the value of NOT(Carry flag) from
            // the value of its first operand.
            //  Rd = Rn - shifter_operand - NOT(C Flag)
            if ( opcode == SBC ) // Subtract with Carry
                begin
                alu_out_sel_nxt  = 4'd1; // Add
                alu_cin_sel_nxt  = 2'd2; // carry in from status_bits
                alu_not_sel_nxt  = 1'd1; // invert B
                end

            if ( opcode == RSB ) // Reverse Subtract
                begin
                alu_out_sel_nxt  = 4'd1; // Add
                alu_cin_sel_nxt  = 2'd1; // cin = 1
                alu_not_sel_nxt  = 1'd1; // invert B
                alu_swap_sel_nxt = 1'd1; // swap A and B
                end

            if ( opcode == RSC ) // Reverse Subtract with carry
                begin
                alu_out_sel_nxt  = 4'd1; // Add
                alu_cin_sel_nxt  = 2'd2; // carry in from status_bits
                alu_not_sel_nxt  = 1'd1; // invert B
                alu_swap_sel_nxt = 1'd1; // swap A and B
                end

            if ( opcode == AND || opcode == TST ) // Logical AND, Test  (using AND operator)
                begin
                alu_out_sel_nxt  = 4'd8;  // AND
                alu_cout_sel_nxt = 1'd1;  // i_barrel_shift_carry
                end

            if ( opcode == EOR || opcode == TEQ ) // Logical Exclusive OR, Test Equivalence (using EOR operator)
                begin
                alu_out_sel_nxt = 4'd6;  // XOR
                alu_cout_sel_nxt = 1'd1; // i_barrel_shift_carry
                end

            if ( opcode == ORR )
                begin
                alu_out_sel_nxt  = 4'd7; // OR
                alu_cout_sel_nxt = 1'd1;  // i_barrel_shift_carry
                end

            if ( opcode == BIC ) // Bit Clear (using AND & NOT operators)
                begin
                alu_out_sel_nxt  = 4'd8;  // AND
                alu_not_sel_nxt  = 1'd1;  // invert B
                alu_cout_sel_nxt = 1'd1;  // i_barrel_shift_carry
                end

            if ( opcode == MOV ) // Move
                begin
                alu_cout_sel_nxt = 1'd1;  // i_barrel_shift_carry
                end

            if ( opcode == MVN ) // Move NOT
                begin
                alu_not_sel_nxt  = 1'd1; // invert B
                alu_cout_sel_nxt = 1'd1;  // i_barrel_shift_carry
                end
            end

        if ( control_state == LUA_EXEC)
            begin
            pc_wen_nxt = 1'd0; // hold current PC value
            if ( itype == LUA_MOVE)
                begin
                // store in R(A)
                write_data_wen_nxt = 1'd1; // write out
                // write_data_update_wen_nxt = 1'd0;
                address_sel_nxt = 4'd1; // alu out

                alu_out_sel_nxt  = 4'd1; // Add

                barrel_shift_data_sel_nxt = 2'd0;
                barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                lua_store_a = 1'd1;

                lua_rn_reg = BASE;
                lua_rd_reg = 'd1; // src of move is in r1
                end

            if ( itype == LUA_ALU2 )
                begin

                if ( i_lua_inst_stage == 'h1 )
                    begin
                    // deref r1 to itself, to get its real value
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_fetch_1 = 1'd1;

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount

                    lua_rn_reg = 'd1;
                    lua_inst_stage_reg = (opcode == LUA_OP_NOT) || (opcode == LUA_OP_BNOT) || (opcode == LUA_OP_UNM) ? 'h4 : 'h2;
                    end

                if ( i_lua_inst_stage == 'h2 )
                    begin
                    // fetch R(C) or RK(C)
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_fetch_2 = 1'd1;
                    lua_custom_imm32 = {24'd0, lua_ck_sel_nxt};

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_function_nxt   = 0; // LSL
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                    lua_rn_reg = lua_ck_base;
                    lua_inst_stage_reg = 'h3;
                    end

                if ( i_lua_inst_stage == 'h3 )
                    begin
                    // deref r2 to itself, to get its real value
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_fetch_2 = 1'd1;

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount

                    lua_rn_reg = 'd2;
                    lua_inst_stage_reg = 'h4;
                    end

                if ( i_lua_inst_stage == 'h4 )
                    begin
                    lua_rn_reg = 'd1;
                    lua_rm_reg = 'd2;
                    barrel_shift_data_sel_nxt = 2'd2;

                    // results in r0
                    reg_bank_wen_nxt  = decode (4'd0);
                    reg_write_sel_nxt = 4'd0; // alu out

                    if ( opcode == LUA_OP_ADD )
                        begin
                        alu_out_sel_nxt  = 4'd1; // Add
                        end

                    if ( opcode == LUA_OP_SUB )
                        begin
                        alu_out_sel_nxt  = 4'd1; // Add
                        alu_cin_sel_nxt  = 2'd1; // cin = 1
                        alu_not_sel_nxt  = 1'd1; // invert B
                        end

                    if ( opcode == LUA_OP_BAND )
                        begin
                        alu_out_sel_nxt  = 4'd8;  // AND
                        alu_cout_sel_nxt = 1'd1;  // i_barrel_shift_carry
                        end

                    if ( opcode == LUA_OP_BOR )
                        begin
                        alu_out_sel_nxt  = 4'd7; // OR
                        alu_cout_sel_nxt = 1'd1;  // i_barrel_shift_carry
                        end

                    if ( opcode == LUA_OP_BXOR )
                        begin
                        alu_out_sel_nxt = 4'd6;  // XOR
                        alu_cout_sel_nxt = 1'd1; // i_barrel_shift_carry
                        end

                    if ( opcode == LUA_OP_SHL )
                        begin
                        // swap registers for the shift
                        lua_rm_reg = 'd1;
                        lua_rd_reg = 'd2;
                        barrel_shift_amount_sel_nxt = 'd1;
                        end

                    if ( opcode == LUA_OP_SHR )
                        begin
                        lua_rm_reg = 'd1;
                        lua_rd_reg = 'd2;
                        barrel_shift_amount_sel_nxt = 'd1;
                        barrel_shift_function_nxt = LSR;
                        end

                    if ( opcode == LUA_OP_UNM )
                        begin
                        barrel_shift_data_sel_nxt = 2'd0;
                        alu_out_sel_nxt  = 4'd1; // Add
                        alu_swap_sel_nxt = 1'd1; // swap A and B
                        alu_not_sel_nxt  = 1'd1; // invert B
                        end

                    if ( opcode == LUA_OP_BNOT )
                        begin
                        alu_swap_sel_nxt = 1'd1; // swap A and B
                        alu_not_sel_nxt  = 1'd1; // invert B
                        end

                    if ( opcode == LUA_OP_NOT )
                        begin
                        alu_out_sel_nxt  = 4'd1;
                        reg_write_sel_nxt = 4'd8;
                        alu_swap_sel_nxt = 1'd1; // swap A and B
                        end

                    lua_inst_stage_reg = 'h5;
                    end

                if ( i_lua_inst_stage == 'h5 )
                    begin
                    // move scratch to r1
                    lua_rm_reg = SCRATCH;
                    barrel_shift_data_sel_nxt = 2'd2;
                    reg_bank_wen_nxt  = decode ('d1);
                    lua_inst_stage_reg = 'h6;
                    end
                if ( i_lua_inst_stage == 'h6 )
                    begin
                    // bump scratch register
                    lua_rn_reg = 'd1;
                    reg_bank_wen_nxt  = decode (SCRATCH);
                    reg_write_sel_nxt = 4'd0; // alu_out
                    alu_out_sel_nxt  = 4'd1; // Add

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                    lua_custom_imm32 = 32'd1;

                    lua_inst_stage_reg = 'h7;
                    end

                if ( i_lua_inst_stage == 'h7 )
                    begin
                    // write out r1 to R(A)
                    data_access_exec_nxt = 1'd1;
                    lua_rd_reg = 'd1;
                    write_data_wen_nxt = 1'd1; // write out
                    address_sel_nxt = 4'd1; // alu out

                    alu_out_sel_nxt  = 4'd1; // Add

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                    lua_custom_imm32 = offseta;

                    lua_store_a = 1'd1;

                    lua_rn_reg = BASE;
                    lua_inst_stage_reg = 'h8;
                    end
                if ( i_lua_inst_stage == 'h8 )
                    begin
                    // write out result itself to r1
                    lua_rd_reg = 'd0;
                    data_access_exec_nxt = 1'd1;
                    write_data_wen_nxt = 1'd1; // write out
                    address_sel_nxt = 4'd4; // rn

                    alu_out_sel_nxt  = 4'd1; // Add

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount

                    lua_store_a = 1'd1;

                    lua_rn_reg = 'd1;
                    lua_inst_stage_reg = 'h0;
                    end
                end

            if ( itype == LUA_TEST )
            begin
                if ( i_lua_inst_stage == 'h1 )
                    begin
                    // deref r1 to itself to get the underlying value
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_fetch_1 = 1'd1;

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount

                    lua_rn_reg = 'd1;
                    lua_inst_stage_reg = 'h2;
                    end
                if ( i_lua_inst_stage == 'h2 )
                    begin
                    // r1 has our value, which we now xor to determine if 1 or 0
                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                    lua_custom_imm32 = 'd0;

                    alu_out_sel_nxt  = 4'd1; // add
                    status_bits_flags_wen_nxt     = 1'd1;

                    pc_wen_nxt = 1'd0; // hold pc
                    address_sel_nxt = 4'd3;

                    lua_rn_reg = 'd1;
                    lua_inst_stage_reg = 'h3;
                    end
                if ( i_lua_inst_stage == 'h3 )
                    begin
                    // stall to allow status flags to reach decode stage
                    pc_wen_nxt = 1'd0; // hold pc
                    address_sel_nxt = 4'd3;
                    lua_inst_stage_reg = 'h4;
                    end
                if ( i_lua_inst_stage == 'h4 )
                    begin
                    // check zero flag
                    pc_wen_nxt = i_execute_status_bits[30:30] == lua_c_sel_nxt[0:0] ? 1'd1 : 1'd0;

                    // if we don't update pc, and we aren't testset, we are done
                    lua_prefetch_1 = !pc_wen_nxt && (opcode != LUA_OP_TESTSET);

                    // need to stall more to fetch pc, if we matched
                    lua_inst_stage_reg = pc_wen_nxt ? 'h5 : ((opcode == LUA_OP_TESTSET) ? 'h6 : 'h0);
                    end
                if ( i_lua_inst_stage == 'h5 )
                    begin
                        pc_wen_nxt = 1'd0; // hold pc
                        address_sel_nxt = 4'd3;
                        pre_fetch_instruction_wen = 1'd1;
                        lua_prefetch_1 = 'd1;
                        lua_inst_stage_reg = 'h0;
                    end 
                if ( i_lua_inst_stage == 'h6 )
                    begin
                        // read R(B)
                        data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                        address_sel_nxt = 4'd1; // alu out
                        alu_out_sel_nxt = 4'd1; // Add
                        lua_fetch_1 = 1'd1;
                        lua_custom_imm32 = {23'd0, lua_b_sel_nxt};

                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_function_nxt   = 0; // LSL
                        barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                        lua_rn_reg = BASE;
                        lua_inst_stage_reg = 'h7;
                    end
                if ( i_lua_inst_stage == 'h7 )
                    begin
                        // store in R(A)
                        lua_rd_reg = 'd1;
                        data_access_exec_nxt = 1'd1;
                        write_data_wen_nxt = 1'd1; // write out
                        address_sel_nxt = 4'd1; // alu out

                        alu_out_sel_nxt  = 4'd1; // Add

                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                        lua_store_a = 1'd1;

                        lua_rn_reg = BASE;
                        lua_inst_stage_reg = 'h0;
                    end
            end
            
            if ( itype == LUA_LOADBOOL )
            begin
                if ( i_lua_inst_stage == 'h1 )
                begin
                    // increment pc if C>0
                    pc_wen_nxt = (lua_c_sel_nxt == 'h0) ? 1'd0 : 1'd1;
                    lua_inst_stage_reg = pc_wen_nxt ? 'h2 : 'h4;
                end
                if ( i_lua_inst_stage == 'h2 )
                begin
                    // stall waiting for new instruction to reach decode...
                    lua_inst_stage_reg = 'h3;
                end
                if ( i_lua_inst_stage == 'h3 )
                begin
                    // prefetch our new pc
                    pre_fetch_instruction_wen = 1'd1; // prefetch whatever the new pc is
                    lua_inst_stage_reg = 'h4;
                end
                if ( i_lua_inst_stage == 'h4 )
                    begin
                    // move scratch to r1
                    lua_rm_reg = SCRATCH;
                    barrel_shift_data_sel_nxt = 2'd2;
                    reg_bank_wen_nxt  = decode ('d1);
                    lua_inst_stage_reg = 'h5;
                    end
                if ( i_lua_inst_stage == 'h5 )
                    begin
                    // bump scratch register
                    lua_rn_reg = 'd1;
                    reg_bank_wen_nxt  = decode (SCRATCH);
                    reg_write_sel_nxt = 4'd0; // alu_out
                    alu_out_sel_nxt  = 4'd1; // Add

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                    lua_custom_imm32 = 32'd1;

                    lua_inst_stage_reg = 'h6;
                    end
                if ( i_lua_inst_stage == 'h6 )
                    begin
                    // write out r1 to R(A)
                    data_access_exec_nxt = 1'd1;
                    lua_rd_reg = 'd1;
                    write_data_wen_nxt = 1'd1; // write out
                    address_sel_nxt = 4'd1; // alu out

                    alu_out_sel_nxt  = 4'd1; // Add

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                    lua_custom_imm32 = offseta;

                    lua_store_a = 1'd1;

                    lua_rn_reg = BASE;
                    lua_inst_stage_reg = 'h7;
                    end
                if ( i_lua_inst_stage == 'h7 )
                    begin
                    // write 1 or 0 to r0 based on B
                    reg_bank_wen_nxt  = decode ('d0);
                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount
                    lua_custom_imm32 = {23'd0, lua_b_sel_nxt};
                    reg_write_sel_nxt = 4'ha; // direct barrel shift output
                    lua_inst_stage_reg = 'h8;
                    end
                if ( i_lua_inst_stage == 'h8 )
                    begin
                    // write out result itself to r1
                    lua_rd_reg = 'd0;
                    data_access_exec_nxt = 1'd1;
                    write_data_wen_nxt = 1'd1; // write out
                    address_sel_nxt = 4'd4; // rn

                    alu_out_sel_nxt  = 4'd1; // Add

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount

                    lua_store_a = 1'd1;

                    lua_rn_reg = 'd1;
                    lua_inst_stage_reg = 'h0;
                    end
            end

            if ( itype == LUA_LOADNIL )
            begin
                if ( i_lua_inst_stage == 'h1 )
                    begin
                    // calculate &R(A), the start of our nil-ing. store in r0
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_custom_imm32 = offseta;

                    reg_bank_wen_nxt  = decode ('d0);
                    reg_write_sel_nxt = 4'd0; // alu_out

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_function_nxt   = 0; // LSL
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                    lua_rn_reg = BASE;
                    lua_inst_stage_reg = 'h2;
                    end

                if ( i_lua_inst_stage == 'h2 )
                    begin
                    // write our DMA engine address to r5
                    reg_bank_wen_nxt  = decode ('d5);
                    lua_custom_imm32 = 32'hC0000000; // dma engine base
                    reg_write_sel_nxt = 4'ha; // direct barrel shift output
                    lua_inst_stage_reg = 'h3;
                    end

                if ( i_lua_inst_stage == 'h3 )
                    begin
                    // store r0 in dma engine src
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    lua_rd_reg = 'd0;
                    write_data_wen_nxt = 1'd1; // write out
                    address_sel_nxt = 4'd1; // alu out

                    alu_out_sel_nxt  = 4'd1; // Add
                    lua_store_a = 1'd1;

                    lua_rn_reg = 'd5;
                    lua_inst_stage_reg = 'h4;
                    end
                if ( i_lua_inst_stage == 'h4 )
                    begin
                    // synthesize zero and store in r2
                    reg_bank_wen_nxt  = decode ('d2);
                    lua_rm_reg = 'd1;
                    barrel_shift_data_sel_nxt = 2'd2;

                    alu_out_sel_nxt  = 4'd6; // xor

                    lua_rn_reg = 'd1;
                    lua_inst_stage_reg = 'h5;
                    end
                if ( i_lua_inst_stage == 'h5 )
                begin
                    // store 0 in scratch
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    lua_rd_reg = 'd2;
                    write_data_wen_nxt = 1'd1; // write out
                    address_sel_nxt = 4'd1; // alu out

                    alu_out_sel_nxt  = 4'd1; // Add
                    lua_store_a = 1'd1;

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_function_nxt   = 0; // LSL
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                    lua_rn_reg = SCRATCH;
                    lua_inst_stage_reg = 'h6;
                end
                if ( i_lua_inst_stage == 'h6 )
                begin
                    // move scratch to r1
                    lua_rm_reg = SCRATCH;
                    barrel_shift_data_sel_nxt = 2'd2;
                    reg_bank_wen_nxt  = decode ('d1);
                    lua_inst_stage_reg = 'h7;
                end
                if ( i_lua_inst_stage == 'h7 )
                    begin
                    // bump scratch register
                    lua_rn_reg = SCRATCH;
                    reg_bank_wen_nxt  = decode (SCRATCH);
                    reg_write_sel_nxt = 4'd9; // alu_out_plus4
                    alu_out_sel_nxt  = 4'd1; // Add

                    lua_inst_stage_reg = 'h8;
                    end
                if ( i_lua_inst_stage == 'h8 )
                    begin
                    // store r1, the nil we just created in our first slot to nil
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    lua_rd_reg = 'd1;
                    write_data_wen_nxt = 1'd1; // write out
                    address_sel_nxt = 4'd1; // alu out

                    alu_out_sel_nxt  = 4'd1; // Add
                    lua_store_a = 1'd1;

                    lua_rn_reg = 'd0;
                    lua_inst_stage_reg = 'h9;
                    end
                if ( i_lua_inst_stage == 'h9 )
                    begin
                    // store r0 in dma engine dest
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    lua_rd_reg = 'd0;
                    write_data_wen_nxt = 1'd1; // write out
                    address_sel_nxt = 4'd1; // alu out

                    alu_out_sel_nxt  = 4'd1; // Add
                    lua_store_a = 1'd1;

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_function_nxt   = 0; // LSL
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                    lua_custom_imm32 = 32'd1; // offset (*4)

                    lua_rn_reg = 'd5;
                    lua_inst_stage_reg = 'ha;
                    end
                if ( i_lua_inst_stage == 'ha )
                    begin
                    // read B into r0
                    data_access_exec_nxt = 1'd1;
                    reg_bank_wen_nxt  = decode ('d0);
                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount
                    lua_custom_imm32 = {23'd0, lua_b_sel_nxt};
                    reg_write_sel_nxt = 4'ha; // direct barrel shift output
                    lua_inst_stage_reg = 'hb;
                    end
                if ( i_lua_inst_stage == 'hb )
                    begin
                    // store r0 in dma engine count
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    lua_rd_reg = 'd0;
                    write_data_wen_nxt = 1'd1; // write out
                    address_sel_nxt = 4'd1; // alu out

                    alu_out_sel_nxt  = 4'd1; // Add
                    lua_store_a = 1'd1;

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_function_nxt   = 0; // LSL
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                    lua_custom_imm32 = 32'd2; // offset (*4)

                    lua_rn_reg = 'd5;
                    lua_inst_stage_reg = 'hc;
                    end
                if ( i_lua_inst_stage == 'hc )
                    begin
                    // write our DMA engine config to r0
                    data_access_exec_nxt = 1'd1;
                    reg_bank_wen_nxt  = decode ('d0);
                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount
                    lua_custom_imm32 = {
                        16'd0, // padding
                        8'd1, // size: 4
                        4'd1, // op = repeated move
                        2'd0, // padding
                        1'd0, // direction 0, copy src -> dst
                        1'd1 // enable bit
                    };
                    reg_write_sel_nxt = 4'ha; // direct barrel shift output
                    lua_inst_stage_reg = 'hd;
                    end
                if ( i_lua_inst_stage == 'hd )
                    begin
                    // store config in the cfg field of the DMA engine
                    // this also drives the engine, since the enable bit is set
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    lua_rd_reg = 'd0;
                    write_data_wen_nxt = 1'd1; // write out
                    address_sel_nxt = 4'd1; // alu out

                    alu_out_sel_nxt  = 4'd1; // Add
                    lua_store_a = 1'd1;

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_function_nxt   = 0; // LSL
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                    lua_custom_imm32 = 32'd3; // offset (*4)

                    lua_rn_reg = 'd5;
                    lua_inst_stage_reg = 'he;
                    end
                if ( i_lua_inst_stage == 'he )
                    begin
                    // worthless, useless, totally skippable store
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    lua_rd_reg = 'd1;
                    write_data_wen_nxt = 1'd1; // write out
                    address_sel_nxt = 4'd1; // alu out

                    alu_out_sel_nxt  = 4'd6; // xor with itself, to get 0

                    lua_rm_reg = 'd1;
                    barrel_shift_data_sel_nxt = 2'd2;
                    lua_store_a = 1'd1;

                    lua_rn_reg = BASE;
                    lua_inst_stage_reg = 'h0;
                    end
            end
            if ( itype == LUA_SELF )
            begin
                if ( i_lua_inst_stage == 'h1 )
                    begin
                    // store R(B), which we read previously, into R(A+1)
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    lua_rd_reg = 'd1;
                    write_data_wen_nxt = 1'd1; // write out
                    address_sel_nxt = 4'd6; // alu out plus4

                    alu_out_sel_nxt  = 4'd1; // Add

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd2;
                    lua_store_a = 1'd1;

                    lua_rn_reg = BASE;
                    lua_inst_stage_reg = 'h2;
                    end
                if ( i_lua_inst_stage == 'h2 )
                    begin
                    // load RK(C)
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_fetch_2 = 1'd1;
                    lua_custom_imm32 = {24'd0, lua_ck_sel_nxt};

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_function_nxt   = 0; // LSL
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                    lua_rn_reg = lua_ck_base;
                    lua_inst_stage_reg = 'h3;
                    end
                 if ( i_lua_inst_stage == 'h3)
                    begin
                    // load table from R(B)
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_fetch_1 = 1'd1;
                    lua_custom_imm32 = {23'd0, lua_b_sel_nxt};

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_function_nxt   = 0; // LSL
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                    lua_rn_reg = BASE;
                    lua_inst_stage_reg = 'h4;
                    end
                if ( i_lua_inst_stage == 'h4 )
                    begin
                    // deref r1 to itself to get raw base of table
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_fetch_1 = 1'd1;

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount

                    lua_rn_reg = 'd1;
                    lua_inst_stage_reg = 'h5;
                    end
                if ( i_lua_inst_stage == 'h5 )
                    begin
                    // deref r2 again in preparation for "hashing" it
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_fetch_2 = 1'd1;

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount

                    lua_rn_reg = 'd2;
                    lua_inst_stage_reg = 'h6;
                    end
                if ( i_lua_inst_stage == 'h6 )
                    begin
                    // shift and mask r2 to get the hash
                    // hash(x) = (x & 0x7fff);

                    lua_rn_reg = 'd2;

                    lua_custom_imm32 = 32'h7fff;

                    reg_bank_wen_nxt  = decode ('d2);
                    reg_write_sel_nxt = 4'd0; // alu_out

                    alu_out_sel_nxt  = 4'd9; // and
                    lua_inst_stage_reg = 'h7;
                    end
                if ( i_lua_inst_stage == 'h7 )
                    begin
                    // read from r1 (table) + (r2 << 2) + 4, into r1
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd6; // alu out plus4
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_fetch_1 = 1'd1;

                    lua_rm_reg = 'd2;
                    barrel_shift_data_sel_nxt = 2'd2;
                    barrel_shift_function_nxt   = 0; // LSL
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                    lua_rn_reg = 'd1;
                    lua_inst_stage_reg = 'h8;
                    end
                if ( i_lua_inst_stage == 'h8 )
                    begin
                    // store r1 in R(A)
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    lua_rd_reg = 'd1;
                    write_data_wen_nxt = 1'd1; // write out
                    address_sel_nxt = 4'd1; // alu out

                    alu_out_sel_nxt  = 4'd1; // Add

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd2;
                    lua_store_a = 1'd1;

                    lua_rn_reg = BASE;
                    lua_inst_stage_reg = 'd0;
                    end
            end

            if ( itype == LUA_CMP )
                begin
                if ( i_lua_inst_stage == 'h1 )
                    begin
                    // fetch RK(C)
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_fetch_2 = 1'd1;
                    lua_custom_imm32 = {24'd0, lua_ck_sel_nxt};

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_function_nxt   = 0; // LSL
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                    lua_rn_reg = lua_ck_base;
                    lua_inst_stage_reg = 'h2;
                    end
                if ( i_lua_inst_stage == 'h2 )
                    begin
                    // deref r1 to get the value or hash
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_fetch_1 = 1'd1;

                    lua_rn_reg = 'd1;
                    lua_inst_stage_reg = 'h3;
                end
                if ( i_lua_inst_stage == 'h3 )
                    begin
                    // deref r2 to get the value or hash
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_fetch_2 = 1'd1;

                    lua_rn_reg = 'd2;
                    lua_inst_stage_reg = 'h4;
                end
                if ( i_lua_inst_stage == 'h4 )
                    begin
                    // r1 has RK(B) and r2 has RK(C), so we can now compare them via subtraction
                    lua_rm_reg = 'd2;

                    barrel_shift_data_sel_nxt = 2'd2;

                    alu_out_sel_nxt  = 4'd1; // Add
                    alu_cin_sel_nxt  = 2'd1; // cin = 1
                    alu_not_sel_nxt  = 1'd1; // invert B
                    status_bits_flags_wen_nxt     = 1'd1;

                    lua_rn_reg = 'd1;
                    lua_inst_stage_reg = 'h5;
                    end
                if ( i_lua_inst_stage == 'h5 )
                    begin
                    // stall to allow status flags to reach decode stage
                    lua_inst_stage_reg = 'h6;
                    end
                if ( i_lua_inst_stage == 'h6 )
                    begin

                        lua_inst_stage_reg = 'h8;
                        if (
                                (opcode == LUA_OP_LT && (i_execute_status_bits[31:31] != offseta[0:0])) ||
                                (opcode == LUA_OP_EQ && (i_execute_status_bits[30:30] != offseta[0:0])) ||
                                (opcode == LUA_OP_LE && (|i_execute_status_bits[31:30] != offseta[0:0]))
                            )
                        begin
                            // add 4 to pc
                            pre_fetch_instruction_wen = 'd1;
                            lua_inst_stage_reg = 'h9;
                        end

                    end
                if ( i_lua_inst_stage == 'h8 )
                    begin
                        lua_store_a = 1'd1;
                        lua_inst_stage_reg = 'd0;
                    end 
                 if ( i_lua_inst_stage == 'h9 )
                    begin
                    lua_inst_stage_reg = 'ha;
                    end
                if ( i_lua_inst_stage == 'ha )
                    begin
                    pc_wen_nxt = 'd1;
                    lua_inst_stage_reg = 'hb;
                    end
                if ( i_lua_inst_stage == 'hb )
                    begin
                    // stall...
                    lua_prefetch_1 = 1'd1;
                    lua_inst_stage_reg = 'h0;
                    end
                end

            if ( itype == LUA_BRANCH )
            begin
                if ( i_lua_inst_stage == 'h1 )
                    begin
                    // zero out r1 in preparation for calculations
                    alu_out_sel_nxt = 4'd6;

                    barrel_shift_data_sel_nxt = 2'd2;
                    reg_bank_wen_nxt = decode('d1);
                    lua_rm_reg = 'd0;
                    lua_rn_reg = 'd0;
                    lua_inst_stage_reg = 'h2;
                    end
                if ( i_lua_inst_stage == 'h2 )
                    begin
                    // add (bx << 2) to r1, which we just zerod
                    lua_rn_reg = 'd1;
                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                    lua_custom_imm32 = {14'd0, lua_bx_sel_nxt}; // 4 bytes for table hash, 0x20000 bytes for the table itself

                    reg_bank_wen_nxt  = decode ('d1);
                    reg_write_sel_nxt = 4'd0; // alu_out

                    alu_out_sel_nxt  = 4'd1; // Add
                    lua_inst_stage_reg = 'h3;
                    end
                if ( i_lua_inst_stage == 'h3 )
                    begin
                    // subtract maxBx from r1
                    lua_rn_reg = 'd1;
                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd2;
                    lua_custom_imm32 = 32'h1ffff; // see lopcodes.h to calculate this

                    reg_bank_wen_nxt  = decode ('d1);
                    reg_write_sel_nxt = 4'd0; // alu_out

                    alu_out_sel_nxt  = 4'd1; // Add
                    alu_cin_sel_nxt  = 2'd1; // cin = 1
                    alu_not_sel_nxt  = 1'd1; // invert B

                    lua_inst_stage_reg = 'h4;
                    end
                if ( i_lua_inst_stage == 'h4 )
                    begin
                    // add r1 to saved instruction address
                    lua_rn_reg = 'd1;
                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount
                    lua_custom_imm32 = saved_current_instruction_address;

                    alu_out_sel_nxt  = 4'd1; // Add
                    pc_wen_nxt = 1'd1;
                    pc_sel_nxt = 3'd1; // alu_out
                    lua_inst_stage_reg = 'h5;
                    end
                if ( i_lua_inst_stage == 'h5 )
                    begin
                    // stall...
                    lua_inst_stage_reg = 'h6;
                    end
                if ( i_lua_inst_stage == 'h6 )
                    begin
                    // stall...
                    pc_wen_nxt = 'd1;
                    lua_inst_stage_reg = 'h7;
                    end
                if ( i_lua_inst_stage == 'h7 )
                    begin
                    // stall...
                    pc_wen_nxt = 'd1;
                    lua_inst_stage_reg = 'h8;
                    end
                if ( i_lua_inst_stage == 'h8 )
                    begin
                    // stall...
                    pc_wen_nxt = 'd1;
                    pre_fetch_instruction_wen = 'd1;
                    lua_prefetch_1 = 1'd1;
                    lua_inst_stage_reg = 'h0;
                    end
            end

            if ( itype == LUA_FORPREP )
            begin
                 if ( i_lua_inst_stage == 'h1 )
                    begin
                    // calculate &R(A), which is the start of our psuedovariables
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_custom_imm32 = offseta;

                    reg_bank_wen_nxt  = decode ('d0);
                    reg_write_sel_nxt = 4'd0; // alu_out

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_function_nxt   = 0; // LSL
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                    lua_rn_reg = BASE;
                    lua_inst_stage_reg = 'h2;
                    end
                if ( i_lua_inst_stage == 'h2 )
                begin
                    // fetch loop step - R(A+2)
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_fetch_2 = 1'd1;

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount
                    lua_custom_imm32 = 'd8;

                    lua_rn_reg = 'd0;
                    lua_inst_stage_reg = 'h3;
                end
                if ( i_lua_inst_stage == 'h3 )
                    begin
                    // get underlying value of step
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_fetch_2 = 1'd1;

                    lua_rn_reg = 'd2;
                    lua_inst_stage_reg = 'h4;
                    end
                if ( i_lua_inst_stage == 'h4 )
                    begin
                    // move to r3
                    lua_rm_reg = 'd2;
                    barrel_shift_data_sel_nxt = 2'd2;
                    reg_bank_wen_nxt  = decode ('d3);
                    lua_inst_stage_reg = 'h5;
                    end
                if ( i_lua_inst_stage == 'h5 )
                    begin
                    // get underlying value of initial value
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_fetch_2 = 1'd1;

                    lua_rn_reg = 'd1;
                    lua_inst_stage_reg = 'h6;
                    end
                if ( i_lua_inst_stage == 'h6 )
                    begin
                    // subtract step from initial value
                    lua_rn_reg = 'd2;
                    lua_rm_reg = 'd3;
                    barrel_shift_data_sel_nxt = 2'd2;

                    reg_bank_wen_nxt  = decode ('d0);
                    reg_write_sel_nxt = 4'd0; // alu_out

                    alu_out_sel_nxt  = 4'd1; // Add
                    alu_cin_sel_nxt  = 2'd1; // cin = 1
                    alu_not_sel_nxt  = 1'd1; // invert B

                    lua_inst_stage_reg = 'h7;
                end
                if ( i_lua_inst_stage == 'h7 )
                    begin
                    // move scratch to r1
                    lua_rm_reg = SCRATCH;
                    barrel_shift_data_sel_nxt = 2'd2;
                    reg_bank_wen_nxt  = decode ('d1);
                    lua_inst_stage_reg = 'h8;
                    end
                if ( i_lua_inst_stage == 'h8 )
                    begin
                    // bump scratch register
                    lua_rn_reg = 'd1;
                    reg_bank_wen_nxt  = decode (SCRATCH);
                    reg_write_sel_nxt = 4'd0; // alu_out
                    alu_out_sel_nxt  = 4'd1; // Add

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                    lua_custom_imm32 = 32'd1;

                    lua_inst_stage_reg = 'h9;
                    end

                if ( i_lua_inst_stage == 'h9 )
                    begin
                    // write out r1 to R(A)
                    data_access_exec_nxt = 1'd1;
                    lua_rd_reg = 'd1;
                    write_data_wen_nxt = 1'd1; // write out
                    address_sel_nxt = 4'd1; // alu out

                    alu_out_sel_nxt  = 4'd1; // Add

                    lua_custom_imm32 = offseta;
                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_function_nxt   = 0; // LSL
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                    lua_store_a = 1'd1;

                    lua_rn_reg = BASE;
                    lua_inst_stage_reg = 'ha;
                    end
                if ( i_lua_inst_stage == 'ha )
                    begin
                    // write out result itself to r1
                    lua_rd_reg = 'd0;
                    data_access_exec_nxt = 1'd1;
                    write_data_wen_nxt = 1'd1; // write out
                    address_sel_nxt = 4'd4; // rn

                    alu_out_sel_nxt  = 4'd1; // Add

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount

                    lua_store_a = 1'd1;

                    lua_rn_reg = 'd1;
                    lua_inst_stage_reg = 'hb;
                    end

                // calculate sBx offset jump
                 if ( i_lua_inst_stage == 'hb )
                    begin
                    // zero out r4 in preparation for calculations
                    alu_out_sel_nxt = 4'd6;

                    barrel_shift_data_sel_nxt = 2'd2;
                    reg_bank_wen_nxt = decode('d4);
                    lua_rm_reg = 'd4;
                    lua_rn_reg = 'd4;
                    lua_inst_stage_reg = 'hc;
                    end
                if ( i_lua_inst_stage == 'hc )
                    begin
                    // add (bx << 2) to r4, which we just zerod
                    lua_rn_reg = 'd4;
                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                    lua_custom_imm32 = {14'd0, lua_bx_sel_nxt}; // 4 bytes for table hash, 0x20000 bytes for the table itself

                    reg_bank_wen_nxt  = decode ('d4);
                    reg_write_sel_nxt = 4'd0; // alu_out

                    alu_out_sel_nxt  = 4'd1; // Add
                    lua_inst_stage_reg = 'hd;
                    end
                if ( i_lua_inst_stage == 'hd )
                    begin
                    // subtract maxBx from r4
                    lua_rn_reg = 'd4;
                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd2;
                    lua_custom_imm32 = 32'h1ffff; // see lopcodes.h to calculate this

                    reg_bank_wen_nxt  = decode ('d4);
                    reg_write_sel_nxt = 4'd0; // alu_out

                    alu_out_sel_nxt  = 4'd1; // Add
                    alu_cin_sel_nxt  = 2'd1; // cin = 1
                    alu_not_sel_nxt  = 1'd1; // invert B

                    lua_inst_stage_reg = 'he;
                    end
                if ( i_lua_inst_stage == 'he )
                    begin
                    // add r4 to saved instruction address
                    lua_rn_reg = 'd4;
                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount
                    lua_custom_imm32 = saved_current_instruction_address;

                    alu_out_sel_nxt  = 4'd1; // Add
                    pc_wen_nxt = 1'd1;
                    pc_sel_nxt = 3'd1; // alu_out
                    lua_inst_stage_reg = 'hf;
                    end
                if ( i_lua_inst_stage == 'hf )
                    begin
                    // stall...
                    lua_inst_stage_reg = 'h10;
                    end
                if ( i_lua_inst_stage == 'h10 )
                    begin
                    // stall...
                    pc_wen_nxt = 'd1;
                    lua_inst_stage_reg = 'h11;
                    end
                if ( i_lua_inst_stage == 'h11 )
                    begin
                    // stall...
                    pc_wen_nxt = 'd1;
                    lua_inst_stage_reg = 'h12;
                    end
                if ( i_lua_inst_stage == 'h12 )
                    begin
                    // stall...
                    pc_wen_nxt = 'd1;
                    pre_fetch_instruction_wen = 'd1;
                    lua_prefetch_1 = 1'd1;
                    lua_inst_stage_reg = 'h0;
                    end
            end

            if ( itype == LUA_FORLOOP )
            begin
                if ( i_lua_inst_stage == 'h1 )
                    begin
                    // calculate &R(A), which is the start of our psuedovariables
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_custom_imm32 = offseta;

                    reg_bank_wen_nxt  = decode ('d0);
                    reg_write_sel_nxt = 4'd0; // alu_out

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_function_nxt   = 0; // LSL
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                    lua_rn_reg = BASE;
                    lua_inst_stage_reg = 'h2;
                    end
                if ( i_lua_inst_stage == 'h2 )
                begin
                    // fetch loop limit - R(A+1)
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_fetch_2 = 1'd1;

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount
                    lua_custom_imm32 = 'd4;

                    lua_rn_reg = 'd0;
                    lua_inst_stage_reg = 'h3;
                end
                if ( i_lua_inst_stage == 'h3 )
                    begin
                    // get underlying value of limit
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_fetch_2 = 1'd1;

                    lua_rn_reg = 'd2;
                    lua_inst_stage_reg = 'h4;
                    end
                if ( i_lua_inst_stage == 'h4 )
                    begin
                    // move to r3
                    lua_rm_reg = 'd2;
                    barrel_shift_data_sel_nxt = 2'd2;
                    reg_bank_wen_nxt  = decode ('d3);
                    lua_inst_stage_reg = 'h5;
                    end
                if ( i_lua_inst_stage == 'h5 )
                    begin
                    // get underlying value of index
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_fetch_2 = 1'd1;

                    lua_rn_reg = 'd1;
                    lua_inst_stage_reg = 'h6;
                    end
                if ( i_lua_inst_stage == 'h6 )
                    begin
                    // move to r5
                    lua_rm_reg = 'd2;
                    barrel_shift_data_sel_nxt = 2'd2;
                    reg_bank_wen_nxt  = decode ('d5);
                    lua_inst_stage_reg = 'h7;
                    end
                if ( i_lua_inst_stage == 'h7 )
                begin
                    // fetch loop step - R(A+2)
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_fetch_2 = 1'd1;

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount
                    lua_custom_imm32 = 'd8;

                    lua_rn_reg = 'd0;
                    lua_inst_stage_reg = 'h8;
                end
                if ( i_lua_inst_stage == 'h8 )
                    begin
                    // get underlying value of step
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_fetch_2 = 1'd1;

                    lua_rn_reg = 'd2;
                    lua_inst_stage_reg = 'h9;
                    end
                if ( i_lua_inst_stage == 'h9 )
                    begin
                    // add step to index
                    lua_rn_reg = 'd5;
                    lua_rm_reg = 'd2;

                    barrel_shift_data_sel_nxt = 2'd2;
                    alu_out_sel_nxt  = 4'd1; // Add

                    reg_bank_wen_nxt  = decode ('d5);
                    reg_write_sel_nxt = 4'd0; // alu_out

                    lua_inst_stage_reg = 'ha;
                    end
                if ( i_lua_inst_stage == 'ha )
                    begin
                    // r2 has step, see if it is positive or not
                    barrel_shift_data_sel_nxt = 2'd0;
                    lua_custom_imm32 = 'd0;

                    alu_out_sel_nxt  = 4'd1; // Add
                    status_bits_flags_wen_nxt     = 1'd1;

                    lua_rn_reg = 'd2;
                    lua_inst_stage_reg = 'hb;
                    end
                if ( i_lua_inst_stage == 'hb )
                    begin
                    // stall to allow status flags to reach decode stage
                    lua_inst_stage_reg = 'hc;
                    end
                if ( i_lua_inst_stage == 'hc )
                    begin
                    // compare idx and limit
                    lua_rm_reg = 'd3; // limit

                    barrel_shift_data_sel_nxt = 2'd2;

                    alu_out_sel_nxt  = 4'd1; // Add
                    alu_cin_sel_nxt  = 2'd1; // cin = 1
                    alu_not_sel_nxt  = 1'd1; // invert B
                    status_bits_flags_wen_nxt     = 1'd1;

                    lua_rn_reg = 'd5; // idx

                    alu_swap_sel_nxt = !i_execute_status_bits[31:31]; // swap A and B

                    lua_inst_stage_reg = 'hd;
                    end
                if ( i_lua_inst_stage == 'hd )
                    begin
                    // stall to allow status flags to reach decode stage
                    lua_inst_stage_reg = 'he;
                    end
                if ( i_lua_inst_stage == 'he )
                    begin
                    lua_inst_stage_reg = i_execute_status_bits[31:31] ? 'hf : 'h10;
                    end
                if ( i_lua_inst_stage == 'hf )
                    begin
                        // done with the loop
                        lua_store_a = 1'd1;
                        lua_inst_stage_reg = 'd0;
                    end 
                if ( i_lua_inst_stage == 'h10 )
                begin
                    // update index directly with our new value (inplace)
                    lua_rd_reg = 'd5;
                    data_access_exec_nxt = 1'd1; //     indicate that its a data read or write,
                    write_data_wen_nxt = 1'd1; // write out
                    address_sel_nxt = 4'd1; // alu out

                    alu_out_sel_nxt  = 4'd1; // Add

                    lua_store_a = 1'd1;
                    lua_rn_reg = 'd1;
                    lua_inst_stage_reg = 'h11;
                end
                if ( i_lua_inst_stage == 'h11 )
                    begin
                    // move scratch to r1
                    lua_rm_reg = SCRATCH;
                    barrel_shift_data_sel_nxt = 2'd2;
                    reg_bank_wen_nxt  = decode ('d1);
                    lua_inst_stage_reg = 'h12;
                    end
                if ( i_lua_inst_stage == 'h12 )
                    begin
                    // bump scratch register
                    lua_rn_reg = 'd1;
                    reg_bank_wen_nxt  = decode (SCRATCH);
                    reg_write_sel_nxt = 4'd0; // alu_out
                    alu_out_sel_nxt  = 4'd1; // Add

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                    lua_custom_imm32 = 32'd1;

                    lua_inst_stage_reg = 'h13;
                    end

                if ( i_lua_inst_stage == 'h13 )
                    begin
                    // write out r1 to R(A+3)
                    data_access_exec_nxt = 1'd1;
                    lua_rd_reg = 'd1;
                    write_data_wen_nxt = 1'd1; // write out
                    address_sel_nxt = 4'd1; // alu out

                    alu_out_sel_nxt  = 4'd1; // Add

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                    lua_custom_imm32 = 'd3;

                    lua_store_a = 1'd1;

                    lua_rn_reg = 'd0;
                    lua_inst_stage_reg = 'h14;
                    end
                if ( i_lua_inst_stage == 'h14 )
                    begin
                    // write out result itself to r1
                    lua_rd_reg = 'd5;
                    data_access_exec_nxt = 1'd1;
                    write_data_wen_nxt = 1'd1; // write out
                    address_sel_nxt = 4'd4; // rn

                    alu_out_sel_nxt  = 4'd1; // Add

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount

                    lua_store_a = 1'd1;

                    lua_rn_reg = 'd1;
                    lua_inst_stage_reg = 'h15;
                    end
                // calculate sBx offset jump
                 if ( i_lua_inst_stage == 'h15 )
                    begin
                    // zero out r4 in preparation for calculations
                    alu_out_sel_nxt = 4'd6;

                    barrel_shift_data_sel_nxt = 2'd2;
                    reg_bank_wen_nxt = decode('d4);
                    lua_rm_reg = 'd4;
                    lua_rn_reg = 'd4;
                    lua_inst_stage_reg = 'h16;
                    end
                if ( i_lua_inst_stage == 'h16 )
                    begin
                    // add (bx << 2) to r4, which we just zerod
                    lua_rn_reg = 'd4;
                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                    lua_custom_imm32 = {14'd0, lua_bx_sel_nxt}; // 4 bytes for table hash, 0x20000 bytes for the table itself

                    reg_bank_wen_nxt  = decode ('d4);
                    reg_write_sel_nxt = 4'd0; // alu_out

                    alu_out_sel_nxt  = 4'd1; // Add
                    lua_inst_stage_reg = 'h17;
                    end
                if ( i_lua_inst_stage == 'h17 )
                    begin
                    // subtract maxBx from r4
                    lua_rn_reg = 'd4;
                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd2;
                    lua_custom_imm32 = 32'h1ffff; // see lopcodes.h to calculate this

                    reg_bank_wen_nxt  = decode ('d4);
                    reg_write_sel_nxt = 4'd0; // alu_out

                    alu_out_sel_nxt  = 4'd1; // Add
                    alu_cin_sel_nxt  = 2'd1; // cin = 1
                    alu_not_sel_nxt  = 1'd1; // invert B

                    lua_inst_stage_reg = 'h18;
                    end
                if ( i_lua_inst_stage == 'h18 )
                    begin
                    // add r4 to saved instruction address
                    lua_rn_reg = 'd4;
                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount
                    lua_custom_imm32 = saved_current_instruction_address;

                    alu_out_sel_nxt  = 4'd1; // Add
                    pc_wen_nxt = 1'd1;
                    pc_sel_nxt = 3'd1; // alu_out
                    lua_inst_stage_reg = 'h19;
                    end
                if ( i_lua_inst_stage == 'h19 )
                    begin
                    // stall...
                    lua_inst_stage_reg = 'h1a;
                    end
                if ( i_lua_inst_stage == 'h1a )
                    begin
                    // stall...
                    pc_wen_nxt = 'd1;
                    lua_inst_stage_reg = 'h1b;
                    end
                if ( i_lua_inst_stage == 'h1b )
                    begin
                    // stall...
                    pc_wen_nxt = 'd1;
                    lua_inst_stage_reg = 'h1c;
                    end
                if ( i_lua_inst_stage == 'h1c )
                    begin
                    // stall...
                    pc_wen_nxt = 'd1;
                    pre_fetch_instruction_wen = 'd1;
                    lua_inst_stage_reg = 'h0;
                    end
            end

            if ( itype == LUA_LEN )
            begin
                if ( i_lua_inst_stage == 'd1 )
                    begin

                    // we only support strings for length, so we know the length will be at +0x4 in the object
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_fetch_1 = 1'd1;

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                    // shift offset
                    lua_custom_imm32 = 32'd1;

                    lua_rn_reg = 'd1;

                    lua_inst_stage_reg = 'h2;
                    end
                if ( i_lua_inst_stage == 'h2 )
                    begin
                    // move scratch to r3
                    lua_rm_reg = SCRATCH;
                    barrel_shift_data_sel_nxt = 2'd2;

                    // results in 'r3
                    reg_bank_wen_nxt  = decode ('d3);
                    lua_inst_stage_reg = 'h3;
                    end
                if ( i_lua_inst_stage == 'h3 )
                    begin
                    // write out result itself
                    lua_rd_reg = 'd1;
                    data_access_exec_nxt = 1'd1;
                    write_data_wen_nxt = 1'd1; // write out
                    address_sel_nxt = 4'd1; // alu out

                    alu_out_sel_nxt  = 4'd1; // Add

                    lua_store_a = 1'd1;

                    lua_rn_reg = 'd3;
                    lua_inst_stage_reg = 'h4;
                    end
                if ( i_lua_inst_stage == 'h4 )
                    begin
                    // write out scratch to R(A)
                    data_access_exec_nxt = 1'd1;
                    lua_rd_reg = SCRATCH;
                    write_data_wen_nxt = 1'd1; // write out
                    address_sel_nxt = 4'd1; // alu out

                    alu_out_sel_nxt  = 4'd1; // Add

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                    lua_custom_imm32 = offseta;

                    // also, bump scratch
                    reg_bank_wen_nxt  = decode (SCRATCH);
                    reg_write_sel_nxt = 4'd9; // alu_out_plus_4

                    lua_store_a = 1'd1;

                    lua_rn_reg = BASE;
                    lua_inst_stage_reg = 'h0;
                    end
                // if ( i_lua_inst_stage == 'h5 )
                //     begin
                //     // bump scratch
                //     lua_rn_reg = SCRATCH;
                //     barrel_shift_data_sel_nxt = 2'd0;
                //     barrel_shift_function_nxt   = 0; // LSL
                //     barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                //     lua_custom_imm32 = 32'd1;

                //     reg_bank_wen_nxt  = decode (SCRATCH);
                //     reg_write_sel_nxt = 4'd0; // alu_out

                //     alu_out_sel_nxt  = 4'd1; // Add

                //     lua_inst_stage_reg = 'h0;
                //     end
            end

            if ( itype == LUA_SET2 )
                begin
                    if ( i_lua_inst_stage == 'd1 )
                        begin

                        if ( opcode == LUA_OP_SETTABUP )
                            begin
                                // deref r1 again to get the value of the upval
                                data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                                address_sel_nxt = 4'd1; // alu out
                                alu_out_sel_nxt = 4'd1; // Add
                                lua_fetch_1 = 1'd1;

                                barrel_shift_data_sel_nxt = 2'd0;
                                barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount

                                lua_rn_reg = 'd1;
                            end

                        lua_inst_stage_reg = 'd2;
                        end
                    if ( i_lua_inst_stage == 'd2 )
                        begin
                        // r1 has the table pointer, so deref it again to get the raw table storage base
                        data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                        address_sel_nxt = 4'd1; // alu out
                        alu_out_sel_nxt = 4'd1; // Add
                        lua_fetch_1 = 1'd1;

                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount

                        lua_rn_reg = 'd1;
                        lua_inst_stage_reg = 'd3;
                        end
                    if ( i_lua_inst_stage == 'd3 )
                        begin
                        // load RK(B)
                        data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                        address_sel_nxt = 4'd1; // alu out
                        alu_out_sel_nxt = 4'd1; // Add
                        lua_fetch_2 = 1'd1;
                        lua_custom_imm32 = {24'd0, lua_bk_sel_nxt};

                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_function_nxt   = 0; // LSL
                        barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                        lua_rn_reg = lua_bk_base;
                        lua_inst_stage_reg = 'd4;
                        end
                    if ( i_lua_inst_stage == 'd4 )
                        begin
                        // deref r2 again in preparation for "hashing" it
                        data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                        address_sel_nxt = 4'd1; // alu out
                        alu_out_sel_nxt = 4'd1; // Add
                        lua_fetch_2 = 1'd1;

                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount

                        lua_rn_reg = 'd2;
                        lua_inst_stage_reg = 'd5;
                        end
                    if ( i_lua_inst_stage == 'd5 )
                        begin
                        // shift and mask r2 to get the hash
                        // hash(x) = (x & 0x7fff);

                        lua_rn_reg = 'd2;

                        lua_custom_imm32 = 32'h7fff;

                        reg_bank_wen_nxt  = decode ('d2);
                        reg_write_sel_nxt = 4'd0; // alu_out

                        alu_out_sel_nxt  = 4'd9; // and
                        lua_inst_stage_reg = 'd6;
                        end
                    if ( i_lua_inst_stage == 'd6 )
                        begin
                        // calculate addr: r1 (table) + (r2 << 2) + 4, into r0
                        reg_bank_wen_nxt  = decode ('d0);
                        reg_write_sel_nxt = 4'd0; // alu_out, since we already have +4
                        alu_out_sel_nxt = 4'd1; // Add

                        lua_rm_reg = 'd2;
                        barrel_shift_data_sel_nxt = 2'd2;
                        barrel_shift_function_nxt   = 0; // LSL
                        barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                        lua_rn_reg = 'd1;
                        lua_inst_stage_reg = 'd7;
                        end
                    if ( i_lua_inst_stage == 'd7 )
                        begin
                        // load RK(C)
                        data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                        address_sel_nxt = 4'd1; // alu out
                        alu_out_sel_nxt = 4'd1; // Add
                        lua_fetch_2 = 1'd1;
                        lua_custom_imm32 = {24'd0, lua_ck_sel_nxt};

                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_function_nxt   = 0; // LSL
                        barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                        lua_rn_reg = lua_ck_base;
                        lua_inst_stage_reg = 'd8;
                        end
                    if ( i_lua_inst_stage == 'd8 )
                        begin
                        // store r2 in [r0]
                        lua_rd_reg = 'd2;
                        data_access_exec_nxt = 1'd1; //     indicate that its a data read or write,
                        write_data_wen_nxt = 1'd1; // write out
                        address_sel_nxt = 4'd4; // rn

                        lua_store_a = 1'd1;
                        lua_rn_reg = 'd0;
                        lua_inst_stage_reg = 'd0;
                        end
                end

            if ( itype == LUA_SET )
                begin
                    if ( i_lua_inst_stage == 'd1 )
                        begin

                        if ( opcode == LUA_OP_SETUPVAL )
                            begin
                                // load upvals(B) into r2
                                data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                                address_sel_nxt = 4'd1; // alu out
                                alu_out_sel_nxt = 4'd1; // Add
                                lua_fetch_2 = 1'd1;

                                barrel_shift_data_sel_nxt = 2'd0;
                                barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                                // shift offset
                                lua_custom_imm32 = {23'd0, lua_b_sel_nxt};

                                lua_rn_reg = UPVALS;
                            end

                        lua_inst_stage_reg = 'd2;
                        end
                    if ( i_lua_inst_stage == 'd2 )
                        begin
                         if ( opcode == LUA_OP_SETUPVAL )
                            begin
                            // store r1 in [r2]
                            lua_rd_reg = 'd1;
                            data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                            write_data_wen_nxt = 1'd1; // write out
                            address_sel_nxt = 4'd4; // rn
                            lua_store_a = 1'd1;
                            lua_rn_reg = 'd2;
                            lua_inst_stage_reg = 'd0;
                            end
                        end
                end

            if ( itype == LUA_GET )
                begin
                    if ( i_lua_inst_stage == 'd1 )
                        begin

                        lua_inst_stage_reg = 'd2;
                        if ( opcode == LUA_OP_GETUPVAL || opcode == LUA_OP_GETTABUP )
                            begin
                                // deref d1 again to get the value of the upval
                                data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                                address_sel_nxt = 4'd1; // alu out
                                alu_out_sel_nxt = 4'd1; // Add
                                lua_fetch_1 = 1'd1;

                                barrel_shift_data_sel_nxt = 2'd0;
                                barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount

                                lua_rn_reg = 'd1;
                            end

                        end
                    if ( i_lua_inst_stage == 'd2 )
                        begin

                        lua_inst_stage_reg = 'd7;
                        if ( opcode == LUA_OP_GETTABLE || opcode == LUA_OP_GETTABUP )
                            begin
                                // deref d1 to get raw table base
                                data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                                address_sel_nxt = 4'd1; // alu out
                                alu_out_sel_nxt = 4'd1; // Add
                                lua_fetch_1 = 1'd1;

                                barrel_shift_data_sel_nxt = 2'd0;
                                barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount

                                lua_rn_reg = 'd1;
                                lua_inst_stage_reg = 'd3;
                            end

                        end
                    if ( i_lua_inst_stage == 'd3 )
                        begin
                        // resolve RK(C) into r2
                        data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                        address_sel_nxt = 4'd1; // alu out
                        alu_out_sel_nxt = 4'd1; // Add
                        lua_fetch_2 = 1'd1;
                        lua_custom_imm32 = {24'd0, lua_ck_sel_nxt};

                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_function_nxt   = 0; // LSL
                        barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                        lua_rn_reg = lua_ck_base;
                        lua_inst_stage_reg = 'd4;
                        end
                    if ( i_lua_inst_stage == 'd4 )
                        begin
                        // deref r2 again in preparation for "hashing" it
                        data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                        address_sel_nxt = 4'd1; // alu out
                        alu_out_sel_nxt = 4'd1; // Add
                        lua_fetch_2 = 1'd1;

                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount

                        lua_rn_reg = 'd2;
                        lua_inst_stage_reg = 'd5;
                        end
                    if ( i_lua_inst_stage == 'd5 )
                        begin
                        // shift and mask r2 to get the hash
                        // hash(x) = (x & 0x7fff);

                        lua_rn_reg = 'd2;

                        lua_custom_imm32 = 32'h7fff;

                        reg_bank_wen_nxt  = decode ('d2);
                        reg_write_sel_nxt = 4'd0; // alu_out

                        alu_out_sel_nxt  = 4'd9; // and
                        lua_inst_stage_reg = 'd6;
                        end
                    if ( i_lua_inst_stage == 'd6 )
                        begin
                        // read from r1 (table) + (r2 << 2) + 4, into r1
                        data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                        address_sel_nxt = 4'd1; // alu out
                        alu_out_sel_nxt = 4'd1; // Add
                        lua_fetch_1 = 1'd1;

                        lua_rm_reg = 'd2;
                        barrel_shift_data_sel_nxt = 2'd2;
                        barrel_shift_function_nxt   = 0; // LSL
                        barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                        lua_rn_reg = 'd1;
                        lua_inst_stage_reg = 'd7;
                        end
                    if ( i_lua_inst_stage == 'd7 )
                        begin
                        // store r1 in R(A)
                        data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                        lua_rd_reg = 'd1;
                        write_data_wen_nxt = 1'd1; // write out
                        address_sel_nxt = 4'd1; // alu out

                        alu_out_sel_nxt  = 4'd1; // Add

                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_amount_sel_nxt = 2'd2;
                        lua_store_a = 1'd1;

                        lua_rn_reg = BASE;
                        lua_inst_stage_reg = 'd0;
                        end
                end

            if ( itype == LUA_TABLE )
                begin
                // we reserve 0x20000 bytes for the table. B and C are ignored.
                if ( i_lua_inst_stage == 'h1 )
                    begin
                    // copy scratch to r2
                    lua_rm_reg = SCRATCH;
                    reg_bank_wen_nxt  = decode ('d2);
                    barrel_shift_data_sel_nxt = 2'd2;
                    lua_inst_stage_reg = 'h2;

                    end
                if ( i_lua_inst_stage == 'h2 )
                    begin
                    // increment scratch by 4
                    lua_rm_reg = SCRATCH;
                    barrel_shift_data_sel_nxt = 2'd2;
                    reg_bank_wen_nxt  = decode (SCRATCH);
                    reg_write_sel_nxt = 4'd9; // alu_out_plus4
                    lua_inst_stage_reg = 'h3;
                    end
                if ( i_lua_inst_stage == 'h3 )
                    begin
                    // write out scratch to r2, representing the table's pointer to its buffer
                    data_access_exec_nxt = 1'd1;
                    lua_rd_reg = SCRATCH;
                    write_data_wen_nxt = 1'd1; // write out
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt  = 4'd1; // Add

                    lua_store_a = 1'd1;

                    lua_rn_reg = 'd2;
                    lua_inst_stage_reg = 'h4;
                    end
                if ( i_lua_inst_stage == 'h4 )
                    begin
                    // increment scratch appropriately for our new table size
                    // write out result itself to scratch
                    lua_rn_reg = SCRATCH;
                    barrel_shift_data_sel_nxt = 2'd0;
                    lua_custom_imm32 = 32'h20000; // 4 bytes for table hash, 0x20000 bytes for the table itself

                    reg_bank_wen_nxt  = decode (SCRATCH);
                    reg_write_sel_nxt = 4'd0; // alu_out

                    alu_out_sel_nxt  = 4'd1; // Add

                    lua_inst_stage_reg = 'h5;
                    end
                if ( i_lua_inst_stage == 'h5 )
                    begin
                    // write out result, r1 to R(A)
                    data_access_exec_nxt = 1'd1;
                    lua_rd_reg = 'd2;
                    write_data_wen_nxt = 1'd1; // write out
                    address_sel_nxt = 4'd1; // alu out

                    alu_out_sel_nxt  = 4'd1; // Add

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                    lua_custom_imm32 = offseta;

                    lua_store_a = 1'd1;

                    lua_rn_reg = BASE;
                    lua_inst_stage_reg = 'h0;
                    end
                end

            if ( itype == LUA_RETURN )
                begin
                    if ( i_lua_inst_stage == 'h1 )
                        begin
                        // calculate &R(A), which is the start of our return arguments
                        alu_out_sel_nxt = 4'd1; // Add
                        lua_custom_imm32 = offseta;

                        reg_bank_wen_nxt  = decode ('d0);
                        reg_write_sel_nxt = 4'd0; // alu_out

                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_function_nxt   = 0; // LSL
                        barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                        lua_rn_reg = BASE;
                        lua_inst_stage_reg = 'h2;
                        end

                    if ( i_lua_inst_stage == 'h2 )
                        begin
                        // write our DMA engine address to r5
                        reg_bank_wen_nxt  = decode ('d5);
                        lua_custom_imm32 = 32'hC0000000; // dma engine base
                        reg_write_sel_nxt = 4'ha; // direct barrel shift output
                        lua_inst_stage_reg = 'h3;
                        end

                    if ( i_lua_inst_stage == 'h3 )
                        begin
                        // store r0 in dma engine src
                        data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                        lua_rd_reg = 'd0;
                        write_data_wen_nxt = 1'd1; // write out
                        address_sel_nxt = 4'd1; // alu out

                        alu_out_sel_nxt  = 4'd1; // Add
                        lua_store_a = 1'd1;

                        lua_rn_reg = 'd5;
                        lua_inst_stage_reg = 'h4;
                        end

                    // reload our callinfo into our current register state
                    if (
                            i_lua_inst_stage == 'h04 ||
                            i_lua_inst_stage == 'h06 ||
                            i_lua_inst_stage == 'h08 ||
                            i_lua_inst_stage == 'h0a ||
                            i_lua_inst_stage == 'h0c ||
                            i_lua_inst_stage == 'h0e ||
                            i_lua_inst_stage == 'h10 ||
                            i_lua_inst_stage == 'h12
                        )
                        begin

                        data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                        address_sel_nxt = 4'd1; // alu out
                        alu_out_sel_nxt = 4'd1; // Add
                        lua_fetch_1 = 1'd1;

                        // shift offset to read each frame field
                        lua_custom_imm32 = {28'd0, i_lua_inst_stage == 'h04 ? 4'h0 :
                                                   i_lua_inst_stage == 'h06 ? 4'h1 :
                                                   i_lua_inst_stage == 'h08 ? 4'h2 :
                                                   i_lua_inst_stage == 'h0a ? 4'h3 :
                                                   i_lua_inst_stage == 'h0c ? 4'h4 :
                                                   i_lua_inst_stage == 'h0e ? 4'h5 :
                                                   i_lua_inst_stage == 'h10 ? 4'h6 :
                                                                              4'h7 };
                        lua_rn_reg = FRAMES; // activation frames stack

                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_function_nxt   = 0; // LSL
                        barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                        lua_inst_stage_reg =   i_lua_inst_stage == 'h04 ? 'h05 :
                                               i_lua_inst_stage == 'h06 ? 'h07 :
                                               i_lua_inst_stage == 'h08 ? 'h09 :
                                               i_lua_inst_stage == 'h0a ? 'h0b :
                                               i_lua_inst_stage == 'h0c ? 'h0d :
                                               i_lua_inst_stage == 'h0e ? 'h0f :
                                               i_lua_inst_stage == 'h10 ? 'h11 :
                                                                          'h13 ;
                        end
                    if (
                            i_lua_inst_stage == 'h05 ||
                            i_lua_inst_stage == 'h07 ||
                            i_lua_inst_stage == 'h09 ||
                            i_lua_inst_stage == 'h0b ||
                            i_lua_inst_stage == 'h0d ||
                            i_lua_inst_stage == 'h0f ||
                            i_lua_inst_stage == 'h11 ||
                            i_lua_inst_stage == 'h13
                        )
                        begin

                        // value was read into r1 -- put it into dest now
                        // this is very inefficient and could be optimized - just make lua_store_a
                        // able to write to a specific register, instead of always r1!
                        lua_rm_reg = 'd1;
                        barrel_shift_data_sel_nxt = 2'd2;

                        reg_write_sel_nxt = 4'ha; // direct barrel shift output

                        /*
                        0x00: func, position of the closure on the stack
                        0x04: base we had when calling
                        0x08: top we had when calling
                        0x0c: upvals we had when calling
                        0x10: protos we had when calling
                        0x14: constants we had when calling
                        0x18: pc we return to
                        0x1c: nresults we need to return <from this frame>
                        */
                        reg_bank_wen_nxt  = decode (
                                               i_lua_inst_stage == 'h05 ? 4'h0 :
                                               i_lua_inst_stage == 'h07 ? BASE :
                                               i_lua_inst_stage == 'h09 ? TOP :
                                               i_lua_inst_stage == 'h0b ? UPVALS :
                                               i_lua_inst_stage == 'h0d ? PROTOS :
                                               i_lua_inst_stage == 'h0f ? CONSTANTS :
                                               i_lua_inst_stage == 'h11 ? 4'd15 :
                                                                          4'h1 );

                        // saved pc occurs on this stage
                        pc_wen_nxt = (i_lua_inst_stage == 'h11);
                        pc_sel_nxt = (i_lua_inst_stage == 'h11) ? 3'd1 : 0;

                        lua_inst_stage_reg =   i_lua_inst_stage == 'h05 ? 'h06 :
                                               i_lua_inst_stage == 'h07 ? 'h08 :
                                               i_lua_inst_stage == 'h09 ? 'h0a :
                                               i_lua_inst_stage == 'h0b ? 'h0c :
                                               i_lua_inst_stage == 'h0d ? 'h0e :
                                               i_lua_inst_stage == 'h0f ? 'h10 :
                                               i_lua_inst_stage == 'h11 ? 'h12 :
                                                                          'h14 ;
                        end

                    if ( i_lua_inst_stage == 'h14 )
                        begin

                        // store our saved func in the dest reg of the DMA engine
                        data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                        lua_rd_reg = 'd0;
                        write_data_wen_nxt = 1'd1; // write out
                        address_sel_nxt = 4'd1; // alu out

                        alu_out_sel_nxt  = 4'd1; // Add
                        lua_store_a = 1'd1;

                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_function_nxt   = 0; // LSL
                        barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                        lua_custom_imm32 = 32'd1; // offset (*4)

                        lua_rn_reg = 'd5;
                        lua_inst_stage_reg = 'h15;
                        end
                    if ( i_lua_inst_stage == 'h15 )
                        begin
                        // store nresults in the count field of the DMA engine
                        data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                        lua_rd_reg = 'd1;
                        write_data_wen_nxt = 1'd1; // write out
                        address_sel_nxt = 4'd1; // alu out

                        alu_out_sel_nxt  = 4'd1; // Add
                        lua_store_a = 1'd1;

                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_function_nxt   = 0; // LSL
                        barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                        lua_custom_imm32 = 32'd2; // offset (*4)

                        lua_rn_reg = 'd5;
                        lua_inst_stage_reg = 'h16;
                        end

                    if ( i_lua_inst_stage == 'h16 )
                        begin
                        // write our DMA engine config to r0
                        data_access_exec_nxt = 1'd1;
                        reg_bank_wen_nxt  = decode ('d0);
                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount
                        lua_custom_imm32 = {
                            16'd0, // padding
                            8'd1, // size: 4
                            4'd0, // op = move
                            2'd0, // padding
                            1'd0, // direction 0, copy src -> dst
                            1'd1 // enable bit
                        };
                        reg_write_sel_nxt = 4'ha; // direct barrel shift output
                        lua_inst_stage_reg = 'h17;
                        end
                    if ( i_lua_inst_stage == 'h17 )
                        begin
                        // store config in the cfg field of the DMA engine
                        // this also drives the engine, since the enable bit is set
                        data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                        lua_rd_reg = 'd0;
                        write_data_wen_nxt = 1'd1; // write out
                        address_sel_nxt = 4'd1; // alu out

                        alu_out_sel_nxt  = 4'd1; // Add
                        lua_store_a = 1'd1;

                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_function_nxt   = 0; // LSL
                        barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                        lua_custom_imm32 = 32'd3; // offset (*4)

                        lua_rn_reg = 'd5;
                        lua_inst_stage_reg = 'h18;
                        end
                    if ( i_lua_inst_stage == 'h18 )
                        begin
                        // move back our activation frame stack pointer
                        lua_rn_reg = FRAMES;
                        barrel_shift_data_sel_nxt = 2'd0;
                        lua_custom_imm32 = 32'hffffffe0;

                        reg_bank_wen_nxt  = decode (FRAMES);
                        reg_write_sel_nxt = 4'd0; // alu_out

                        alu_out_sel_nxt  = 4'd1; // Add

                        lua_inst_stage_reg = 'h19;
                        end
                    if ( i_lua_inst_stage == 'h19 )
                        begin
                        data_access_exec_nxt            = 1'd1;
                        lua_rd_reg = FRAMES;
                        write_data_wen_nxt = 1'd1; // write out
                        address_sel_nxt = 4'd1; // alu out

                        alu_out_sel_nxt  = 4'd1; // Add

                        lua_custom_imm32 = 'd3;
                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_amount_sel_nxt = 2'd2; // no shift
                        lua_store_a = 1'd1;

                        lua_rn_reg = 'd5;
                        pre_fetch_instruction_wen = 1'd1;
                        lua_inst_stage_reg = 'h1a;
                        end
                    if ( i_lua_inst_stage == 'h1a )
                        begin
                        // stall...
                        pc_wen_nxt = 'd1;
                        lua_inst_stage_reg = 'h1b;
                        end
                    if ( i_lua_inst_stage == 'h1b )
                        begin
                        // stall...
                        pc_wen_nxt = 'd1;
                        lua_prefetch_1 = 'd1;
                        lua_inst_stage_reg = 'h0;
                        end
                end

            if ( itype == LUA_CALL )
                begin
                    if ( i_lua_inst_stage == 'd1 )
                        begin

                        // fetch R(A), our closure object. also, store its address in r0
                        data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                        address_sel_nxt = 4'd1; // alu out
                        alu_out_sel_nxt = 4'd1; // Add
                        lua_fetch_1 = 1'd1;
                        lua_custom_imm32 = offseta;

                        reg_bank_wen_nxt  = decode ('d0);
                        reg_write_sel_nxt = 4'd0; // alu_out

                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_function_nxt   = 0; // LSL
                        barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                        lua_rn_reg = BASE;
                        lua_inst_stage_reg = 'd2;
                        end
                    if ( i_lua_inst_stage == 'd2 )
                        begin

                        // bump our activation frame stack pointer
                        lua_rn_reg = FRAMES;
                        barrel_shift_data_sel_nxt = 2'd0;
                        lua_custom_imm32 = 32'h20;

                        reg_bank_wen_nxt  = decode (FRAMES);
                        reg_write_sel_nxt = 4'd0; // alu_out

                        alu_out_sel_nxt  = 4'd1; // Add

                        lua_inst_stage_reg = 'd3;
                        end
                    if (
                            i_lua_inst_stage == 'd3 ||
                            i_lua_inst_stage == 'd4 ||
                            i_lua_inst_stage == 'd5 ||
                            i_lua_inst_stage == 'd6 ||
                            i_lua_inst_stage == 'd7 ||
                            i_lua_inst_stage == 'd8
                        )
                        begin

                        // spill current state to activation frame stack
                        lua_rd_reg = i_lua_inst_stage == 'd3 ? 'd0       : // current closure
                                     i_lua_inst_stage == 'd4 ? BASE      :
                                     i_lua_inst_stage == 'd5 ? TOP       :
                                     i_lua_inst_stage == 'd6 ? UPVALS    :
                                     i_lua_inst_stage == 'd7 ? PROTOS    :
                                                               CONSTANTS ;
                        write_data_wen_nxt = 1'd1; // write out
                        address_sel_nxt = 4'd1; // alu out

                        alu_out_sel_nxt  = 4'd1; // Add

                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                        // shift offset
                        lua_custom_imm32 = i_lua_inst_stage == 'd3 ? 'h0 :
                                           i_lua_inst_stage == 'd4 ? 'h1 :
                                           i_lua_inst_stage == 'd5 ? 'h2 :
                                           i_lua_inst_stage == 'd6 ? 'h3 :
                                           i_lua_inst_stage == 'd7 ? 'h4 :
                                                                     'h5 ;
                        lua_store_a = 1'd1;
                        lua_rn_reg = FRAMES;
                        lua_inst_stage_reg = i_lua_inst_stage == 'd3 ? 'd4 :
                                             i_lua_inst_stage == 'd4 ? 'd5 :
                                             i_lua_inst_stage == 'd5 ? 'd6 :
                                             i_lua_inst_stage == 'd6 ? 'd7 :
                                             i_lua_inst_stage == 'd7 ? 'd8 :
                                                                       'd9 ;
                        end
                    if ( i_lua_inst_stage == 'd9 )
                        begin
                            // write our saved pc to r2
                            reg_bank_wen_nxt  = decode ('d2);
                            barrel_shift_data_sel_nxt = 2'd0;
                            barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount
                            lua_custom_imm32 = pre_fetch_instruction_address;
                            reg_write_sel_nxt = 4'ha; // direct barrel shift output
                            lua_inst_stage_reg = 'ha;
                        end
                    if ( i_lua_inst_stage == 'ha )
                        begin
                        // save pc to activation frame
                        lua_rd_reg = 'd2; // r2
                        write_data_wen_nxt = 1'd1; // write out
                        address_sel_nxt = 4'd1; // alu out

                        alu_out_sel_nxt  = 4'd1; // Add

                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                        lua_custom_imm32 = 'h6; // recall this is shifted right by 2
                        lua_store_a = 1'd1;

                        lua_rn_reg = FRAMES;

                        lua_inst_stage_reg = 'hb;

                        end
                    if ( i_lua_inst_stage == 'hb )
                        begin
                            // retrieve the proto object from our closure -- it is the very first field
                            data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                            address_sel_nxt = 4'd1; // alu out
                            alu_out_sel_nxt = 4'd1; // Add
                            lua_fetch_2 = 1'd1;

                            barrel_shift_data_sel_nxt = 2'd0;
                            barrel_shift_function_nxt   = 0; // LSL
                            barrel_shift_amount_sel_nxt = 2'd0;

                            lua_rn_reg = 'd1; // the closure, r1
                            lua_inst_stage_reg = 'hc;
                        end
                    if ( i_lua_inst_stage == 'hc )
                        begin
                            // calculate our new base, which is just func + 4
                            lua_rn_reg = 'd0;
                            barrel_shift_data_sel_nxt = 2'd0;
                            lua_custom_imm32 = 32'd4;

                            reg_bank_wen_nxt  = decode (BASE);
                            reg_write_sel_nxt = 4'd0; // alu_out
                            alu_out_sel_nxt  = 4'd1; // Add

                            lua_inst_stage_reg = 'hd;
                        end
                    if ( i_lua_inst_stage == 'hd )
                        begin
                            // update our base to point to func+4
                            lua_rn_reg = 'd0;
                            barrel_shift_data_sel_nxt = 2'd0;
                            lua_custom_imm32 = 32'h4;

                            reg_bank_wen_nxt  = decode (BASE);
                            reg_write_sel_nxt = 4'd0; // alu_out

                            alu_out_sel_nxt  = 4'd1; // Add

                            lua_inst_stage_reg = 'he;
                        end
                    if ( i_lua_inst_stage == 'he )
                        begin
                            // set our upvals to point to closure+4
                            lua_rn_reg = 'd1;
                            barrel_shift_data_sel_nxt = 2'd0;
                            lua_custom_imm32 = 32'h4;

                            reg_bank_wen_nxt  = decode (UPVALS);
                            reg_write_sel_nxt = 4'd0; // alu_out

                            alu_out_sel_nxt  = 4'd1; // Add

                            lua_inst_stage_reg = 'h0f;
                        end

                    if (
                            i_lua_inst_stage == 'h0f ||
                            i_lua_inst_stage == 'h11 ||
                            i_lua_inst_stage == 'h13 ||
                            i_lua_inst_stage == 'h15
                        )
                        begin

                        // read our new state from the proto
                        /*
                        new values:
                            protos - from new closure->proto->protos
                            top = newbase + proto->maxstacksize
                            constants - from new closure->proto->constants
                            pc - from new closure->proto->instructions
                        */

                        data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                        address_sel_nxt = 4'd1; // alu out
                        alu_out_sel_nxt = 4'd1; // Add
                        lua_fetch_1 = 1'd1;

                        // shift offset to read each proto field
                        lua_custom_imm32 = {28'd0, i_lua_inst_stage == 'h0f ? PROTO_OFF_INSTRUCTION :
                                                   i_lua_inst_stage == 'h11 ? PROTO_OFF_MAXSTACKSIZE :
                                                   i_lua_inst_stage == 'h13 ? PROTO_OFF_CONSTANTS :
                                                                              PROTO_OFF_PROTOS };
                        lua_rn_reg = 'd2; // proto

                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_function_nxt   = 0; // LSL
                        barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                        lua_inst_stage_reg = i_lua_inst_stage == 'h0f ? 'h10 :
                                             i_lua_inst_stage == 'h11 ? 'h12 :
                                             i_lua_inst_stage == 'h13 ? 'h14 :
                                                                        'h16 ;
                        end
                    if (
                            i_lua_inst_stage == 'h10 ||
                            i_lua_inst_stage == 'h12 ||
                            i_lua_inst_stage == 'h14 ||
                            i_lua_inst_stage == 'h16
                        )
                        begin

                        // value was read into r1 -- put it into dest now
                        // this is very inefficient and could be optimized - just make lua_store_a
                        // able to write to a specific register, instead of always r1!
                        lua_rm_reg = 'd1;
                        barrel_shift_data_sel_nxt = 2'd2;

                        reg_write_sel_nxt = 4'ha; // direct barrel shift output

                        reg_bank_wen_nxt  = decode ( i_lua_inst_stage == 'h10 ? 'd15 :
                                                     i_lua_inst_stage == 'h12 ? TOP : // we will fix this one up in the following steps
                                                     i_lua_inst_stage == 'h14 ? CONSTANTS :
                                                                                PROTOS);

                        pc_wen_nxt = (i_lua_inst_stage == 'h10);
                        pc_sel_nxt = (i_lua_inst_stage == 'h10) ? 3'd1 : 0;

                        lua_inst_stage_reg = i_lua_inst_stage == 'h10 ? 'h11 :
                                             i_lua_inst_stage == 'h12 ? 'h13 :
                                             i_lua_inst_stage == 'h14 ? 'h15 :
                                                                        'h17 ;
                        end
                    if ( i_lua_inst_stage == 'h17 )
                        begin
                        // // add our new base to top (which temporarily is holding maxstacksiez) to calculate the new top
                        // lua_rn_reg = BASE;
                        // lua_rm_reg = TOP;
                        // barrel_shift_data_sel_nxt = 2'd2;
                        // data_access_exec_nxt      = 1'd1;
                        // reg_bank_wen_nxt = decode(TOP);
                        // reg_write_sel_nxt = 4'd0; // alu_out

                        // this is a hack -- just alway use maxstacksize as the # of args to copy back
                        // this often copies back more results than needed, but we cheat with our "dma"
                        lua_rm_reg = TOP;
                        barrel_shift_data_sel_nxt = 2'd2;
                        barrel_shift_amount_sel_nxt = 2'd2;
                        barrel_shift_function_nxt   = 1; // LSR

                        // results in r0
                        reg_bank_wen_nxt  = decode ('d0);

                        lua_inst_stage_reg = 'h19;

                        end
                    // if ( i_lua_inst_stage == 'h18 )
                    //     begin
                    //     // load nresults from C into r0, in preparation for storing in our activation frame
                    //     barrel_shift_data_sel_nxt = 2'd0;
                    //     lua_custom_imm32 = lua_c_sel_nxt == 'd0 ? 32'd1 : {23'd0, lua_c_sel_nxt};

                    //     reg_bank_wen_nxt  = decode ('d0);
                    //     reg_write_sel_nxt = 4'ha; // barrel_shift_out
                    //     lua_inst_stage_reg = 'h19;
                    //     end
                    if ( i_lua_inst_stage == 'h19 )
                        begin
                        // subtract 1 from results, unconditionally
                        lua_rn_reg = 'd0;
                        barrel_shift_data_sel_nxt = 2'd0;
                        lua_custom_imm32 = 32'hffffffff;

                        reg_bank_wen_nxt  = decode ('d0);
                        reg_write_sel_nxt = 4'd0; // alu_out

                        alu_out_sel_nxt  = 4'd1; // Add

                        lua_inst_stage_reg = 'h1a;
                        end
                    if ( i_lua_inst_stage == 'h1a )
                        begin
                        // spill nresults-1 to activation frame stack
                        lua_rd_reg = 'd0;
                        write_data_wen_nxt = 1'd1; // write out
                        address_sel_nxt = 4'd1; // alu out

                        alu_out_sel_nxt  = 4'd1; // Add

                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                        // shift offset
                        lua_custom_imm32 = 'h7;
                        lua_store_a = 1'd1;
                        lua_rn_reg = FRAMES;
                        lua_inst_stage_reg = 'h1b;
                        end
                    if ( i_lua_inst_stage == 'h1b )
                        begin
                        lua_inst_stage_reg = 'h1c;
                        end
                    if ( i_lua_inst_stage == 'h1c )
                        begin
                        pre_fetch_instruction_wen = 1'd1;
                        lua_inst_stage_reg = 'h1d;
                        end
                    if ( i_lua_inst_stage == 'h1d )
                        begin
                        // stall...
                        pc_wen_nxt = 'd1;
                        lua_inst_stage_reg = 'h1e;
                        end
                    if ( i_lua_inst_stage == 'h1e )
                        begin
                        // stall...
                        pc_wen_nxt = 'd1;
                        lua_prefetch_1 = 'd1;
                        lua_inst_stage_reg = 'h0;
                        end
                end
            if ( itype == LUA_CLOSURE )
                begin

                    if ( i_lua_inst_stage == 'd0 )
                        begin
                        // fetch number of upvals
                        data_access_exec_nxt            = 1'd1; // indicate that its a data read or write,
                        address_sel_nxt = 4'd1; // alu out
                        alu_out_sel_nxt = 4'd1; // Add
                        lua_fetch_2 = 1'd1;

                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_function_nxt   = 0; // LSL
                        barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                        lua_rn_reg = 'd1;

                        lua_custom_imm32 = {28'd0, PROTO_OFF_MAXUPVALS};
                        lua_inst_stage_reg = 'd1;
                        end
                    if ( i_lua_inst_stage == 'd1 )
                        begin
                        // move scratch to r3, this will be the base of our closure
                        lua_rm_reg = SCRATCH;
                        barrel_shift_data_sel_nxt = 2'd2;

                        // results in 'r3
                        reg_bank_wen_nxt  = decode ('d3);
                        lua_inst_stage_reg = 'd2;
                        end
                    if ( i_lua_inst_stage == 'd2 )
                        begin
                        // bump scratch register
                        lua_rn_reg = SCRATCH;
                        lua_rm_reg = 'd2;
                        barrel_shift_data_sel_nxt = 2'd2;
                        barrel_shift_function_nxt   = 0; // LSL
                        barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount

                        reg_bank_wen_nxt  = decode (SCRATCH);
                        reg_write_sel_nxt = 4'd9; // alu_out_plus4

                        alu_out_sel_nxt  = 4'd1; // Add

                        lua_inst_stage_reg = 'd4;
                        end
                     if ( i_lua_inst_stage == 'd4 )
                        begin
                        // store proto -> closurebase[0]
                        data_access_exec_nxt            = 1'd1;
                        lua_rd_reg = 'd1;
                        write_data_wen_nxt = 1'd1; // write out
                        address_sel_nxt = 4'd1; // alu out

                        alu_out_sel_nxt  = 4'd1; // Add

                        lua_custom_imm32 = 'd0;
                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_amount_sel_nxt = 2'd0; // no shift
                        lua_store_a = 1'd1;

                        lua_rn_reg = 'd3;
                        lua_inst_stage_reg = 'd5;
                        end
                    if ( i_lua_inst_stage == 'd5 )
                        begin
                        // fetch upval base
                        data_access_exec_nxt            = 1'd1; // indicate that its a data read or write,
                        address_sel_nxt = 4'd1; // alu out
                        alu_out_sel_nxt = 4'd1; // Add
                        lua_fetch_1 = 1'd1;

                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_function_nxt   = 0; // LSL
                        barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                        lua_rn_reg = 'd1;

                        lua_custom_imm32 = {28'd0, PROTO_OFF_UPVALS};
                        lua_inst_stage_reg = 'd6;
                        end
                    if ( i_lua_inst_stage == 'd6 )
                        begin
                        // move # of upvals to to r4
                        lua_rm_reg = 'd2;
                        barrel_shift_data_sel_nxt = 2'd2;

                        // results in 'r4
                        reg_bank_wen_nxt  = decode ('d4);
                        lua_inst_stage_reg = 'd7;
                        end

                    // upval fetch and decode loop
                    if ( i_lua_inst_stage == 'd7 )
                        begin

                        // r0 -> current working upval offset
                        // r1 -> upval base
                        // r2 -> scratch
                        // r3 -> closure base
                        // r4 -> # of upvals

                        // calculate the next offset to write to
                        // if it does not overflow, we are done with upvals (i.e. it was 0)
                        // bump scratch register
                        lua_rn_reg = 'd4;
                        barrel_shift_data_sel_nxt = 2'd0;
                        lua_custom_imm32 = 32'hfffffffc;

                        reg_bank_wen_nxt  = decode ('d4);
                        reg_write_sel_nxt = 4'd0; // alu_out

                        alu_out_sel_nxt  = 4'd1; // Add
                        alu_cout_sel_nxt = 1'd1;  // i_barrel_shift_carry

                        // detect underflow
                        status_bits_flags_wen_nxt     = 1'd1;

                        lua_inst_stage_reg = 'd8;
                        end
                    if ( (i_lua_inst_stage == 'd8) )
                        begin
                        // stall...
                        lua_inst_stage_reg = 'd9;
                        end
                    if ( (i_lua_inst_stage == 'd9) )
                        begin        
                        lua_inst_stage_reg = 'h0;              
                        if (i_execute_status_bits[31:30] == 2'd0)
                            begin
                            // read upval at that offset, store into r2 
                            lua_rn_reg = 'd1;
                            lua_rm_reg = 'd4;
                            barrel_shift_data_sel_nxt = 2'd2;
                            data_access_exec_nxt            = 1'd1; // indicate that its a data read or write,
                            address_sel_nxt = 4'd1; // alu out
                            alu_out_sel_nxt = 4'd1; // Add
                            lua_fetch_2 = 1'd1;

                            lua_inst_stage_reg = 'ha;
                            end
                        if (i_execute_status_bits[31:30] != 2'd0)
                            begin
                            // store closure base in R(A)
                            data_access_exec_nxt = 1'd1;
                            lua_rd_reg = 'd3;
                            write_data_wen_nxt = 1'd1; // write out
                            address_sel_nxt = 4'd1; // alu out

                            alu_out_sel_nxt  = 4'd1; // Add

                            barrel_shift_data_sel_nxt = 2'd0;
                            barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                            lua_store_a = 1'd1;

                            lua_rn_reg = BASE;
                            end
                        end
                    if ( i_lua_inst_stage == 'ha )
                        begin

                        // lua_scratch_reg contains our previous upval
                        // bottom 8 bits are instack, top 24 are offset

                        // load the actual upval

                        data_access_exec_nxt            = 1'd1; // indicate that its a data read or write,
                        address_sel_nxt = 4'd1; // alu out
                        alu_out_sel_nxt = 4'd1; // Add
                        lua_fetch_2 = 1'd1;

                        barrel_shift_data_sel_nxt = 2'd0;
                        barrel_shift_function_nxt   = 0; // LSL
                        barrel_shift_amount_sel_nxt = 2'd0; // imm_shift_amount

                        lua_rn_reg = lua_scratch_reg[7:0] == 'd1 ? BASE : UPVALS;

                        lua_custom_imm32 = {6'd0, lua_scratch_reg[31:8], 2'd0};

                        // upval requires an extra stage here, for the second deref
                        lua_inst_stage_reg = lua_scratch_reg[7:0] == 'd1 ? 'hc: 'hb;
                        end
                    if ( i_lua_inst_stage == 'hb )
                        begin
                        // optional stage: if we read an upval, deref it again to get the real value
                        data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                        address_sel_nxt = 4'd1; // alu out
                        alu_out_sel_nxt = 4'd1; // Add
                        lua_fetch_2 = 1'd1;
                        lua_rn_reg = 'd2;

                        lua_inst_stage_reg = 'hc;
                        end
                    if ( i_lua_inst_stage == 'hc )
                        begin
                        // write out scratch to upval array, then increment upval ptr
                        data_access_exec_nxt            = 1'd1; // indicate that its a data read or write,
                        lua_rd_reg = SCRATCH;
                        write_data_wen_nxt = 1'd1; // write out
                        address_sel_nxt = 4'd6; // alu out plus4

                        alu_out_sel_nxt  = 4'd1; // Add

                        lua_custom_imm32 = 'd0;
                        lua_store_a = 1'd1;

                        lua_rn_reg = 'd3;
                        lua_rm_reg = 'd4;
                        barrel_shift_data_sel_nxt = 2'd2;
                        lua_inst_stage_reg = 'hd;
                        end
                    if ( i_lua_inst_stage == 'hd )
                        begin

                        // write out upval itself to scratch, then bump scratch
                        data_access_exec_nxt = 1'd1;
                        lua_rd_reg = 'd2;
                        write_data_wen_nxt = 1'd1; // write out
                        address_sel_nxt = 4'd4; // rn

                        alu_out_sel_nxt  = 4'd1; // Add

                        reg_bank_wen_nxt  = decode (SCRATCH);
                        reg_write_sel_nxt = 4'd9; // alu_out_plus4

                        lua_store_a = 1'd1;

                        lua_rn_reg = SCRATCH;
                        lua_inst_stage_reg = 'h7;
                        end
                end
            end

        if ( control_state == LUA_DECODE || (i_lua_mode && control_state == PRE_FETCH_EXEC) || (control_state == LUA_LOAD2F && lua_inst_stage_reg == 'd0 ))
            begin

            pc_wen_nxt = 'd0;
            if ( itype == LUA_MOVE )
                begin
                    saved_current_instruction_wen   = 1'd1;  // Save the memory access instruction to refer back to later
                    data_access_exec_nxt            = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_fetch_1 = 1'd1;

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_function_nxt   = 0; // LSL
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                    // locals or constants
                    lua_rn_reg = move_src(instruction[5:0]);
                end
            if ( itype == LUA_ALU2 )
                begin
                // we need to go to exec stage to begin
                lua_inst_stage_reg = 'd1;
                saved_current_instruction_wen   = 1'd1;

                // fetch R(B) or RK(B)
                data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                address_sel_nxt = 4'd1; // alu out
                alu_out_sel_nxt = 4'd1; // Add
                lua_fetch_1 = 1'd1;
                lua_custom_imm32 = (opcode == LUA_OP_NOT) || (opcode == LUA_OP_BNOT) || (opcode == LUA_OP_UNM) ? {23'd0, lua_b_sel_nxt} : {24'd0, lua_bk_sel_nxt};

                barrel_shift_data_sel_nxt = 2'd0;
                barrel_shift_function_nxt   = 0; // LSL
                barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                lua_rn_reg = (opcode == LUA_OP_NOT) || (opcode == LUA_OP_BNOT) || (opcode == LUA_OP_UNM) ? BASE : lua_bk_base;
                end
            if ( itype == LUA_CLOSURE )
                begin
                    saved_current_instruction_wen   = 1'd1;
                    data_access_exec_nxt            = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt = 4'd1; // Add

                    // fetch closure object itself
                    lua_fetch_1 = 1'd1;

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_function_nxt   = 0; // LSL
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                    lua_rn_reg = PROTOS;
                end
            if ( itype == LUA_CALL )
                begin
                    // we need to go to exec stage to begin
                    lua_inst_stage_reg = 'd1;
                    saved_current_instruction_wen   = 1'd1;
                end
            if ( itype == LUA_RETURN )
                begin
                    lua_inst_stage_reg = 'd1;
                    saved_current_instruction_wen   = 1'd1;
                end
            if ( itype == LUA_TABLE )
                begin
                    lua_inst_stage_reg = 'd1;
                    saved_current_instruction_wen   = 1'd1;
                end
            if ( itype == LUA_SET )
                begin
                    // we need to go to exec stage to begin
                    lua_inst_stage_reg = 'd1;
                    saved_current_instruction_wen   = 1'd1;

                    // fetch R(A), the value we want to store
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_fetch_1 = 1'd1;
                    lua_custom_imm32 = offseta;

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_function_nxt   = 0; // LSL
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                    lua_rn_reg = BASE;
                end
            if ( itype == LUA_SET2 )
                begin
                    lua_inst_stage_reg = 'd1;
                    saved_current_instruction_wen   = 1'd1;

                    // fetch dest table, either R(A) or upvals(A)
                    data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                    address_sel_nxt = 4'd1; // alu out
                    alu_out_sel_nxt = 4'd1; // Add
                    lua_fetch_1 = 1'd1;
                    lua_custom_imm32 = offseta;

                    barrel_shift_data_sel_nxt = 2'd0;
                    barrel_shift_function_nxt   = 0; // LSL
                    barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                    lua_rn_reg = opcode == LUA_OP_SETTABUP ? UPVALS : BASE;
                end
            if ( itype == LUA_BRANCH )
                begin
                lua_inst_stage_reg = 'd1;
                saved_current_instruction_wen   = 1'd1;
                end
            if ( itype == LUA_FORPREP )
                begin
                // fetch loop initial value R(A)
                lua_inst_stage_reg = 'd1;
                saved_current_instruction_wen   = 1'd1;

                data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                address_sel_nxt = 4'd1; // alu out
                alu_out_sel_nxt = 4'd1; // Add
                lua_fetch_1 = 1'd1;
                lua_custom_imm32 = offseta;

                barrel_shift_data_sel_nxt = 2'd0;
                barrel_shift_function_nxt   = 0; // LSL
                barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                lua_rn_reg = BASE;
                end
            if ( itype == LUA_FORLOOP )
                begin
                // fetch loop index R(A)
                lua_inst_stage_reg = 'd1;
                saved_current_instruction_wen   = 1'd1;

                data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                address_sel_nxt = 4'd1; // alu out
                alu_out_sel_nxt = 4'd1; // Add
                lua_fetch_1 = 1'd1;
                lua_custom_imm32 = offseta;

                barrel_shift_data_sel_nxt = 2'd0;
                barrel_shift_function_nxt   = 0; // LSL
                barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                lua_rn_reg = BASE;
                end
            if ( itype == LUA_LEN )
                begin
                lua_inst_stage_reg = 'd1;
                saved_current_instruction_wen   = 1'd1;

                // fetch R(B), the value whose len we want to read
                data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                address_sel_nxt = 4'd1; // alu out
                alu_out_sel_nxt = 4'd1; // Add
                lua_fetch_1 = 1'd1;
                lua_custom_imm32 = {23'd0, lua_b_sel_nxt};

                barrel_shift_data_sel_nxt = 2'd0;
                barrel_shift_function_nxt   = 0; // LSL
                barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                lua_rn_reg = BASE;
                end
            if ( itype == LUA_CMP )
                begin
                lua_inst_stage_reg = 'd1;
                saved_current_instruction_wen   = 1'd1;

                // fetch RK(B)
                data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                address_sel_nxt = 4'd1; // alu out
                alu_out_sel_nxt = 4'd1; // Add
                lua_fetch_1 = 1'd1;
                lua_custom_imm32 = {24'd0, lua_bk_sel_nxt};

                barrel_shift_data_sel_nxt = 2'd0;
                barrel_shift_function_nxt   = 0; // LSL
                barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                lua_rn_reg = lua_bk_base;
                end
            if ( itype == LUA_SELF )
                begin
                lua_inst_stage_reg = 'd1;
                saved_current_instruction_wen   = 1'd1;

                // fetch R(B), which is used for the first part of this instruction
                data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                address_sel_nxt = 4'd1; // alu out
                alu_out_sel_nxt = 4'd1; // Add
                lua_fetch_1 = 1'd1;
                lua_custom_imm32 = {23'd0, lua_b_sel_nxt};

                barrel_shift_data_sel_nxt = 2'd0;
                barrel_shift_function_nxt   = 0; // LSL
                barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                lua_rn_reg = BASE;
                end
            if ( itype == LUA_LOADNIL )
                begin
                lua_inst_stage_reg = 'd1;
                saved_current_instruction_wen   = 1'd1;
                end
            if ( itype == LUA_LOADBOOL )
                begin
                lua_inst_stage_reg = 'd1;
                saved_current_instruction_wen   = 1'd1;
                end
            if ( itype == LUA_TEST )
                begin
                lua_inst_stage_reg = 'd1;
                saved_current_instruction_wen   = 1'd1;

                // load either R(A) (OP_TEST) or R(B) (OP_TESTSET)
                data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                address_sel_nxt = 4'd1; // alu out
                alu_out_sel_nxt = 4'd1; // Add
                lua_fetch_1 = 1'd1;
                lua_custom_imm32 = opcode == LUA_OP_TEST ? offseta : {23'd0, lua_b_sel_nxt};

                barrel_shift_data_sel_nxt = 2'd0;
                barrel_shift_function_nxt   = 0; // LSL
                barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                lua_rn_reg = BASE;
                end
            if ( itype == LUA_GET )
                begin
                // we need to go to exec stage to begin
                lua_inst_stage_reg = 'd1;
                saved_current_instruction_wen   = 1'd1;

                // fetch upval(B), the value we want to read
                data_access_exec_nxt = 1'd1; // indicate that its a data read or write,
                address_sel_nxt = 4'd1; // alu out
                alu_out_sel_nxt = 4'd1; // Add
                lua_fetch_1 = 1'd1;
                lua_custom_imm32 = {23'd0, lua_b_sel_nxt};

                barrel_shift_data_sel_nxt = 2'd0;
                barrel_shift_function_nxt   = 0; // LSL
                barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount

                lua_rn_reg = opcode == LUA_OP_GETTABLE ? BASE : UPVALS;
                end
            end


        // Load & Store instructions
        if ( mem_op )
            begin
            saved_current_instruction_wen   = 1'd1;
            pc_wen_nxt                      = 1'd0; // hold current PC value
            data_access_exec_nxt            = 1'd1; // indicate that its a data read or write,
                                                    // rather than an instruction fetch
            alu_out_sel_nxt                 = 4'd1; // Add

            if ( !instruction[23] )  // U: Subtract offset
                begin
                alu_cin_sel_nxt  = 2'd1; // cin = 1
                alu_not_sel_nxt  = 1'd1; // invert B
                end

            if ( store_op )
                begin
                write_data_wen_nxt = 1'd1;
                if ( itype == TRANS && instruction[22] )
                    byte_enable_sel_nxt = 2'd1;         // Save byte
                end

                // need to update the register holding the address ?
                // This is Rn bits [19:16]
            if ( mem_op_pre_indexed || mem_op_post_indexed )
                begin
                // Check is the load destination is the PC
                if ( rn_sel_nxt  == 4'd15 )
                    pc_sel_nxt = 3'd1;
                else
                    reg_bank_wen_nxt = decode ( rn_sel_nxt );
                end

                // if post-indexed, then use Rn rather than ALU output, as address
            if ( mem_op_post_indexed )
               address_sel_nxt = 4'd4; // Rn
            else
               address_sel_nxt = 4'd1; // alu out

            if ( instruction[25] && itype ==  TRANS )
                barrel_shift_data_sel_nxt = 2'd2; // Shift value from Rm register

            if ( itype == TRANS && instruction[25] && shift_imm != 5'd0 )
                begin
                barrel_shift_function_nxt   = instruction[6:5];
                barrel_shift_amount_sel_nxt = 2'd2; // imm_shift_amount
                end
            end

        if ( itype == BRANCH )
            begin
            pc_sel_nxt      = is_bxl ? 3'd0 : 3'd1; // alu_out
            address_sel_nxt = is_bxl ? 4'd3 : 4'd1; // alu_out
            alu_out_sel_nxt = 4'd1; // Add

            if ( instruction[24] ) // Link
                begin
                reg_bank_wen_nxt  = decode (4'd14);  // Save PC to LR
                reg_write_sel_nxt = 4'd1;            // pc - 32'd4
                end
            end

        if ( itype == MTRANS )
            begin
            saved_current_instruction_wen   = 1'd1;
            pc_wen_nxt                      = 1'd0; // hold current PC value
            data_access_exec_nxt            = 1'd1; // indicate that its a data read or write,
                                                    // rather than an instruction fetch
            alu_out_sel_nxt                 = 4'd1; // Add
            mtrans_r15_nxt                  = instruction[15];  // load or save r15 ?
            base_address_wen_nxt            = 1'd1; // Save the value of the register used for the base address,
                                                    // in case of a data abort, and need to restore the value

            // The spec says -
            // If the instruction would have overwritten the base with data
            // (that is, it has the base in the transfer list), the overwriting is prevented.
            // This is true even when the abort occurs after the base word gets loaded
            restore_base_address_nxt        = instruction[20] &&
                                                (instruction[{1'b0,instruction[19:16]}]);

            // Increment or Decrement
            if ( instruction[23] ) // increment
                begin
                if ( instruction[24] )    // increment before
                    address_sel_nxt = 4'd7; // Rn + 4
                else
                    address_sel_nxt = 4'd4; // Rn
                end
            else // decrement
                begin
                alu_cin_sel_nxt  = 2'd1; // cin = 1
                alu_not_sel_nxt  = 1'd1; // invert B
                if ( !instruction[24] )    // decrement after
                    address_sel_nxt  = 4'd6; // alu out + 4
                else
                    address_sel_nxt  = 4'd1; // alu out
                end

            // Load or store ?
            if ( !instruction[20] )  // Store
                write_data_wen_nxt = 1'd1;

            // LDM: load into user mode registers, when in priviledged mode
            // DOnt use mtrans_r15 here because its not loaded yet
            if ( {instruction[22:20],instruction[15]} == 4'b1010 )
                user_mode_regs_load_nxt = 1'd1;

            // SDM: store the user mode registers, when in priviledged mode
            if ( {instruction[22:20]} == 3'b100 )
                o_user_mode_regs_store_nxt = 1'd1;

            // update the base register ?
            if ( instruction[21] )  // the W bit
                reg_bank_wen_nxt  = decode (rn_sel_nxt);
            end


        if ( itype == MULT )
            begin
            multiply_function_nxt[0]        = 1'd1; // set enable
                                                    // some bits can be changed just below
            saved_current_instruction_wen   = 1'd1; // Save the Multiply instruction to
                                                    // refer back to later
            pc_wen_nxt                      = 1'd0; // hold current PC value

            if ( instruction[21] )
                multiply_function_nxt[1]    = 1'd1; // accumulate
            end


        // swp - do read part first
        if ( itype == SWAP )
            begin
            saved_current_instruction_wen   = 1'd1;
            pc_wen_nxt                      = 1'd0; // hold current PC value
            data_access_exec_nxt            = 1'd1; // indicate that its a data read or write,
                                                    // rather than an instruction fetch
            barrel_shift_data_sel_nxt       = 2'd2; // Shift value from Rm register
            address_sel_nxt                 = 4'd4; // Rn
            exclusive_exec_nxt              = 1'd1; // signal an exclusive access
            end


        // mcr & mrc - takes two cycles
        if ( itype == CORTRANS && !und_request )
            begin
            saved_current_instruction_wen   = 1'd1;
            pc_wen_nxt                      = 1'd0; // hold current PC value
            address_sel_nxt                 = 4'd3; // pc  (not pc + 4)

            if ( instruction[20] ) // MRC
                copro_operation_nxt         = 2'd1;  // Register transfer from Co-Processor
            else // MCR
                begin
                 // Don't enable operation to Co-Processor until next period
                 // So it gets the Rd value from the execution stage at the same time
                copro_operation_nxt      = 2'd0;
                copro_write_data_wen_nxt = 1'd1;  // Rd register value to co-processor
                end
            end


        if ( itype == SWI || und_request )
            begin
            // save address of next instruction to Supervisor Mode LR
            reg_write_sel_nxt               = 4'd1;            // pc -4
            reg_bank_wen_nxt                = decode (4'd14);  // LR

            address_sel_nxt                 = 4'd2;            // interrupt_vector
            pc_sel_nxt                      = 3'd2;            // interrupt_vector

            status_bits_mode_nxt            = interrupt_mode;  // e.g. Supervisor mode
            status_bits_mode_wen_nxt        = 1'd1;

            // disable normal interrupts
            status_bits_irq_mask_nxt        = 1'd1;
            status_bits_irq_mask_wen_nxt    = 1'd1;
            end


        if ( regop_set_flags )
            begin
            status_bits_flags_wen_nxt = 1'd1;

            // If <Rd> is r15, the ALU output is copied to the Status Bits.
            // Not allowed to use r15 for mul or lma instructions
            if ( instruction[15:12] == 4'd15 )
                begin
                status_bits_sel_nxt       = 3'd1; // alu out

                // Priviledged mode? Then also update the other status bits
                if ( i_execute_status_bits[1:0] != USR )
                    begin
                    status_bits_mode_wen_nxt      = 1'd1;
                    status_bits_irq_mask_wen_nxt  = 1'd1;
                    status_bits_firq_mask_wen_nxt = 1'd1;
                    end
                end
            end

        end

    // Handle asynchronous interrupts.
    // interrupts are processed only during execution states
    // multicycle instructions must complete before the interrupt starts
    // SWI, Address Exception and Undefined Instruction interrupts are only executed if the
    // instruction that causes the interrupt is conditionally executed so
    // its not handled here
    if ( instruction_valid && interrupt &&  next_interrupt != 3'd6 )
        begin
        // Save the interrupt causing instruction to refer back to later
        // This also saves the instruction abort vma and status, in the case of an
        // instruction abort interrupt
        saved_current_instruction_wen   = 1'd1;

        // save address of next instruction to Supervisor Mode LR
        // Address Exception ?
        if ( next_interrupt == 3'd4 )
            reg_write_sel_nxt               = 4'd7;            // pc
        else
            reg_write_sel_nxt               = 4'd1;            // pc -4

        reg_bank_wen_nxt                = decode (4'd14);  // LR

        address_sel_nxt                 = 4'd2;            // interrupt_vector
        pc_sel_nxt                      = 3'd2;            // interrupt_vector

        status_bits_mode_nxt            = interrupt_mode;  // e.g. Supervisor mode
        status_bits_mode_wen_nxt        = 1'd1;

        // disable normal interrupts
        status_bits_irq_mask_nxt        = 1'd1;
        status_bits_irq_mask_wen_nxt    = 1'd1;

        // disable fast interrupts
        if ( next_interrupt == 3'd2 ) // FIRQ
            begin
            status_bits_firq_mask_nxt        = 1'd1;
            status_bits_firq_mask_wen_nxt    = 1'd1;
            end
        end


    // previous instruction was either ldr or sdr
    // if it is currently executing in the execute stage do the following
    if ( control_state == MEM_WAIT1 )
        begin
        // Save the next instruction to execute later
        // Do this even if this instruction does not execute because of Condition
        pre_fetch_instruction_wen   = 1'd1;

        if ( instruction_execute ) // conditional execution state
            begin
            address_sel_nxt             = 4'd3; // pc  (not pc + 4)
            pc_wen_nxt                  = 1'd0; // hold current PC value
            end
        end

    if ( control_state == LUA_LOAD1 )
        begin
        // Save the next instruction to execute later
        // Do this even if this instruction does not execute because of Condition
        lua_fetch_1 = 1'd0;
        data_access_exec_nxt        = 1'd1;
        address_sel_nxt             = 4'd3; // pc  (not pc + 4)
        pc_wen_nxt                  = 1'd0; // hold current PC value
        end

    if ( control_state == LUA_LOAD2 )
        begin
        lua_fetch_2 = 1'd0;

        data_access_exec_nxt        = 1'd1;
        address_sel_nxt             = 4'd3; // pc  (not pc + 4)
        pc_wen_nxt                  = 1'd0; // hold current PC value
        end

    if ( control_state == LUA_STOREA )
        begin

        lua_store_a = 1'd0;

        data_access_exec_nxt        = 1'd1;
        address_sel_nxt             = 4'd3; // pc  (not pc + 4)
        pc_wen_nxt                  = 1'd0; // hold current PC value
        end

    if ( control_state == LUA_LOAD1F )
        begin

        if ( !dabt )  // dont load data there is an abort on the data read
            begin
            address_sel_nxt = 4'd3; // pc  (not pc + 4)
            pc_wen_nxt = 1'd0;
            // B loads to r1
            reg_write_sel_nxt = 4'd4;
            reg_bank_wen_nxt = decode (4'd1);
            end
        end

    if ( control_state == LUA_LOAD2F )
        begin

        if ( !dabt )  // dont load data there is an abort on the data read
            begin
            address_sel_nxt = 4'd3; // pc  (not pc + 4)
            pc_wen_nxt = 1'd0;
            // C loads to r2
            reg_write_sel_nxt = 4'd4;
            reg_bank_wen_nxt = decode (4'd2);
            end
        end

    // completion of load operation
    if ( control_state == MEM_WAIT2 && load_op )
        begin
        barrel_shift_data_sel_nxt   = 2'd1;  // load word from memory
        barrel_shift_amount_sel_nxt = 2'd3;  // shift by address[1:0] x 8

        // shift needed
        if ( i_execute_address[1:0] != 2'd0 )
            barrel_shift_function_nxt = ROR;

        // load a byte
        if ( itype == TRANS && instruction[22] )
            alu_out_sel_nxt             = 4'd3;  // zero_extend8

        if ( !dabt )  // dont load data there is an abort on the data read
            begin
            // Check if the load destination is the PC
            if (instruction[15:12]  == 4'd15)
                begin
                pc_sel_nxt      = 3'd1; // alu_out
                address_sel_nxt = 4'd1; // alu_out
                end
            else
                reg_bank_wen_nxt = decode (instruction[15:12]);
            end
        end


    // second cycle of multiple load or store
    if ( control_state == MTRANS_EXEC1 )
        begin
        // Save the next instruction to execute later
        // Do this even if this instruction does not execute because of Condition
        pre_fetch_instruction_wen   = 1'd1;

        if ( instruction_execute ) // conditional execution state
            begin
            address_sel_nxt             = 4'd5;  // o_address
            pc_wen_nxt                  = 1'd0;  // hold current PC value
            data_access_exec_nxt        = 1'd1;  // indicate that its a data read or write,
                                                 // rather than an instruction fetch

            if ( !instruction[20] ) // Store
                write_data_wen_nxt = 1'd1;

            // LDM: load into user mode registers, when in priviledged mode
            if ( {instruction[22:20],mtrans_r15} == 4'b1010 )
                user_mode_regs_load_nxt = 1'd1;

            // SDM: store the user mode registers, when in priviledged mode
            if ( {instruction[22:20]} == 3'b100 )
                o_user_mode_regs_store_nxt = 1'd1;
            end
        end


        // third cycle of multiple load or store
    if ( control_state == MTRANS_EXEC2 )
        begin
        address_sel_nxt             = 4'd5;  // o_address
        pc_wen_nxt                  = 1'd0;  // hold current PC value
        data_access_exec_nxt        = 1'd1;  // indicate that its a data read or write,
                                             // rather than an instruction fetch
        barrel_shift_data_sel_nxt   = 2'd1;  // load word from memory

        // Load or Store
        if ( instruction[20] ) // Load
            begin
            // Can never be loading the PC in this state, as the PC is always
            // the last register in the set to be loaded
            if ( !dabt )
                reg_bank_wen_nxt = decode (mtrans_reg_d2);
            end
        else // Store
            write_data_wen_nxt = 1'd1;

        // LDM: load into user mode registers, when in priviledged mode
        if ( {instruction[22:20],mtrans_r15} == 4'b1010 )
            user_mode_regs_load_nxt = 1'd1;

        // SDM: store the user mode registers, when in priviledged mode
        if ( {instruction[22:20]} == 3'b100 )
            o_user_mode_regs_store_nxt = 1'd1;
        end


        // second or fourth cycle of multiple load or store
    if ( control_state == MTRANS_EXEC3 && instruction_execute )
        begin
        address_sel_nxt             = 4'd3; // pc  (not pc + 4)
        pc_wen_nxt                  = 1'd0;  // hold current PC value
        barrel_shift_data_sel_nxt   = 2'd1;  // load word from memory

        // Can never be loading the PC in this state, as the PC is always
        // the last register in the set to be loaded
        if ( instruction[20] && !dabt ) // Load
            reg_bank_wen_nxt = decode (mtrans_reg_d2);

        // LDM: load into user mode registers, when in priviledged mode
        if ( {instruction[22:20],mtrans_r15} == 4'b1010 )
            user_mode_regs_load_nxt = 1'd1;

        // SDM: store the user mode registers, when in priviledged mode
        if ( {instruction[22:20]} == 3'b100 )
            o_user_mode_regs_store_nxt = 1'd1;
       end

    // state is used for LMD/STM of a single register
    if ( control_state == MTRANS_EXEC3B && instruction_execute )
        begin
        // Save the next instruction to execute later
        // Do this even if this instruction does not execute because of Condition
        pre_fetch_instruction_wen   = 1'd1;

        address_sel_nxt             = 4'd3;  // pc  (not pc + 4)
        pc_wen_nxt                  = 1'd0;  // hold current PC value

        // LDM: load into user mode registers, when in priviledged mode
        if ( {instruction[22:20],mtrans_r15} == 4'b1010 )
            user_mode_regs_load_nxt = 1'd1;

        // SDM: store the user mode registers, when in priviledged mode
        if ( {instruction[22:20]} == 3'b100 )
            o_user_mode_regs_store_nxt = 1'd1;
        end

    if ( control_state == MTRANS_EXEC4 )
        begin
        barrel_shift_data_sel_nxt   = 2'd1;  // load word from memory

        if ( instruction[20] ) // Load
            begin
            if (!dabt) // dont overwrite registers or status if theres a data abort
                begin
                if ( mtrans_reg_d2 == 4'd15 ) // load new value into PC
                    begin
                    address_sel_nxt = 4'd1; // alu_out - read instructions using new PC value
                    pc_sel_nxt      = 3'd1; // alu_out
                    pc_wen_nxt      = 1'd1; // write PC

                    // ldm with S bit and pc: the Status bits are updated
                    // Node this must be done only at the end
                    // so the register set is the set in the mode before it
                    // gets changed.
                    if ( instruction[22] )
                         begin
                         status_bits_sel_nxt           = 3'd1; // alu out
                         status_bits_flags_wen_nxt     = 1'd1;

                         // Can't change the mode or mask bits in User mode
                         if ( i_execute_status_bits[1:0] != USR )
                            begin
                            status_bits_mode_wen_nxt      = 1'd1;
                            status_bits_irq_mask_wen_nxt  = 1'd1;
                            status_bits_firq_mask_wen_nxt = 1'd1;
                            end
                         end
                    end
                else
                    begin
                    reg_bank_wen_nxt = decode (mtrans_reg_d2);
                    end
                end
            end

           // we have a data abort interrupt
        if ( dabt )
            begin
            pc_wen_nxt = 1'd0;  // hold current PC value
            end

        // LDM: load into user mode registers, when in priviledged mode
        if ( {instruction[22:20],mtrans_r15} == 4'b1010 )
            user_mode_regs_load_nxt = 1'd1;

        // SDM: store the user mode registers, when in priviledged mode
        if ( {instruction[22:20]} == 3'b100 )
            o_user_mode_regs_store_nxt = 1'd1;
        end


    // state is for when a data abort interrupt is triggered during an LDM
    if ( control_state == MTRANS5_ABORT )
        begin
        // Restore the Base Address, if the base register is included in the
        // list of registers being loaded
        if (restore_base_address) // LDM with base address in register list
            begin
            reg_write_sel_nxt = 4'd6;                        // write base_register
            reg_bank_wen_nxt  = decode ( instruction[19:16] ); // to Rn
            end
        end


        // Multiply or Multiply-Accumulate
    if ( control_state == MULT_PROC1 && instruction_execute )
        begin
        // Save the next instruction to execute later
        // Do this even if this instruction does not execute because of Condition
        pre_fetch_instruction_wen   = 1'd1;
        pc_wen_nxt                  = 1'd0;  // hold current PC value
        multiply_function_nxt       = o_multiply_function;
        end


        // Multiply or Multiply-Accumulate
        // Do multiplication
        // Wait for done or accumulate signal
    if ( control_state == MULT_PROC2 )
        begin
        // Save the next instruction to execute later
        // Do this even if this instruction does not execute because of Condition
        pc_wen_nxt              = 1'd0;  // hold current PC value
        address_sel_nxt         = 4'd3;  // pc  (not pc + 4)
        multiply_function_nxt   = o_multiply_function;
        end


    // Save RdLo
    // always last cycle of all multiply or multiply accumulate operations
    if ( control_state == MULT_STORE )
        begin
        reg_write_sel_nxt     = 4'd2; // multiply_out
        multiply_function_nxt = o_multiply_function;

        if ( itype == MULT ) // 32-bit
            reg_bank_wen_nxt      = decode (instruction[19:16]); // Rd
        else  // 64-bit / Long
            reg_bank_wen_nxt      = decode (instruction[15:12]); // RdLo

        if ( instruction[20] )  // the 'S' bit
            begin
            status_bits_sel_nxt       = 3'd4; // { multiply_flags, status_bits_flags[1:0] }
            status_bits_flags_wen_nxt = 1'd1;
            end
        end

        // Add lower 32 bits to multiplication product
    if ( control_state == MULT_ACCUMU )
        begin
        multiply_function_nxt = o_multiply_function;
        pc_wen_nxt            = 1'd0;  // hold current PC value
        address_sel_nxt       = 4'd3;  // pc  (not pc + 4)
        end

    // swp - do write request in 2nd cycle
    if ( control_state == SWAP_WRITE && instruction_execute )
        begin
        barrel_shift_data_sel_nxt       = 2'd2; // Shift value from Rm register
        address_sel_nxt                 = 4'd4; // Rn
        write_data_wen_nxt              = 1'd1;
        data_access_exec_nxt            = 1'd1; // indicate that its a data read or write,
                                                // rather than an instruction fetch

        if ( instruction[22] )
            byte_enable_sel_nxt = 2'd1;         // Save byte

        if ( instruction_execute )                         // conditional execution state
            pc_wen_nxt                  = 1'd0; // hold current PC value

        // Save the next instruction to execute later
        // Do this even if this instruction does not execute because of Condition
        pre_fetch_instruction_wen   = 1'd1;

        end


    // swp - receive read response in 3rd cycle
    if ( control_state == SWAP_WAIT1 )
        begin
        barrel_shift_data_sel_nxt   = 2'd1;  // load word from memory
        barrel_shift_amount_sel_nxt = 2'd3;  // shift by address[1:0] x 8

        // shift needed
        if ( i_execute_address[1:0] != 2'd0 )
            barrel_shift_function_nxt = ROR;

        if ( instruction_execute ) // conditional execution state
            begin
            address_sel_nxt             = 4'd3; // pc  (not pc + 4)
            pc_wen_nxt                  = 1'd0; // hold current PC value
            end

        // load a byte
        if ( instruction[22] )
            alu_out_sel_nxt = 4'd3;  // zero_extend8

        if ( !dabt )
            begin
            // Check is the load destination is the PC
            if ( instruction[15:12]  == 4'd15 )
                begin
                pc_sel_nxt      = 3'd1; // alu_out
                address_sel_nxt = 4'd1; // alu_out
                end
            else
                reg_bank_wen_nxt = decode (instruction[15:12]);
            end
        end

    // 1 cycle delay for Co-Processor Register access
    if ( control_state == COPRO_WAIT && instruction_execute )
        begin
        pre_fetch_instruction_wen = 1'd1;

        if ( instruction[20] ) // mrc instruction
            begin
            // Check is the load destination is the PC
            if ( instruction[15:12]  == 4'd15 )
                begin
                // If r15 is specified for <Rd>, the condition code flags are
                // updated instead of a general-purpose register.
                status_bits_sel_nxt           = 3'd3;  // i_copro_data
                status_bits_flags_wen_nxt     = 1'd1;

                // Can't change these in USR mode
                if ( i_execute_status_bits[1:0] != USR )
                   begin
                   status_bits_mode_wen_nxt      = 1'd1;
                   status_bits_irq_mask_wen_nxt  = 1'd1;
                   status_bits_firq_mask_wen_nxt = 1'd1;
                   end
                end
            else
                reg_bank_wen_nxt = decode (instruction[15:12]);

            reg_write_sel_nxt = 4'd5;     // i_copro_data
            end
        else // mcr instruction
            begin
            copro_operation_nxt      = 2'd2;  // Register transfer to Co-Processor
            end
        end


    // Have just changed the status_bits mode but this
    // creates a 1 cycle gap with the old mode
    // coming back from execute into instruction_decode
    // So squash that old mode value during this
    // cycle of the interrupt transition
    if ( control_state == INT_WAIT1 )
        status_bits_mode_nxt            = o_status_bits_mode;   // Supervisor mode

    end


// Speed up the long path from u_decode/o_read_data to u_register_bank/r8_firq
// This pre-encodes the firq_s3 signal thats used in u_register_bank
assign firq_not_user_mode_nxt = !user_mode_regs_load_nxt && status_bits_mode_nxt == FIRQ;


// ========================================================
// Next State Logic
// ========================================================

// this replicates the current value of the execute signal in the execute stage
assign instruction_execute = i_lua_mode || (conditional_execute ( o_condition, i_execute_status_bits[31:28] ));

assign instruction_valid = (control_state == EXECUTE || control_state == PRE_FETCH_EXEC ||
                            control_state == LUA_EXEC || control_state == LUA_DECODE ||

                            // chained multi-cycle lua loads
                            control_state == LUA_LOAD2F) || (i_lua_mode) ||
                     // when last instruction was multi-cycle instruction but did not execute
                     // because condition was false then act like you're in the execute state
                    (!instruction_execute && (control_state == PC_STALL1    ||
                                              control_state == MEM_WAIT1    ||
                                              control_state == COPRO_WAIT   ||
                                              control_state == SWAP_WRITE   ||
                                              control_state == MULT_PROC1   ||
                                              control_state == MTRANS_EXEC1 ||
                                              control_state == MTRANS_EXEC3 ||
                                              control_state == MTRANS_EXEC3B  ) );


 always @*
    begin
    // default is to hold the current state
    control_state_nxt = control_state;

    // Note: The order is important here
    if ( control_state == RST_WAIT1 )     control_state_nxt = RST_WAIT2;
    if ( control_state == RST_WAIT2 )     control_state_nxt = EXECUTE;
    if ( control_state == INT_WAIT1 )     control_state_nxt = INT_WAIT2;
    if ( control_state == INT_WAIT2 )     control_state_nxt = EXECUTE;
    if ( control_state == COPRO_WAIT )    control_state_nxt = PRE_FETCH_EXEC;
    if ( control_state == PC_STALL1 )     control_state_nxt = PC_STALL2;
    if ( control_state == PC_STALL2 )     control_state_nxt = EXECUTE;
    if ( control_state == SWAP_WRITE )    control_state_nxt = SWAP_WAIT1;
    if ( control_state == SWAP_WAIT1 )    control_state_nxt = SWAP_WAIT2;
    if ( control_state == MULT_STORE )    control_state_nxt = PRE_FETCH_EXEC;
    if ( control_state == MTRANS5_ABORT ) control_state_nxt = PRE_FETCH_EXEC;

    if ( control_state == MEM_WAIT1 )
        control_state_nxt = MEM_WAIT2;

    if ( control_state == MEM_WAIT2   ||
        control_state == SWAP_WAIT2    )
        begin
        if ( write_pc ) // writing to the PC!!
            control_state_nxt = PC_STALL1;
        else
            control_state_nxt = PRE_FETCH_EXEC;
        end

    if ( control_state == MTRANS_EXEC1 )
        begin
        if (mtrans_instruction_nxt[15:0] != 16'd0)
            control_state_nxt = MTRANS_EXEC2;
        else   // if the register list holds a single register
            control_state_nxt = MTRANS_EXEC3;
        end

        // Stay in State MTRANS_EXEC2 until the full list of registers to
        // load or store has been processed
    if ( control_state == MTRANS_EXEC2 && mtrans_num_registers == 5'd1 )
        control_state_nxt = MTRANS_EXEC3;

    if ( control_state == MTRANS_EXEC3 )     control_state_nxt = MTRANS_EXEC4;

    if ( control_state == MTRANS_EXEC3B )    control_state_nxt = MTRANS_EXEC4;

    if ( control_state == MTRANS_EXEC4  )
        begin
        if ( dabt ) // data abort
            control_state_nxt = MTRANS5_ABORT;
        else if (write_pc) // writing to the PC!!
            control_state_nxt = PC_STALL1;
        else
            control_state_nxt = PRE_FETCH_EXEC;
        end

    if ( control_state == MULT_PROC1 )
        begin
        if (!instruction_execute)
            control_state_nxt = PRE_FETCH_EXEC;
        else
            control_state_nxt = MULT_PROC2;
        end

    if ( control_state == MULT_PROC2 )
        begin
        if ( i_multiply_done )
            if      ( o_multiply_function[1] )  // Accumulate ?
                control_state_nxt = MULT_ACCUMU;
            else
                control_state_nxt = MULT_STORE;
        end


    if ( control_state == MULT_ACCUMU )
        begin
        control_state_nxt = MULT_STORE;
        end

    // ordering matters
    if ( lua_mode_nxt )
        begin
        control_state_nxt = LUA_DECODE;

        if ( control_state == LUA_PC_STALL2) control_state_nxt = LUA_EXEC;
        if ( control_state == LUA_STOREA2) control_state_nxt = PRE_FETCH_EXEC;
        if ( control_state == LUA_LOAD2F) control_state_nxt = LUA_EXEC;
        if ( control_state == LUA_LOAD1F) control_state_nxt = LUA_EXEC;
        if ( lua_inst_stage_reg != 'd0 ) control_state_nxt = LUA_EXEC;

        if ( lua_store_a )
            control_state_nxt = LUA_STOREA;
        if ( lua_fetch_1 )
            control_state_nxt = LUA_LOAD1;
        if ( lua_fetch_2 ) 
            control_state_nxt = LUA_LOAD2;
        if ( lua_prefetch_1 )
            control_state_nxt = PRE_FETCH_EXEC;
        if ( lua_pc_stall )
            control_state_nxt = LUA_PC_STALL;

        if ( control_state == LUA_PC_STALL ) control_state_nxt = LUA_PC_STALL2;
        if ( control_state == LUA_STOREA ) control_state_nxt = LUA_STOREA2;
        if ( control_state == LUA_LOAD1 ) control_state_nxt = LUA_LOAD1F;
        if ( control_state == LUA_LOAD2 ) control_state_nxt = LUA_LOAD2F;

        if ( interrupt ) control_state_nxt = INT_WAIT1;
        end

    // This should come at the end, so that conditional execution works
    // correctly
    else if ( instruction_valid )
        begin
        // default is to stay in execute state, or to move into this
        // state from a conditional execute state
        control_state_nxt = control_state;

        if ( mem_op )  // load or store word or byte
             control_state_nxt = MEM_WAIT1;
        if ( write_pc )
             control_state_nxt = PC_STALL1;
        if ( itype == MTRANS )
            begin
            if ( mtrans_num_registers != 5'd0 )
                begin
                // check for LDM/STM of a single register
                if ( mtrans_num_registers == 5'd1 )
                    control_state_nxt = MTRANS_EXEC3B;
                else
                    control_state_nxt = MTRANS_EXEC1;
                end
            else
                control_state_nxt = MTRANS_EXEC3;
            end

        if ( itype == MULT )
                control_state_nxt = MULT_PROC1;

        if ( itype == SWAP )
                control_state_nxt = SWAP_WRITE;

        if ( itype == CORTRANS && !und_request )
                control_state_nxt = COPRO_WAIT;

         // interrupt overrides everything else so its last
        if ( interrupt )
                control_state_nxt = INT_WAIT1;
        end
    end


// ========================================================
// Register Update
// ========================================================
always @ ( posedge i_clk )
    if (!i_fetch_stall)
        begin
        o_read_data                 <= i_read_data;
        o_read_data_alignment       <= {i_execute_address[1:0], 3'd0};
        abt_address_reg             <= i_execute_address;
        iabt_reg                    <= i_iabt;
        adex_reg                    <= i_adex;
        abt_status_reg              <= i_abt_status;
        o_status_bits_mode          <= status_bits_mode_nxt;
        o_status_bits_irq_mask      <= status_bits_irq_mask_nxt;
        o_status_bits_firq_mask     <= status_bits_firq_mask_nxt;
        o_imm32                     <= imm32_nxt;
        o_imm_shift_amount          <= imm_shift_amount_nxt;
        o_shift_imm_zero            <= shift_imm_zero_nxt;

                                        // when have an interrupt, execute the interrupt operation
                                        // unconditionally in the execute stage
                                        // ensures that status_bits register gets updated correctly
                                        // Likewise when in middle of multi-cycle instructions
                                        // execute them unconditionally
        o_condition                 <= instruction_valid && !interrupt ? condition_nxt : AL;
        o_exclusive_exec            <= exclusive_exec_nxt;
        o_data_access_exec          <= data_access_exec_nxt;

        o_rm_sel                    <= rm_sel_nxt;
        o_rds_sel                   <= rds_sel_nxt;
        o_rn_sel                    <= rn_sel_nxt;
        o_barrel_shift_amount_sel   <= barrel_shift_amount_sel_nxt;
        o_barrel_shift_data_sel     <= barrel_shift_data_sel_nxt;
        o_barrel_shift_function     <= barrel_shift_function_nxt;
        o_alu_function              <= alu_function_nxt;
        o_multiply_function         <= multiply_function_nxt;
        o_interrupt_vector_sel      <= next_interrupt;
        o_address_sel               <= address_sel_nxt;
        o_pc_sel                    <= pc_sel_nxt;
        o_byte_enable_sel           <= byte_enable_sel_nxt;
        o_status_bits_sel           <= status_bits_sel_nxt;
        o_reg_write_sel             <= reg_write_sel_nxt;
        o_user_mode_regs_load       <= user_mode_regs_load_nxt;
        o_firq_not_user_mode        <= firq_not_user_mode_nxt;
        o_write_data_update_wen     <= write_data_update_wen_nxt;
        o_write_data_wen            <= write_data_wen_nxt;
        o_base_address_wen          <= base_address_wen_nxt;
        o_pc_wen                    <= pc_wen_nxt;
        o_reg_bank_wen              <= reg_bank_wen_nxt;
        o_status_bits_flags_wen     <= status_bits_flags_wen_nxt;
        o_status_bits_mode_wen      <= status_bits_mode_wen_nxt;
        o_status_bits_irq_mask_wen  <= status_bits_irq_mask_wen_nxt;
        o_status_bits_firq_mask_wen <= status_bits_firq_mask_wen_nxt;

        o_copro_opcode1             <= instruction[23:21];
        o_copro_opcode2             <= instruction[7:5];
        o_copro_crn                 <= instruction[19:16];
        o_copro_crm                 <= instruction[3:0];
        o_copro_num                 <= instruction[11:8];
        o_copro_operation           <= copro_operation_nxt;
        o_copro_write_data_wen      <= copro_write_data_wen_nxt;
        mtrans_r15                  <= mtrans_r15_nxt;
        restore_base_address        <= restore_base_address_nxt;

        o_prefetch_instruction      <= control_state == LUA_DECODE || (i_lua_mode && control_state == PRE_FETCH_EXEC);
        o_dbg_instruction_address   <= instruction;
        o_control_state             <= control_state;
        control_state               <= control_state_nxt;
        mtrans_reg_d1               <= mtrans_reg;
        mtrans_reg_d2               <= mtrans_reg_d1;

        o_lua_mode                  <= lua_mode_nxt;
        o_lua_inst_stage_nxt        <= lua_inst_stage_reg;
        lua_scratch_reg             <= lua_scratch_reg_wen ? i_read_data : lua_scratch_reg;
        end



always @ ( posedge i_clk )
    if ( !i_fetch_stall )
        begin
        // sometimes this is a pre-fetch instruction
        // e.g. two ldr instructions in a row. The second ldr will be saved
        // to the pre-fetch instruction register
        // then when its decoded, a copy is saved to the saved_current_instruction
        // register
        if (itype == MTRANS)
            begin
            saved_current_instruction              <= mtrans_instruction_nxt;
            saved_current_instruction_iabt         <= instruction_iabt;
            saved_current_instruction_adex         <= instruction_adex;
            saved_current_instruction_address      <= instruction_address;
            saved_current_instruction_iabt_status  <= instruction_iabt_status;
            end
        else if (saved_current_instruction_wen)
            begin
            saved_current_instruction              <= instruction;
            saved_current_instruction_iabt         <= instruction_iabt;
            saved_current_instruction_adex         <= instruction_adex;
            saved_current_instruction_address      <= instruction_address;
            saved_current_instruction_iabt_status  <= instruction_iabt_status;
            end

        if      (pre_fetch_instruction_wen)
            begin
            pre_fetch_instruction                  <= o_read_data;
            pre_fetch_instruction_iabt             <= iabt_reg;
            pre_fetch_instruction_adex             <= adex_reg;
            pre_fetch_instruction_address          <= abt_address_reg;
            pre_fetch_instruction_iabt_status      <= abt_status_reg;
            end
        end



always @ ( posedge i_clk )
    if ( !i_fetch_stall )
        begin
        irq   <= i_irq;
        firq  <= i_firq;

        if ( control_state == INT_WAIT1 && o_status_bits_mode == SVC )
            begin
            dabt_reg  <= 1'd0;
            end
        else
            begin
            dabt_reg  <= dabt_reg || i_dabt;
            end

        dabt_reg_d1  <= dabt_reg;
        end

assign dabt = dabt_reg || i_dabt;


// ========================================================
// Decompiler for debugging core - not synthesizable
// ========================================================

endmodule



//////////////////////////////////////////////////////////////////
//                                                              //
//  Parameters file for Amber 2 Core                            //
//                                                              //
//  This file is part of the Amber project                      //
//  http://www.opencores.org/project,amber                      //
//                                                              //
//  Description                                                 //
//  Holds general parameters that are used is several core      //
//  modules                                                     //
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


// Instruction Types
localparam [4:0]    REGOP       = 5'h0, // Data processing
                    MULT        = 5'h1, // Multiply
                    SWAP        = 5'h2, // Single Data Swap
                    TRANS       = 5'h3, // Single data transfer
                    MTRANS      = 5'h4, // Multi-word data transfer
                    BRANCH      = 5'h5, // Branch
                    CODTRANS    = 5'h6, // Co-processor data transfer
                    COREGOP     = 5'h7, // Co-processor data operation
                    CORTRANS    = 5'h8, // Co-processor register transfer
                    SWI         = 5'h9, // software interrupt

                    LUA_MOVE    = 5'ha,
                    LUA_LOADBOOL= 5'hb,
                    LUA_GET     = 5'hc,
                    LUA_SET     = 5'hd,
                    LUA_TABLE   = 5'he,
                    LUA_SELF    = 5'hf,
                    LUA_LEN     = 5'h10,
                    LUA_ALU2    = 5'h11,
                    LUA_CONCAT  = 5'h12,
                    LUA_BRANCH  = 5'h13,
                    LUA_CMP     = 5'h14,
                    LUA_CALL    = 5'h15,
                    LUA_LOOP    = 5'h16,
                    LUA_SETLIST = 5'h17,
                    LUA_CLOSURE = 5'h18,
                    LUA_FORLOOP = 5'h19,
                    LUA_FORPREP = 5'h1a,
                    LUA_RETURN  = 5'h1b,
                    LUA_SET2    = 5'h1c,
                    LUA_LOADNIL = 5'h1d,
                    LUA_TEST    = 5'h1e,
                    LUA_UNKNOWN = 5'h1f;

// ARM register <-> Lua state mapping
localparam [3:0] PROTOS    = 4'd6, // technically our parent functions closures
                 SCRATCH   = 4'd7,
                 TOP       = 4'h8,
                 BASE      = 4'h9,
                 UPVALS    = 4'ha,
                 CONSTANTS = 4'hb,
                 CALLINFO  = 4'hc,
                 FRAMES    = 4'hd;

/*
    saved activation frame (0x20 byte aligned)
    0x00: func, position of the closure on the stack
    0x04: base we had when calling
    0x08: top we had when calling
    0x0c: upvals we had when calling
    0x10: protos we had when calling
    0x14: constants we had when calling
    0x18: pc we return to
*/

// Offsets into lua proto
localparam [3:0] PROTO_OFF_INSTRUCTION  = 4'd0,
                 PROTO_OFF_UPVALS       = 4'd1,
                 PROTO_OFF_MAXSTACKSIZE = 4'd2,
                 PROTO_OFF_CONSTANTS    = 4'd3,
                 PROTO_OFF_PROTOS       = 4'd4,
                 PROTO_OFF_MAXUPVALS    = 4'd5;

// Opcodes
localparam [5:0] AND = 6'h0,        // Logical AND
                 EOR = 6'h1,        // Logical Exclusive OR
                 SUB = 6'h2,        // Subtract
                 RSB = 6'h3,        // Reverse Subtract
                 ADD = 6'h4,        // Add
                 ADC = 6'h5,        // Add with Carry
                 SBC = 6'h6,        // Subtract with Carry
                 RSC = 6'h7,        // Reverse Subtract with Carry
                 TST = 6'h8,        // Test  (using AND operator)
                 TEQ = 6'h9,        // Test Equivalence (using EOR operator)
                 CMP = 6'ha,       // Compare (using Subtract operator)
                 CMN = 6'hb,       // Compare Negated
                 ORR = 6'hc,       // Logical OR
                 MOV = 6'hd,       // Move
                 BIC = 6'he,       // Bit Clear (using AND & NOT operators)
                 MVN = 6'hf;       // Move NOT

// Lua opcodes
localparam [5:0] LUA_OP_MOVE     = 6'h0, 
                 LUA_OP_LOADK    = 6'h1,
                 LUA_OP_LOADKX   = 6'h2,
                 LUA_OP_LOADBOOL = 6'h3,
                 LUA_OP_LOADNIL  = 6'h4,
                 LUA_OP_GETUPVAL = 6'h5,
                 LUA_OP_GETTABUP = 6'h6,
                 LUA_OP_GETTABLE = 6'h7,
                 LUA_OP_SETTABUP = 6'h8,
                 LUA_OP_SETUPVAL = 6'h9,
                 LUA_OP_SETTABLE = 6'ha,
                 LUA_OP_NEWTABLE = 6'hb,
                 LUA_OP_SELF     = 6'hc,
                 LUA_OP_ADD      = 6'hd,
                 LUA_OP_SUB      = 6'he,
                 LUA_OP_MUL      = 6'hf,
                 LUA_OP_MOD      = 6'h10,
                 LUA_OP_POW      = 6'h11,
                 LUA_OP_DIV      = 6'h12,
                 LUA_OP_IDIV     = 6'h13,
                 LUA_OP_BAND     = 6'h14,
                 LUA_OP_BOR      = 6'h15,
                 LUA_OP_BXOR     = 6'h16,
                 LUA_OP_SHL      = 6'h17,
                 LUA_OP_SHR      = 6'h18,
                 LUA_OP_UNM      = 6'h19,
                 LUA_OP_BNOT     = 6'h1a,
                 LUA_OP_NOT      = 6'h1b,
                 LUA_OP_LEN      = 6'h1c,
                 LUA_OP_CONCAT   = 6'h1d,
                 LUA_OP_JMP      = 6'h1e,
                 LUA_OP_EQ       = 6'h1f,
                 LUA_OP_LT       = 6'h20,
                 LUA_OP_LE       = 6'h21,
                 LUA_OP_TEST     = 6'h22,
                 LUA_OP_TESTSET  = 6'h23,
                 LUA_OP_CALL     = 6'h24,
                 LUA_OP_TAILCALL = 6'h25,
                 LUA_OP_RETURN   = 6'h26,
                 LUA_OP_FORLOOP  = 6'h27,
                 LUA_OP_FORPREP  = 6'h28,
                 LUA_OP_TFORCALL = 6'h29,
                 LUA_OP_TFORLOOP = 6'h2a,
                 LUA_OP_SETLIST  = 6'h2b,
                 LUA_OP_CLOSURE  = 6'h2c,
                 LUA_OP_VARARG   = 6'h2d,
                 LUA_OP_EXTRAARG = 6'h2e;


// Condition Encoding
localparam [3:0] EQ  = 4'h0,        // Equal            / Z set
                 NE  = 4'h1,        // Not equal        / Z clear
                 CS  = 4'h2,        // Carry set        / C set
                 CC  = 4'h3,        // Carry clear      / C clear
                 MI  = 4'h4,        // Minus            / N set
                 PL  = 4'h5,        // Plus             / N clear
                 VS  = 4'h6,        // Overflow         / V set
                 VC  = 4'h7,        // No overflow      / V clear
                 HI  = 4'h8,        // Unsigned higher  / C set and Z clear
                 LS  = 4'h9,        // Unsigned lower
                                    // or same          / C clear or Z set
                 GE  = 4'ha,        // Signed greater 
                                    // than or equal    / N == V
                 LT  = 4'hb,        // Signed less than / N != V
                 GT  = 4'hc,        // Signed greater
                                    // than             / Z == 0, N == V
                 LE  = 4'hd,        // Signed less than
                                    // or equal         / Z == 1, N != V
                 AL  = 4'he,        // Always
                 NV  = 4'hf;        // Never

// Any instruction with a condition field of 0b1111 is UNPREDICTABLE.                
                
// Shift Types
localparam [1:0] LSL = 2'h0,
                 LSR = 2'h1,
                 ASR = 2'h2,
                 RRX = 2'h3,
                 ROR = 2'h3; 
 
// Modes
localparam [1:0] SVC  =  2'b11,  // Supervisor
                 IRQ  =  2'b10,  // Interrupt
                 FIRQ =  2'b01,  // Fast Interrupt
                 USR  =  2'b00;  // User

// One-Hot Mode encodings
localparam [5:0] OH_USR  = 0,
                 OH_IRQ  = 1,
                 OH_FIRQ = 2,
                 OH_SVC  = 3;



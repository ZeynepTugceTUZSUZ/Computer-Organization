//--------------------------------------------------------------
// mips.sv
// David_Harris@hmc.edu and Sarah_Harris@hmc.edu 23 October 2005
// Updated to SystemVerilog dmh 12 November 2010
// Single-cycle MIPS processor
//--------------------------------------------------------------
//
// TODO SUMMARY: You need to add support for the ori and bne instructions.
//   Changes are needed in 4 modules: mips, controller, maindec, aludec, datapath.
//   Every location requiring a change is marked with "// TODO".
//   Read all TODOs before you start coding — some changes ripple across modules.
//
// HINTS:
//   - ori uses zero-extension (not sign-extension) of the 16-bit immediate.
//     Think about how alusrc must change to select between register, sign-extended
//     immediate, and zero-extended immediate. What width should alusrc be?
//   - bne branches when the two registers are NOT equal. Think about how pcsrc
//     is currently computed for beq, and what needs to change.
//   - ori performs a bitwise OR. You need a new aluop encoding to tell the ALU
//     decoder to produce the OR control signal (3'b001).
//--------------------------------------------------------------

// files needed for simulation:
//  mipstest.sv
//  mipstop.sv
//  mipsmem.sv
//  mips.sv
//  mipsparts.sv
//  alu.sv

// single-cycle MIPS processor
module mips(input  logic        clk, reset,
            output logic [31:0] pc,
            input  logic [31:0] instr,
            output logic        memwrite,
            output logic [31:0] aluout, writedata,
            input  logic [31:0] readdata);

  logic        memtoreg, branch,pcsrc, zero,
               regdst, regwrite, jump;

  logic [1:0] alusrc;
  // TODO: (1) Declare any new control signals needed for bne.
  // TODO: (2) alusrc is currently 1 bit. ori needs a third mux input
  //           (zero-extended immediate). Consider widening alusrc to [1:0].
  logic [2:0]  alucontrol;

  controller c(instr[31:26], instr[5:0], zero,
               memtoreg, memwrite, pcsrc,
               alusrc, regdst, regwrite, jump,
               alucontrol);
  datapath dp(clk, reset, memtoreg, pcsrc,
              alusrc, regdst, regwrite, jump,
              alucontrol,
              zero, pc, instr,
              aluout, writedata, readdata);
endmodule

module controller(input  logic [5:0] op, funct,
                  input  logic       zero,
                  output logic       memtoreg, memwrite,
                  output logic       pcsrc,
                  output logic       [1:0] alusrc,
                  // TODO: Update alusrc width if you changed it above.
                  output logic       regdst, regwrite,
                  output logic       jump,
                  output logic [2:0] alucontrol);

  logic [1:0] aluop;
  logic       branch;
  // TODO: Declare a bne signal.
  logic bne;

  maindec md(op, memtoreg, memwrite, branch, bne, alusrc, regdst, regwrite, jump,aluop);
  // TODO: Pass the bne signal to/from maindec.

  aludec  ad(funct, aluop, alucontrol);

  // assign pcsrc = branch & zero;
  // TODO: Modify pcsrc so that it is also asserted when bne is active
  //       and zero is NOT set.  (beq branches when equal; bne branches
  //       when not equal.)
  assign pcsrc = (branch & zero) | (bne & ~zero);
endmodule

module maindec(input  logic [5:0] op,
               output logic       memtoreg, memwrite,
               output logic       branch, bne,
               // TODO: (1) Add a bne output port.
               // TODO: (2) Update alusrc width if needed.
               output logic [1:0] alusrc,
               output logic       regdst, regwrite,
               output logic       jump,
               output logic [1:0] aluop);

  logic [10:0] controls;
  // TODO: The controls vector must grow to hold the new fields (bne, wider alusrc).
  //       Update its width accordingly.

  assign {regwrite, regdst, alusrc,
          branch, bne, memwrite,
          memtoreg, jump, aluop} = controls;
  // TODO: Add bne (and the extra alusrc bit) into this concatenation.
  //       Make sure the bit positions match your new controls width.

  always_comb
    case(op)
      6'b000000: controls <= 11'b11000000010; // R-type
      6'b100011: controls <= 11'b10010001000; // LW
      6'b101011: controls <= 11'b0x01001x000; // SW
      6'b000100: controls <= 11'b0x00100x001; // BEQ
      6'b001000: controls <= 11'b10010000000; // ADDI
      6'b000010: controls <= 11'b0xxx000x1xx; // J
      // TODO: Add a case for ori (opcode 6'b001101).
      //       ori writes to a register (regwrite=1), uses rt as destination (regdst=0),
      //       uses a zero-extended immediate (alusrc=??), does not branch or write memory,
      //       and the ALU should perform OR. Pick a new aluop encoding (e.g., 2'b11).
      //
      6'b001101: controls <= 11'b10100000011; //ORI
      // TODO: Add a case for bne (opcode 6'b000101).
      //       bne is like beq but sets the bne signal instead of branch.
      //       The ALU still subtracts to check equality (aluop=01).
      6'b000101: controls <= 11'b0x00010x001; //BNE
      default:   controls <= 11'bxxxxxxxxxxx; // ???
      // TODO: Update the default width to match your new controls width.
    endcase
endmodule

module aludec(input  logic [5:0] funct,
              input  logic [1:0] aluop,
              output logic [2:0] alucontrol);

  always_comb
    case(aluop)
      2'b00: alucontrol <= 3'b010;  // add
      2'b01: alucontrol <= 3'b110;  // sub
      // TODO: Add a case for your new aluop encoding (the one you used for ori).
      //       It should produce alucontrol = 3'b001 (OR).
      2'b11: alucontrol <= 3'b001;
      default: case(funct)          // RTYPE
          6'b100000: alucontrol <= 3'b010; // ADD
          6'b100010: alucontrol <= 3'b110; // SUB
          6'b100100: alucontrol <= 3'b000; // AND
          6'b100101: alucontrol <= 3'b001; // OR
          6'b101010: alucontrol <= 3'b111; // SLT
          default:   alucontrol <= 3'bxxx; // ???
        endcase
    endcase
endmodule

module datapath(input  logic        clk, reset,
                input  logic        memtoreg, pcsrc,
                input  logic [1:0]  alusrc,
                input  logic        regdst,
                // TODO: Update alusrc width if you changed it.
                input  logic        regwrite, jump,
                input  logic [2:0]  alucontrol,
                output logic        zero,
                output logic [31:0] pc,
                input  logic [31:0] instr,
                output logic [31:0] aluout, writedata,
                input  logic [31:0] readdata);

  logic [4:0]  writereg;
  logic [31:0] pcnext, pcnextbr, pcplus4, pcbranch;
  logic [31:0] signimm, signimmsh;
  // TODO: Declare a zero-extended immediate signal (e.g., zeroimm).
  logic [31:0] zeroimm;

  logic [31:0] srca, srcb;
  logic [31:0] result;

  // next PC logic
  flopr #(32) pcreg(clk, reset, pcnext, pc);
  adder       pcadd1(pc, 32'b100, pcplus4);
  sl2         immsh(signimm, signimmsh);
  adder       pcadd2(pcplus4, signimmsh, pcbranch);
  mux2 #(32)  pcbrmux(pcplus4, pcbranch, pcsrc,pcnextbr);

  mux2 #(32)  pcmux(pcnextbr, {pcplus4[31:28], instr[25:0], 2'b00}, jump, pcnext);

  // register file logic
  regfile     rf(clk, regwrite, instr[25:21],instr[20:16], writereg,result, srca, writedata);

  mux2 #(5)   wrmux(instr[20:16], instr[15:11],regdst, writereg);

  mux2 #(32)  resmux(aluout, readdata,memtoreg, result);

  signext     se(instr[15:0], signimm);

  // TODO: Instantiate the zeroext module (defined in mipsparts.sv) to produce zeroimm.
  zeroext     ze(instr[15:0], zeroimm);
  // ALU logic
  mux3 #(32)  srcbmux(writedata, signimm, zeroimm, alusrc,srcb);
  // TODO: The SrcB mux currently has 2 inputs (register vs. sign-extended imm).
  //       You need a 3rd input (zero-extended imm) for ori.
  //       Replace mux2 with mux3 (defined in mipsparts.sv) and connect
  //       the three data inputs and the 2-bit alusrc select signal.
  //       alusrc encoding: 00 = register, 01 = sign-ext imm, 10 = zero-ext imm
  alu         alu(.a(srca), .b(srcb), .f(alucontrol),.y(aluout), .zero(zero));
endmodule

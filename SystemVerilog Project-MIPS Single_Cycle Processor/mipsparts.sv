//------------------------------------------------
// mipsparts.sv
// David_Harris@hmc.edu 23 October 2005
// Updated to SystemVerilog dmh 12 November 2010
// Components used in MIPS processor
//------------------------------------------------
//
// TODO SUMMARY: You need to add two new modules at the bottom of this file:
//   1. zeroext — zero-extends a 16-bit input to 32 bits (for ori).
//   2. mux3   — a 3-input multiplexer (for the SrcB selection in the datapath).
//   Both are marked with TODO below.
//------------------------------------------------

module regfile(input  logic        clk, 
               input  logic        we3, 
               input  logic [4:0]  ra1, ra2, wa3, 
               input  logic [31:0] wd3, 
               output logic [31:0] rd1, rd2);

  logic [31:0] rf[31:0];

  // three ported register file
  // read two ports combinationally
  // write third port on rising edge of clock
  // register 0 hardwired to 0

  always_ff @(posedge clk)
    if (we3) rf[wa3] <= wd3;	

  assign rd1 = (ra1 != 0) ? rf[ra1] : 0;
  assign rd2 = (ra2 != 0) ? rf[ra2] : 0;
endmodule

module adder(input  logic [31:0] a, b,
             output logic [31:0] y);

  assign y = a + b;
endmodule

module sl2(input  logic [31:0] a,
           output logic [31:0] y);

  // shift left by 2
  assign y = {a[29:0], 2'b00};
endmodule

module signext(input  logic [15:0] a,
               output logic [31:0] y);
              
  assign y = {{16{a[15]}}, a};
endmodule

// TODO: Add a zeroext module here.
//       It should have the same interface as signext (16-bit input, 32-bit output)
//       but pad the upper 16 bits with zeros instead of replicating the sign bit.
//       This is needed for the ori instruction, which uses zero-extended immediates.

module zeroext(input logic [15:0] a, output logic [31:0] y);
  assign y = {16'b0, a};
endmodule

module flopr #(parameter WIDTH = 8)
              (input  logic             clk, reset,
               input  logic [WIDTH-1:0] d, 
               output logic [WIDTH-1:0] q);

  always_ff @(posedge clk, posedge reset)
    if (reset) q <= 0;
    else       q <= d;
endmodule

module flopenr #(parameter WIDTH = 8)
                (input  logic             clk, reset,
                 input  logic             en,
                 input  logic [WIDTH-1:0] d, 
                 output logic [WIDTH-1:0] q);
 
  always_ff @(posedge clk, posedge reset)
    if      (reset) q <= 0;
    else if (en)    q <= d;
endmodule

module mux2 #(parameter WIDTH = 8)
             (input  logic [WIDTH-1:0] d0, d1, 
              input  logic             s, 
              output logic [WIDTH-1:0] y);

  assign y = s ? d1 : d0; 
endmodule

// TODO: Add a mux3 module here.
//       It should be parameterized like mux2 (with a WIDTH parameter),
//       have three data inputs (d0, d1, d2), a 2-bit select input (s),
//       and one output (y).
//       Use a case statement inside always_comb:
//         s = 2'b00 -> y = d0
//         s = 2'b01 -> y = d1
//         s = 2'b10 -> y = d2
//       This mux is used in the datapath to select SrcB among:
//         register data (00), sign-extended imm (01), zero-extended imm (10).

module mux3 #(parameter WIDTH = 8)
             (input  logic [WIDTH-1:0] d0, d1, d2,
              input  logic [1:0]           s, 
              output logic [WIDTH-1:0] y);

  always_comb
    case(s)
      2'b00: y = d0;
      2'b01: y = d1;
      2'b10: y = d2;
      default: y = {WIDTH{1'bx}};
    endcase
endmodule
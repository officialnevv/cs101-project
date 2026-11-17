// alu.h - the 8-bit ALU (the actual CS101 project).
//
// Everything here is plain C++ with no library calls, so the same code is
// compiled into the console program (main.cpp) and into WebAssembly for the
// web page (wasm.cpp).

#ifndef ALU_H
#define ALU_H

enum Operation {
    OP_ADD = 0,   // A + B
    OP_SUB = 1,   // A - B  (done as A + NOT B + 1, two's complement)
    OP_AND = 2,   // A AND B
    OP_OR  = 3,   // A OR B
    OP_XOR = 4,   // A XOR B
    OP_NOT = 5,   // NOT A        (B is ignored)
    OP_SHL = 6,   // A << B       (logical shift left,  B = 0..7)
    OP_SHR = 7,   // A >> B       (logical shift right, B = 0..7)
    OP_COUNT = 8
};

struct AluResult {
    unsigned char value;    // the 8-bit result
    unsigned char carries;  // ADD/SUB: carry out of every bit position (bit i = carry out of bit i)
    bool carry;             // C: ADD = carry out of bit 7, SUB = borrow, shifts = last bit shifted out
    bool overflow;          // V: signed (two's complement) result does not fit in -128..127
    bool zero;              // Z: result is 0000 0000
    bool negative;          // N: bit 7 of the result is 1 (negative when read as signed)
};

// Runs one ALU operation on two 8-bit inputs.
AluResult alu_execute(Operation op, unsigned char a, unsigned char b);

// True if the operation uses input B.
bool uses_b(Operation op);

// Reads an 8-bit pattern as a two's complement signed number (-128..127).
int to_signed(unsigned char value);

#endif

// alu.cpp - the 8-bit ALU (the actual CS101 project).
//
// Addition is built the way the hardware does it: eight full adders chained
// together (a ripple-carry adder), using only the bitwise operators
// AND (&), OR (|), XOR (^) and shifts.

#include "alu.h"

// Ripple-carry adder: adds a + b + carry_in one bit at a time.
static AluResult ripple_add(unsigned char a, unsigned char b, int carry_in) {
    AluResult r = {};
    int carry = carry_in;

    for (int i = 0; i < 8; i++) {
        int x = (a >> i) & 1;               // bit i of A
        int y = (b >> i) & 1;               // bit i of B

        int sum = x ^ y ^ carry;                         // full adder: sum bit
        carry = (x & y) | (x & carry) | (y & carry);     // full adder: carry out

        r.value |= sum << i;
        r.carries |= carry << i;
    }

    int carry_into_bit7 = (r.carries >> 6) & 1;
    int carry_out_of_bit7 = (r.carries >> 7) & 1;

    r.carry = carry_out_of_bit7;
    // Signed overflow happens when the carry INTO the sign bit differs from
    // the carry OUT of it.
    r.overflow = carry_into_bit7 ^ carry_out_of_bit7;
    return r;
}

AluResult alu_execute(Operation op, unsigned char a, unsigned char b) {
    AluResult r = {};
    int shift = b & 7;   // shifts only use 0..7 positions

    switch (op) {
        case OP_ADD:
            r = ripple_add(a, b, 0);
            break;

        case OP_SUB:
            // Two's complement: A - B = A + (NOT B) + 1
            r = ripple_add(a, (unsigned char)~b, 1);
            // The adder's carry out is 1 when there is NO borrow, so flip it
            // to get the borrow flag (1 when A < B as unsigned numbers).
            r.carry = !r.carry;
            break;

        case OP_AND: r.value = a & b; break;
        case OP_OR:  r.value = a | b; break;
        case OP_XOR: r.value = a ^ b; break;
        case OP_NOT: r.value = (unsigned char)~a; break;

        case OP_SHL:
            r.value = (unsigned char)(a << shift);
            // C = the last bit pushed out on the left
            r.carry = shift > 0 && ((a >> (8 - shift)) & 1);
            break;

        case OP_SHR:
            r.value = a >> shift;
            // C = the last bit pushed out on the right
            r.carry = shift > 0 && ((a >> (shift - 1)) & 1);
            break;

        default:
            break;
    }

    r.zero = (r.value == 0);
    r.negative = (r.value >> 7) & 1;
    return r;
}

bool uses_b(Operation op) {
    return op != OP_NOT;
}

int to_signed(unsigned char value) {
    // If the sign bit (bit 7) is 1, the value is negative: subtract 2^8.
    return (value & 0x80) ? value - 256 : value;
}

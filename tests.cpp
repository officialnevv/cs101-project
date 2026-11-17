// tests.cpp - checks alu_execute() against known answers.
//
// Build & run:  g++ -std=c++17 -Wall -o tests tests.cpp alu.cpp && ./tests

#include <iostream>

#include "alu.h"

using namespace std;

int failures = 0;

void check(const char* name, Operation op, int a, int b,
           int value, bool c, bool v, bool z, bool n) {
    AluResult r = alu_execute(op, (unsigned char)a, (unsigned char)b);
    bool ok = r.value == value && r.carry == c && r.overflow == v && r.zero == z && r.negative == n;
    cout << (ok ? "  PASS  " : "  FAIL  ") << name;
    if (!ok) {
        cout << "  (got value=" << (int)r.value << " C=" << r.carry << " V=" << r.overflow
             << " Z=" << r.zero << " N=" << r.negative << ")";
        failures++;
    }
    cout << "\n";
}

int main() {
    //                                   op      A     B     value  C  V  Z  N
    check("ADD 20 + 30 = 50",           OP_ADD,  20,   30,    50,   0, 0, 0, 0);
    check("ADD 90 + 60 signed overflow",OP_ADD,  90,   60,   150,   0, 1, 0, 1);
    check("ADD 200 + 100 carry",        OP_ADD, 200,  100,    44,   1, 0, 0, 0);
    check("ADD -1 + 1 = 0 (carry)",     OP_ADD, 255,    1,     0,   1, 0, 1, 0);
    check("ADD -128 + -1 overflow",     OP_ADD, 128,  255,   127,   1, 1, 0, 0);
    check("SUB 50 - 20 = 30",           OP_SUB,  50,   20,    30,   0, 0, 0, 0);
    check("SUB 20 - 50 borrow",         OP_SUB,  20,   50,   226,   1, 0, 0, 1);
    check("SUB 7 - 7 = 0",              OP_SUB,   7,    7,     0,   0, 0, 1, 0);
    check("SUB -128 - 1 overflow",      OP_SUB, 128,    1,   127,   0, 1, 0, 0);
    check("SUB 127 - -1 overflow",      OP_SUB, 127,  255,   128,   1, 1, 0, 1);
    check("AND 0xCC & 0xAA",            OP_AND, 0xCC, 0xAA, 0x88,   0, 0, 0, 1);
    check("OR  0xCC | 0xAA",            OP_OR,  0xCC, 0xAA, 0xEE,   0, 0, 0, 1);
    check("XOR 0xCC ^ 0xAA",            OP_XOR, 0xCC, 0xAA, 0x66,   0, 0, 0, 0);
    check("XOR x ^ x = 0",              OP_XOR, 0x5A, 0x5A, 0x00,   0, 0, 1, 0);
    check("NOT 0x0F",                   OP_NOT, 0x0F,    0, 0xF0,   0, 0, 0, 1);
    check("SHL 0x81 << 1",              OP_SHL, 0x81,    1, 0x02,   1, 0, 0, 0);
    check("SHL 0x01 << 7",              OP_SHL, 0x01,    7, 0x80,   0, 0, 0, 1);
    check("SHL by 0 keeps value",       OP_SHL, 0x5A,    0, 0x5A,   0, 0, 0, 0);
    check("SHR 0x81 >> 1",              OP_SHR, 0x81,    1, 0x40,   1, 0, 0, 0);
    check("SHR 0x80 >> 7",              OP_SHR, 0x80,    7, 0x01,   0, 0, 0, 0);

    // Every ADD/SUB pair must match ordinary integer maths modulo 256.
    int wrong = 0;
    for (int a = 0; a < 256; a++) {
        for (int b = 0; b < 256; b++) {
            AluResult add = alu_execute(OP_ADD, a, b);
            AluResult sub = alu_execute(OP_SUB, a, b);
            int sa = to_signed(a), sb = to_signed(b);
            if (add.value != ((a + b) & 0xFF) || add.carry != (a + b > 255) ||
                add.overflow != (sa + sb < -128 || sa + sb > 127)) wrong++;
            if (sub.value != ((a - b) & 0xFF) || sub.carry != (a < b) ||
                sub.overflow != (sa - sb < -128 || sa - sb > 127)) wrong++;
        }
    }
    cout << (wrong == 0 ? "  PASS  " : "  FAIL  ") << "all 65,536 ADD and SUB pairs\n";
    if (wrong) failures++;

    cout << (failures == 0 ? "\nAll tests passed.\n" : "\nSome tests failed.\n");
    return failures == 0 ? 0 : 1;
}

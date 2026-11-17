// wasm.cpp - tiny wrapper that lets the web page call the ALU.
//
// Like app.py in the image project, this does no maths of its own. It calls
// alu_execute() from alu.cpp and packs the answer into one integer so
// JavaScript can read it easily:
//
//   bits  0-7   result value
//   bits  8-15  carry out of every bit (ADD/SUB)
//   bit   16    C (carry / borrow)
//   bit   17    V (overflow)
//   bit   18    Z (zero)
//   bit   19    N (negative)
//
// Build with ./build_wasm.sh (clang, no Emscripten needed).

#include "alu.h"

extern "C" __attribute__((export_name("alu_run")))
int alu_run(int op, int a, int b) {
    if (op < 0 || op >= OP_COUNT) return -1;

    AluResult r = alu_execute((Operation)op, (unsigned char)a, (unsigned char)b);

    return r.value
         | (r.carries << 8)
         | (r.carry << 16)
         | (r.overflow << 17)
         | (r.zero << 18)
         | (r.negative << 19);
}

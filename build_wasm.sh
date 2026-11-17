#!/usr/bin/env sh
# Compiles the ALU core (alu.cpp) + wrapper (wasm.cpp) to public/alu.wasm.
# Needs only clang and wasm-ld (no Emscripten). The output is committed, so
# Vercel serves it as a static file and does not need to build anything.
set -e
cd "$(dirname "$0")"

clang++ --target=wasm32 -std=c++17 -O2 -nostdlib -ffreestanding \
  -fno-exceptions -fno-rtti \
  -Wl,--no-entry -Wl,--strip-all \
  -o public/alu.wasm alu.cpp wasm.cpp

echo "built public/alu.wasm ($(wc -c < public/alu.wasm) bytes)"

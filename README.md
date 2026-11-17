# 8-Bit ALU Simulator

CS101 – Introduction to Computer Systems, group project (topic #2: 8-bit ALU Simulator).

The user enters two 8-bit values and picks an operation (ADD, SUB, AND, OR, XOR, NOT, SHL or SHR). The program shows the result in binary, unsigned decimal, signed decimal and hex, along with the four status flags:

| Flag | Name | Set when… |
| --- | --- | --- |
| **C** | Carry / Borrow | ADD: the result needs a 9th bit. SUB: A < B (unsigned), so a borrow happened. Shifts: the last bit pushed out was 1. |
| **V** | Overflow | ADD/SUB: the signed (two's complement) answer doesn't fit in −128…127. |
| **Z** | Zero | The result is `0000 0000`. |
| **N** | Negative | Bit 7 of the result is 1. |

## Project structure

```text
cs101-project/
├── alu.h / alu.cpp    # The project's core: the 8-bit ALU (plain C++, no libraries)
├── main.cpp           # Console program: menu, input, output (the submission)
├── tests.cpp          # Known-answer tests + all 65,536 ADD/SUB input pairs
├── wasm.cpp           # Small wrapper that exposes the ALU to the web page
├── build_wasm.sh      # Compiles alu.cpp + wasm.cpp to public/alu.wasm
├── public/
│   ├── index.html     # Pixel-style browser interface (with Nib the mascot)
│   └── alu.wasm       # Compiled ALU (committed, so Vercel serves it directly)
└── vercel.json
```

## The core: `alu.cpp`

`alu_execute(op, a, b)` does all the work and returns the result plus the flags.

- **ADD** is a ripple-carry adder: eight full adders chained together, built only from `&`, `|`, `^` and shifts:
  `sum = a ^ b ^ carry`, `carry = (a & b) | (a & carry) | (b & carry)`.
- **SUB** reuses the adder through two's complement: `A − B = A + (NOT B) + 1`. The `+1` goes in as the first carry.
- **Overflow** is `carry into bit 7 XOR carry out of bit 7`.
- **AND / OR / XOR / NOT** use the C++ bitwise operators.
- **SHL / SHR** are logical shifts by 0–7 places. The last bit shifted out goes into C.

The same `alu.cpp` file is compiled twice: once into the console program, and once into WebAssembly for the web page. The web page therefore runs the exact same C++ code; its JavaScript only draws the screen.

## Run the console program

With g++ (on Windows that means MinGW or MSYS2; Visual Studio works too):

```bash
g++ -std=c++17 -Wall -o alu main.cpp alu.cpp
./alu          # Windows: alu.exe
```

Inputs can be decimal (`90`, `-38`), binary (`0b01011010`) or hexadecimal (`0x5A`). Negative decimals are stored as two's complement.

Example session:

```text
Choose an operation: 1
Enter A (e.g. 90, -38, 0b01011010, 0x5A): 90
Enter B: 60

  carries   0111 1000
  A         0101 1010   unsigned  90  |  signed   90  |  hex 0x5A
  + B       0011 1100   unsigned  60  |  signed   60  |  hex 0x3C
            ---------
  ADD =     1001 0110   unsigned 150  |  signed -106  |  hex 0x96

  Flags:  C=0  V=1  Z=0  N=1
  - V=1  signed overflow: 90 + 60 = 150 is outside -128..127, so the 8 bits read as -106.
  - N=1  bit 7 is set, so the result is negative when read as signed.
```

## Run the tests

```bash
g++ -std=c++17 -Wall -o tests tests.cpp alu.cpp
./tests
```

## Web version

The page in `public/` loads `alu.wasm` and calls `alu_run()`, which is defined in `wasm.cpp`. Each run goes through three steps: pick an operation, set A and B (click the bits or type a number), then see the long-hand calculation with its carries and flags. On the result screen you can click any bit of A or B and the answer updates straight away.

Preview locally (it has to be served over HTTP, opening the file directly won't load the WASM):

```bash
python -m http.server 8000 -d public
# then open http://localhost:8000
```

Rebuild the WASM after changing `alu.cpp` (needs clang and lld; Emscripten isn't needed):

```bash
./build_wasm.sh
```

### Deploy on Vercel

The site is fully static. `vercel.json` points Vercel at `public/` and there's no build step. You can either:

- import this GitHub repo at <https://vercel.com/new> and keep the defaults, or
- run `vercel --prod` from the project folder.

## Use of AI tools

As the assignment requires, state here which AI tools the group used and what they helped with (for example: planning, the web interface, or debugging). Every group member should be able to explain all of the code.

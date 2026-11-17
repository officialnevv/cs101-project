// main.cpp - console interface for the 8-bit ALU simulator.
//
// This file only handles input and output. All the arithmetic and logic is
// done by alu_execute() in alu.cpp.
//
// Build:  g++ -std=c++17 -Wall -o alu main.cpp alu.cpp
// Run:    ./alu        (Windows: alu.exe)

#include <iomanip>
#include <iostream>
#include <string>

#include "alu.h"

using namespace std;

const char* OP_NAMES[OP_COUNT] = {"ADD", "SUB", "AND", "OR", "XOR", "NOT", "SHL", "SHR"};
const char* OP_SYMBOLS[OP_COUNT] = {"+", "-", "AND", "OR", "XOR", "NOT", "<<", ">>"};

// "0101 1010" - an 8-bit pattern split into two nibbles.
string to_binary(unsigned char value) {
    string text;
    for (int i = 7; i >= 0; i--) {
        text += ((value >> i) & 1) ? '1' : '0';
        if (i == 4) text += ' ';
    }
    return text;
}

string to_hex(unsigned char value) {
    const char* digits = "0123456789ABCDEF";
    string text = "0x";
    text += digits[value >> 4];    // high nibble
    text += digits[value & 0xF];   // low nibble
    return text;
}

// Value of one digit character in the given base, or -1 if it is not valid.
int digit_value(char c, int base) {
    int v = -1;
    if (c >= '0' && c <= '9') v = c - '0';
    else if (c >= 'a' && c <= 'f') v = c - 'a' + 10;
    else if (c >= 'A' && c <= 'F') v = c - 'A' + 10;
    return (v >= 0 && v < base) ? v : -1;
}

// Parses decimal (-128..255), binary (0b0101) or hexadecimal (0x5A) input.
// Negative decimals are stored as their two's complement bit pattern.
bool parse_byte(string text, unsigned char& out) {
    // remove spaces so "0b0101 1010" also works
    string s;
    for (char c : text) if (c != ' ' && c != '\t' && c != '\r') s += c;
    if (s.empty()) return false;

    bool negative = false;
    int base = 10;
    size_t i = 0;

    if (s[0] == '-') { negative = true; i = 1; }
    if (!negative && s.size() > 2 && s[0] == '0' && (s[1] == 'b' || s[1] == 'B')) { base = 2; i = 2; }
    if (!negative && s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) { base = 16; i = 2; }
    if (i >= s.size()) return false;

    long value = 0;
    for (; i < s.size(); i++) {
        int d = digit_value(s[i], base);
        if (d < 0) return false;
        value = value * base + d;
        if (value > 255) return false;
    }

    if (negative) {
        if (value > 128) return false;
        value = 256 - value;          // two's complement of the magnitude
        if (value == 256) value = 0;  // "-0"
    }

    out = (unsigned char)value;
    return true;
}

void print_row(const string& label, unsigned char value) {
    cout << "  " << left << setw(10) << label << to_binary(value)
         << "   unsigned " << right << setw(3) << (int)value
         << "  |  signed " << setw(4) << to_signed(value)
         << "  |  hex " << to_hex(value) << "\n";
}

void explain_flags(Operation op, unsigned char a, unsigned char b, const AluResult& r) {
    int sa = to_signed(a), sb = to_signed(b);

    cout << "\n  Flags:  C=" << r.carry << "  V=" << r.overflow
         << "  Z=" << r.zero << "  N=" << r.negative << "\n";

    if (r.carry) {
        if (op == OP_ADD)
            cout << "  - C=1  carry out of bit 7: unsigned " << (int)a << " + " << (int)b << " = "
                 << (int)a + (int)b << " does not fit in 0..255 (kept " << (int)r.value << ").\n";
        else if (op == OP_SUB)
            cout << "  - C=1  borrow: unsigned " << (int)a << " < " << (int)b
                 << ", so the result wrapped around to " << (int)r.value << ".\n";
        else
            cout << "  - C=1  the last bit shifted out was a 1.\n";
    }
    if (r.overflow) {
        int exact = (op == OP_ADD) ? sa + sb : sa - sb;
        cout << "  - V=1  signed overflow: " << sa << (op == OP_ADD ? " + " : " - ") << sb << " = " << exact
             << " is outside -128..127, so the 8 bits read as " << to_signed(r.value) << ".\n";
    }
    if (r.zero) cout << "  - Z=1  the result is zero.\n";
    if (r.negative) cout << "  - N=1  bit 7 is set, so the result is negative when read as signed.\n";
    if (!r.carry && !r.overflow && !r.zero && !r.negative) cout << "  - No flags set.\n";
}

bool read_line(const string& prompt, string& line) {
    cout << prompt;
    return (bool)getline(cin, line);
}

bool read_byte(const string& prompt, unsigned char& out) {
    string line;
    while (read_line(prompt, line)) {
        if (parse_byte(line, out)) return true;
        cout << "  Invalid. Use decimal -128..255, binary 0b01011010 or hex 0x5A.\n";
    }
    return false;
}

bool read_shift(unsigned char& out) {
    string line;
    while (read_line("Shift by how many bits (0-7): ", line)) {
        unsigned char n;
        if (parse_byte(line, n) && n <= 7) { out = n; return true; }
        cout << "  Invalid. Enter a number from 0 to 7.\n";
    }
    return false;
}

void print_menu() {
    cout << "\n==========================================================\n"
         << "               8-BIT ALU SIMULATOR  (CS101)\n"
         << "==========================================================\n"
         << "  1) ADD   A + B            5) XOR   A XOR B\n"
         << "  2) SUB   A - B            6) NOT   NOT A\n"
         << "  3) AND   A AND B          7) SHL   A << n (shift left)\n"
         << "  4) OR    A OR B           8) SHR   A >> n (shift right)\n"
         << "  0) Exit\n";
}

int main() {
    string line;

    while (true) {
        print_menu();
        if (!read_line("Choose an operation: ", line)) break;
        if (line == "0") break;
        if (line.size() != 1 || line[0] < '1' || line[0] > '8') {
            cout << "  Please choose 0-8.\n";
            continue;
        }
        Operation op = (Operation)(line[0] - '1');

        unsigned char a = 0, b = 0;
        if (!read_byte("Enter A (e.g. 90, -38, 0b01011010, 0x5A): ", a)) break;
        if (op == OP_SHL || op == OP_SHR) {
            if (!read_shift(b)) break;
        } else if (uses_b(op)) {
            if (!read_byte("Enter B: ", b)) break;
        }

        AluResult r = alu_execute(op, a, b);

        cout << "\n";
        if (op == OP_ADD || op == OP_SUB)
            cout << "  " << left << setw(10) << "carries" << to_binary(r.carries) << "\n";
        print_row("A", a);
        if (op == OP_SUB) {
            print_row("NOT B", (unsigned char)~b);
            cout << "  (A - B is computed as A + NOT B + 1, with B = " << to_binary(b) << ")\n";
        } else if (op == OP_SHL || op == OP_SHR) {
            cout << "  " << left << setw(10) << OP_SYMBOLS[op] << (int)b << " bit(s)\n";
        } else if (uses_b(op)) {
            print_row(string(OP_SYMBOLS[op]) + " B", b);
        }
        cout << "  " << setw(10) << "" << "---------\n";
        print_row(string(OP_NAMES[op]) + " =", r.value);
        explain_flags(op, a, b, r);
    }

    cout << "\nBye!\n";
    return 0;
}

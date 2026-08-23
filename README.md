# ⚙️ C-to-X86-Compiler

A modular, multi-pass C++20-subset compiler targeting x86-64 Linux. This compiler has been completely re-architected from the ground up to follow modern compiler design principles, featuring distinct frontend, middle-end, and backend phases.

---

## **Overview**

This repository contains a fully functional compiler that translates a subset of C++ into Intel-syntax x86-64 assembly. It was built incrementally through an 18-phase roadmap to demonstrate how production-grade compilers like Clang and GCC operate.

### Key Features
- **Modern Architecture**: Distinct stages for Lexical Analysis, Parsing, Semantic Analysis, Intermediate Representation (IR), Optimization, and Code Generation.
- **Advanced C++ Support**: Supports primitive types, pointers, arrays, structs/classes, methods, fields, implicit `this` pointer offset calculations, and complex control flow (`if`, `else`, `while`).
- **Resilient Error Recovery**: Features a modern `DiagnosticEngine` with source code context (color-coded carets `^`) and a Parser implementing **Panic Mode Error Recovery** (token synchronization) to prevent cascading errors on syntax violations.
- **SSA-inspired IR**: An Intermediate Representation utilizing Basic Blocks and Instruction objects for structural control flow.
- **Optimization Passes**: Pluggable optimization pipeline including Constant Folding and Dead Code Elimination.
- **Compiler Driver**: A robust CLI mimicking GCC/Clang with support for multi-file compilation, automated linker invocation, and pipeline introspection flags (`-ast`, `-ir`, `-asm`, etc.).

---

## **Architecture Pipeline**

```
Source Code (.cpp)
       ↓
[ Lexer ]             → Emits a stream of expanded TokenKinds
       ↓
[ Parser ]            → Pratt-parsing & Recursive Descent → Abstract Syntax Tree (AST)
       ↓
[ Semantic Analyzer ] → Scoping, Type Checking, Symbol Resolution
       ↓
[ IR Generator ]      → Lowers AST into BasicBlock & Instruction IR
       ↓
[ Optimizer ]         → Runs pass manager (Constant Folding, DCE)
       ↓
[ x86-64 Backend ]    → Instruction Selection & System V AMD64 ABI Lowering
       ↓
[ GCC / Linker ]      → Auto-links generated assembly into final ELF executable
```

---

## **Build Instructions**

### **Prerequisites**
- **CMake** (3.10+)
- A C++20 compatible compiler (GCC or Clang)
- **GoogleTest** (Optional, for running tests)

### **Building from Source**
```bash
mkdir build
cd build
cmake ..
make
```
This will produce the `cppx86_test` compiler executable in the `build/` directory (or in root depending on build).

### **Running Tests**
```bash
cd build
ctest --output-on-failure
```

---

## **Usage**

The compiler driver mimics standard compiler flags:

```bash
# Compile and link multiple files into an executable
./cppx86_test main.cpp utils.cpp -o my_app

# Compile to assembly only
./cppx86_test -c main.cpp
```

### **Pipeline Introspection Flags**
You can halt the compilation pipeline at various stages to inspect internal representations:

- `-tokens` : Dump the lexical token stream
- `-ast`    : Dump the Abstract Syntax Tree (AST)
- `-sema`   : Halt after Semantic Analysis (verification mode)
- `-ir`     : Dump the unoptimized Intermediate Representation
- `-opt`    : Dump the optimized Intermediate Representation
- `-asm`    : Dump the final x86-64 Intel assembly output

**Example:**
```bash
./cppx86_test -ast my_file.cpp
```

---

## **Language Subset Supported**

- **Types**: `int`, `float`, `void`, pointers, arrays.
- **Control Flow**: `if`, `else`, `while`, `return`.
- **Expressions**: Binary operations, unary operations, assignments, variable access.
- **Object Model**: `class` and `struct` declarations, member fields, method declarations, and member access via `.` and `->` operators.

### **Example Input**
```cpp
class Vector {
    int x;
    int y;
};

int main() {
    int counter = 10;
    while (counter > 0) {
        counter = counter - 1;
    }
    return counter;
}
```

---

## **Historical Note**

The original monolithic compiler prototype can be found in the `legacy/` directory. The repository has since been overhauled into a modular pipeline following the 18-phase implementation roadmap.

> _“Compilers convert human intent into machine action — and learning them is one of the most rewarding journeys in systems programming.”_

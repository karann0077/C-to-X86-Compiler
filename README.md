# ⚙️ A Minimal C++ to x86 Compiler**

>  A lightweight compiler written in **C++17** that translates a subset of C++-like syntax into **x86 assembly**.

---

## **Overview**

Built to demonstrate the fundamental steps of compilation  from **lexical analysis** to **assembly generation**.  

This project aims to **help learners understand how compilers actually work**, covering:
- **Tokenization**  
- **Parsing**  
- **Abstract Syntax Tree (AST)** creation  
- **Code generation** for x86-64 assembly  

---

## **Project Goals**

-  Build a **self-contained compiler** with no external frameworks  
-  Implement a **recursive descent parser** for C-like syntax  
-  Generate **x86-64 assembly** output  
-  Provide **readable, beginner-friendly code**  
-  Serve as a foundation for further compiler projects or experiments  

---

## **Architecture**

```
Source Code (.tc)
     ↓
[ Lexer ]  → Converts characters into tokens
     ↓
[ Parser ] → Builds an Abstract Syntax Tree (AST)
     ↓
[ CodeGen ] → Produces x86-64 assembly (.s)
     ↓
[ Assembler + Linker ] → Generates final executable
```

---

##  **Project Structure**

```
tinycc/
├── src/
│   ├── lexer.cpp
│   ├── lexer.h     
│   ├── parser.cpp
│   ├── parser.h     
│   ├── main.cpp    
│   ├── codegen.cpp        
│   ├── codegen.h    
│   ├── ast.h     
├── test/
│   └── sample.tc      
└── README
```

---

##  **Build Instructions**

### **Prerequisites**
Make sure you have:
- **GCC** or **Clang** (with C++17 support)  
- **Linux x86_64** (preferred) or macOS (Intel)  
- `make` (optional)

> **Note:** The provided code generator emits **Intel style x86-64 assembly** and targets the **System V ABI** (Linux x86_64). If you are on macOS (especially Apple Silicon / arm64), see the **Platform Notes** section below.

---

###  **Build the Compiler**

```bash
g++ -std=c++17 -O0 -g -Isrc -o tinycc src/main.cpp src/lexer.cpp src/parser.cpp src/codegen.cpp
```

This will produce an executable named `tinycc` in the project root.

---

###  **Run the Compiler**

```bash
./tinycc test/sample.tc
```

This generates the x86 assembly output file:  
`test/sample.tc.s`

---

###  **Assemble and Link**

```bash
# Assemble the .s into an object file
gcc -c test/sample.tc.s -o test/sample.o

# Link the object into an executable
gcc test/sample.o -o prog

# Run the program
./prog

# Show the return value of main()
echo $?
```

The final number printed (`$?`) is the **return value** from `main()` — the output of your compiled program.

---

##  **Example Input File (`test/sample.tc`)**

```c
int main() {
    int x = 10;
    int y = 20;
    int z;
    z = x + y * 2;
    if (z > 40) {
        z = z - 10;
    } else {
        z = z + 5;
    }
    while (x < 20) {
        x = x + 1;
    }
    return z;
}
```

---

##  **How It Works — Stage by Stage**

### 1. **Lexical Analysis (Lexer)**
- Reads source code character-by-character.
- Groups sequences into **tokens** (identifiers, keywords, numbers, operators).
- Removes whitespace and comments.

Example:
```c
int x = 5 + 3;
```
→ Tokens: `KwInt`, `Identifier(x)`, `Assign`, `Number(5)`, `Plus`, `Number(3)`, `Semicolon`

---

### 2. **Parsing (Recursive Descent Parser)**
- Parses tokens into an **AST** following grammar rules.
- Supports expressions, declarations, `if`/`else`, `while`, and `return`.
- Produces structured nodes for code generation.

---

### 3. **AST Representation**
- AST node types include: `Integer`, `VarExpr`, `Binary`, `DeclStmt`, `ExprStmt`, `ReturnStmt`, `IfStmt`, `WhileStmt`, `BlockStmt`, `Function`, `Program`.
- Nodes are stored using `std::unique_ptr` for safe ownership semantics.

---

### 4. **Code Generation**
- Translates AST nodes into **x86-64 Intel syntax** assembly using a simple stack-frame model:
  - Function prologue: `push rbp; mov rbp, rsp; sub rsp, <frameSize>`
  - Locals stored at `[rbp - offset]` (4 bytes per `int`)
  - Expression evaluation uses `eax`, `ebx`, `push`/`pop` temporaries
  - Control flow is implemented using labels `.L0`, `.L1`, etc.
  - Function return in `eax`

---

## **Generated Assembly (Excerpt)**

```asm
    .text
    .global main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    mov eax, 10
    mov DWORD PTR [rbp-4], eax
    mov eax, 20
    mov DWORD PTR [rbp-8], eax
    ...
    add rsp, 16
    pop rbp
    ret
```

> The above is Intel-style assembly; Linux `gcc`/`as` on x86-64 will assemble it directly.

---

##  **Limitations & Known Issues**

- **Language subset only**: currently supports `int` type only, functions without parameters, local variables, arithmetic, comparisons, `if`/`else`, `while`, and `return`.
- **Simple code generation**: uses `push`/`pop` and `eax/ebx` temporaries — not optimized.
- **Return handling**: early `return` handling may need refinement (epilogue correctness); can be improved.
- **No function calls/params**: calling convention and parameter passing are not implemented yet.
- **Target platform**: emits x86-64 (Intel syntax) for System V ABI (Linux). macOS or ARM targets require codegen changes.

---

##  **Platform Notes (macOS / Apple Silicon)**

- On **macOS (arm64 / Apple Silicon)** the native assembler expects ARM64 assembly and will not accept Intel-style x86-64 `.s` files. You have options:
  1. **Use Docker** (recommended) to run an x86_64 Linux container and assemble/run inside it.
  2. **Use an x86_64 toolchain** on macOS (via Rosetta or cross-toolchain), and assemble with flags like `clang -x assembler -arch x86_64`.
  3. **Modify codegen** to emit **AT&T syntax** or an **ARM64 backend** — I can help add these options.

**Docker example (quick test on macOS):**
```bash
# from project root (requires Docker installed)
docker run --rm -it -v "$(pwd)":/src -w /src ubuntu:22.04 bash
# inside container:
apt update && apt install -y build-essential
gcc -no-pie -o prog test/sample.tc.s
./prog && echo $?
```

---

##  **Suggested Next Improvements (Roadmap)**

- [ ] Fix **early return** epilogue (proper stack unwind on `return`)
- [ ] Add **function parameters** & **call support** using System V ABI (`rdi`, `rsi`, ...)
- [ ] Implement **type checking** and better error messages
- [ ] Add **simple optimizations** (constant folding, dead-code elimination)
- [ ] Implement a **register allocator** (linear scan or graph-coloring)
- [ ] Provide **LLVM IR** backend (optional) for better code generation
- [ ] Add **unit tests** & CI (GitHub Actions)

---

##  **Contributing**

Contributions are welcome! If you want to help:
1. Fork the repository
2. Create a feature branch (e.g., `feature/params`)
3. Open a pull request with a clear description of your changes

Please follow this repository's code style and add tests for any new features.

---

]
> _“Compilers convert human intent into machine action — and learning them is one of the most rewarding journeys in systems programming.”_

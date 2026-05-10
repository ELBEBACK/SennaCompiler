# SennaCompiler

> An educational optimizing compiler for a C-like statically compiled language, written in C++17.

SennaC is a from-scratch compiler where every major algorithm of frontend and middlend are implemented in the project in a form of introductory material on compiler theory for compiler enthuasists.

---

## Language

SennaC is **C with a different surface syntax**. The type system, memory model, calling convention, and runtime behavior are identical to C. Only the way you write programs differs.

```
func add(a, b) {
    c = a + b;
    c += 1;
    add(c, b);
    while( c > 2) {
        if(a == 1) {
            break;
        }
    }
}

for(e = 0; e < 1; e++) {
    c = add(1, 2);
}

res = add(10, 20);

print res;
```

### What stays the same as C

- Static compilation — no interpreter, no runtime, no GC
- Same type semantics: integer arithmetic, overflow, signedness
- Same memory model — stack frames, function calls, System V AMD64 ABI
- Same control flow semantics — `if`/`else`, `while`, functions, `return`
- The binary produced is indistinguishable in behavior from equivalent C

### What differs syntactically

| C | SennaC |
|---|---|
| `int foo(int x, int y)` | `func foo(x, y)` |
| `printf("%d\n", x)` | `print x` |
| `x += 1;` | `x += 1;` (same) |
| `/* comment */` | `/* comment */` (same) |
| `// comment` | `// comment` (same) |


## How to use


```sh
git clone git@github.com:ELBEBACK/SennaCompiler.git
cd *project/*
cmake -B build
cmake --build build
```

The binary is `build/senna`.
After successful compilation, the **dot2png.sh** script is provided to transform all existing **.dot** files in **output/dot** into **.png** in **output/png** directory.

---

## Usage

```sh
senna <source_file> [options]
```

### Options

| Flag | Description |
|---|---|
| `--emit=ast` | Parse source, dump AST as Graphviz DOT to `output/dot/ast_output.dot` |

### Examples

```sh
# Parse and check for syntax errors
senna program.pcl

# Dump AST as DOT graph
senna program.pcl --emit=ast

# Render to PNG (requires Graphviz)
./dot2png.sh
# or manually:
dot -Tpng output/dot/ast_output.dot -o ast.png
```

---

## Planned features

Full menu of what can be implemented. Items will be checked off as they land.

### Frontend
- [x] Flex-based lexer (DFA)
- [x] LALR(1) parser (Bison)
- [x] AST construction, visitor pattern
- [x] DOT dump of AST (`--emit=ast`)
- [ ] Source locations (line/column) in AST nodes
- [ ] Semantic analysis — symbol table, scope resolution
- [ ] Type checking
- [ ] Error recovery in parser
- [ ] Own DFA and PDA implementation instead of Bison and Flex

### IR
- [ ] Three-address IR: `alloca` / `load` / `store` form, `--emit=ir`
- [ ] CFG construction, `--dump-dot=cfg`
- [ ] Textual IR printer (for diff-based testing)
- [ ] IR verifier (use-def consistency, dominance, type invariants)

### Middle-end — SSA
- [ ] Dominator tree
- [ ] Dominance frontiers
- [ ] `mem2reg` — phi placement  + renaming via domtree walk, `--emit=ssa`
- [ ] SSA verifier — every use dominated by def, phi operand count matches predecessor count

### Middle-end — Scalar optimizations
- [ ] Dead code elimination (DCE)
- [ ] Constant folding + algebraic simplification
- [ ] SCCP — sparse conditional constant propagation 

### Middle-end — Loop optimizations
- [ ] Natural loop detection (back-edges + dominator tree)
- [ ] Loop nesting tree (Havlak's algorithm)
- [ ] Loop canonicalization — single preheader, single latch, dedicated exits
- [ ] LICM — loop-invariant code motion

### Backend
- [ ] Usage of LLVM tooling for backend part

### Debug and tooling
- [ ] `--explain` — trace algorithm decisions at every pass (phi placement, DCE removals, etc.)

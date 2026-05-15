# SennaC

> An educational optimizing compiler for a C-like language, written in C++17.
> Every major algorithm — SSA construction, dominance, scalar and loop optimizations — is implemented from scratch as introductory material on compiler theory.

> The middle-end runs entirely on SennaC's own IR. LLVM is used only as an assembler — invoked with `-O0` so it does no further optimization, and the output is indistinguishable from equivalent C compiled at `-O0`.



## Language

SennaC is **C with a different surface syntax**. The type system, memory model, calling convention, and runtime behavior are identical to C.

```
func add(a, b) {
    c = a + b;
    c += 1;
    while (c > 2) {
        if (a == 1) {
            break;
        }
    }
    return c;
}

for (e = 0; e < 10; e++) {
    res = add(e, 2);
    print res;
}
```

### What stays the same as C

- Static compilation — no interpreter, no runtime, no GC
- Integer arithmetic, overflow, signedness semantics
- Stack frames, System V AMD64 ABI
- Control flow: `if`/`else`, `while`, `for`, `break`, `continue`, `return`

### Syntax differences

| C | SennaC |
|---|--------|
| `int foo(int x, int y)` | `func foo(x, y)` |
| `printf("%d\n", x)` | `print x` |
| `// comment`, `/* */` | same |

---

## Build and install

| Command | Description |
|---------|-------------|
| `make` | Build the project |
| `make install` | Install `senna` to `~/.local/bin` |
| `make dot` | Convert **.dot** files in `output/dot/` into **.png** in `output/png/` |
| `make clean` | Wipe `build/` and `output/` |
| `make uninstall` | Remove `senna` from `~/.local/bin` |



## Usage

```sh
senna <source_file> [options]
```

### Flags

The manual can be printed with `-h` / `--help`. All output is controlled via `--emit` with a comma-separated list of targets.

| Flag | Output | Description |
|------|--------|-------------|
| `--emit=ast` | `output/dot/ast_output.dot` | AST as Graphviz DOT |
| `--emit=ir` | `output/out.ir` | Three-address IR before SSA |
| `--emit=cfg` | `output/dot/cfg_<fn>.dot` | CFG per function |
| `--emit=dom` | `output/dot/dom_<fn>.dot` | Dominator tree per function |
| `--emit=fdom` | `output/dot/fdom_<fn>.dot` | Dominance frontiers per function |
| `--emit=ssa` | `output/out.ssa` | IR after mem2reg, in SSA form |
| `--emit=loops` | `output/dot/loops_<fn>.dot` | Loop nesting tree per function |
| `--emit=llvm` | `output/out.ll` | LLVM IR; `clang -O0` is invoked automatically to produce the binary |

### Default mode

Running `senna` with no `--emit` flags compiles straight to a native binary and removes the intermediate `.ll` file afterwards. It is equivalent to `--emit=llvm` without keeping the IR on disk.

### Examples

```sh
# Compile to native binary (by default intermediate .ll is removed)
senna program.sn

# Keep the LLVM IR alongside the binary
senna program.sn --emit=llvm

# Dump intermediate graphs
senna program.sn --emit=ast,fdom
make dot                          # render to PNG

# Inspect IR at each stage
senna program.sn --emit=ir
senna program.sn --emit=ssa
```

---

## Planned features

### Frontend
- [x] Flex lexer (DFA)
- [x] Bison LALR(1) parser
- [x] AST construction, visitor pattern
- [x] DOT dump of AST (`--emit=ast`)
- [x] Line/column tracking in lexer (`yylloc`)
- [x] Semantic analysis — scope resolution, use-before-def
- [x] Error recovery in parser

### IR
- [x] Three-address IR: `alloca` / `load` / `store` form (`--emit=ir`)
- [x] CFG construction (`--emit=cfg`)
- [x] Textual IR printer (for diff-based testing)
- [x] IR verifier (use-def consistency, type invariants)

### SSA construction
- [x] Dominator tree (`--emit=dom`)
- [x] Dominance frontiers (`--emit=fdom`)
- [x] `mem2reg` — phi placement + renaming via domtree walk (`--emit=ssa`)
- [ ] SSA verifier — every use dominated by its def, phi operand count matches predecessor count

### Middle-end — Loop analysis
- [ ] Natural loop detection (back-edges + dominator tree)
- [ ] Loop nesting tree — Havlak's algorithm (`--emit=loops`)
- [ ] Loop canonicalization — single preheader, single latch, dedicated exits

### Middle-end — Scalar optimizations
- [x] Dead code elimination (DCE)
- [ ] Constant folding and algebraic simplification

### Middle-end — Loop optimizations
- [ ] LICM — loop-invariant code motion

### Backend — LLVM IR emitter
- [x] Type lowering (all values → `i64`; alloca slots → `i64*` in pre-SSA LLVM IR)
- [x] Phi node lowering (operands emitted as `[ value, %label ]` pairs)
- [x] LLVM IR text emitter (`--emit=llvm`)
- [x] Invoke `clang -O0` on the emitted `.ll` to output binary 
### Debug and tooling
- [ ] `--explain` — trace pass decisions (phi placement sites, DCE removals, SCCP lattice states)
- [x] `make dot` renders all DOT outputs to PNG

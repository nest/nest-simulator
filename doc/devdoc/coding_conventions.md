# Coding Conventions {#devdoc_coding_conventions}

This page describes the coding style and naming conventions used in the NEST
C++ codebase. All contributions should follow these guidelines to keep the code
consistent and readable.

## Formatting

- **Indentation:** 2 spaces. No tabs.
- **Line length:** 120 characters maximum.
- **Braces:** Opening brace on the same line as the control statement or
  function signature.
- **Blank lines:** One blank line between member function definitions in a
  `.cpp` file. Two blank lines between free functions.
- Formatting is enforced by `clang-format`. Run it before committing:
  ```
  clang-format -i <file>
  ```
  The configuration is in `.clang-format` at the repository root.

## Naming

### C++ identifiers

| Entity | Convention | Example |
|---|---|---|
| Types (classes, structs, enums) | `PascalCase` | `NodeManager` |
| Functions and methods | `snake_case` | `get_node_id()` |
| Member variables | `snake_case_` (trailing underscore) | `node_id_` |
| Constants and enumerators | `UPPER_SNAKE_CASE` | `MAX_THREAD_NUM` |
| Template parameters | `PascalCase` | `ValueT` |
| Namespaces | `snake_case` | `nest::` |
| Preprocessor macros | `UPPER_SNAKE_CASE` with `NEST_` prefix | `NEST_ASSERT` |

### Biophysical parameter names

Model parameters should follow the notation of Dayan & Abbott (2001) where
available, using subscripts indicated by underscores.

| Parameter | Symbol |
|-----------|--------|
| Membrane potential | `V_m` |
| Resting potential | `E_L` |
| Input current | `I_e` |
| Leak current | `I_L` |
| Threshold | `V_th` |
| Reset potential | `V_reset` |
| Capacity / specific capacitance | `c_m` |
| Capacitance | `C_m` |
| Membrane time constant | `tau_m` |
| Synapse time constant | `tau_syn` |
| Refractory period | `t_ref` |
| Time of last spike | `t_spike` |
| Excitatory reversal potential | `E_ex` |
| Inhibitory reversal potential | `E_in` |
| Conductance | `g` |
| Leak conductance | `g_L` |
| Sodium conductance | `g_Na` |
| Sodium reversal potential | `E_Na` |
| Potassium conductance | `g_K` |
| Potassium reversal potential | `E_K` |

Common subscript conventions: `m` membrane, `L` leak, `e` extern, `th`
threshold, `syn` synapse, `ref` refractory, `ex` excitatory, `in` inhibitory.

## File Organization

- Header files: `.h` extension, in the same directory as the corresponding
  `.cpp`.
- One class per file (with rare justified exceptions).
- Include guards use `#ifndef` / `#define` with the pattern
  `NEST_<FILENAME>_H`.
- Include order (separated by blank lines):
  1. Corresponding header (in `.cpp` files)
  2. C++ standard library headers
  3. Third-party library headers (e.g., GSL, MPI)
  4. NEST project headers

## Documentation Comments

All public API must be documented with Doxygen comments. Use Doxygen-style
comments in header (`.h`) files only — avoid them in `.cpp` files. Do not
duplicate the code in comments.

### Functions and classes (multi-line)

```cpp
/**
 * Brief one-line description.
 *
 * Longer description if needed. Explain parameters, preconditions, and
 * any non-obvious behavior.
 *
 * @param node_id  The local node identifier.
 * @returns        Pointer to the node, or nullptr if not found.
 */
Node* get_node( index node_id );
```

Functions and classes should use the multi-line style even for a single-line
comment. Use `@param`, `@returns`, `@throws`, `@note`, `@see` as appropriate.

### Member variables

```cpp
//! Brief description of the variable.
long max_delay_;

long max_delay_; //!< Brief inline description (for short annotations).
```

### Implementation comments

Implementation details that are not part of the public contract belong in
regular `//` comments inside the function body.

## Error Handling

- Use `NEST_ASSERT` (not `assert`) for internal invariants.
- Throw `nest::KernelException` or a derived type for recoverable errors
  originating from the kernel.
- Never use exceptions for normal control flow.

## Thread Safety

- Clearly document whether a function is safe to call concurrently from
  multiple threads.
- Prefer thread-local storage or per-thread data structures over locks where
  possible.
- Document which data structures are protected by which mutex.

## See Also

- \ref devdoc_architecture "Architecture Overview"
- \ref devdoc_design_decisions "Design Decisions"

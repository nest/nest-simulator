# Coding Conventions {#devdoc_coding_conventions}

This page describes the coding style and naming conventions used in the NEST
C++ codebase. All contributions should follow these guidelines to keep the code
consistent and readable.

## Formatting

- **Indentation:** 2 spaces. No tabs.
- **Line length:** 120 characters maximum. No trailing whitespace.
- **File length:** Maximum 2000 lines.
- **Braces:** Always use braces around blocks. Opening brace on its own line.
- **Blank lines:** One blank line between member function definitions in a
  `.cpp` file. Two blank lines between free functions.
- Formatting is enforced by `clang-format`. Run it before committing:
  ```
  clang-format -i <file>
  ```
  The configuration is in `.clang-format` at the repository root.
  Commit formatting changes separately from logic changes.

### Control structures

Space after the keyword; spaces inside the test parentheses; braces on their own line:

\code{.cpp}
if ( x > 0 )
{
  // code
}
else
{
  // code
}

switch ( i )
{
case 0:
  // code
default:
  // code
}
\endcode

### Operators

Binary operators surrounded by one space: `a + b`.
Unary operators: no space between operator and operand: `-a`.
Use `not` instead of `!` — the negation operator is easily overlooked:

\code{.cpp}
if ( not vec.empty() )  // preferred
if ( !vec.empty() )     // avoid
\endcode

No space before a semicolon: `return a + 3;`

### Function signatures (.cpp files)

Put a line break after the return type. Parameters either all fit on one line, or
each goes on its own line:

\code{.cpp}
inline void
nest::Stopwatch::print( const char* msg,
                        timeunit_t timeunit,
                        std::ostream& os ) const
{
  // code
}
\endcode

## C++ Language Features

- Use only ISO C++ language features. Prefer C++ library functions and containers
  (STL) over their C equivalents.
- Prefer C++ headers over their C equivalents (e.g. `<cstdio>` over `<stdio.h>`).
- Do not use `printf` and related functions; use `std::cout` / `std::cerr`.
- Use C++ cast notation (`static_cast`, `dynamic_cast`, `const_cast`,
  `reinterpret_cast`). Never use C-style casts.
- Use the `const` qualifier wherever appropriate, and use it consistently.
- Use namespaces and exceptions.
- Avoid static class members that require a constructor (non-POD types).
- Use `enum` for integer constants rather than `#define`.
- In kernel code, use the NEST type aliases defined in `nest.h`
  (e.g. `nest::float_t` instead of `float`).

## Naming

### General

- All identifiers, class names, function names, and comments must be written
  in English.
- File names and folder names use `lower_case_under_lined` notation.
  C/C++ header files use the `.h` extension; C++ implementation files use `.cpp`.

### C++ identifiers

| Entity | Convention | Example |
|---|---|---|
| Types (classes, structs, enums) | `PascalCase` | `NodeManager` |
| Private nested classes/structs | `PascalCase_` (trailing underscore) | `State_` |
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

- Header files: `.h` extension, in the same directory as the corresponding `.cpp`.
- One class per file (with rare justified exceptions).
- All files begin with the NEST copyright preamble.
- Include guards use `#ifndef` / `#define` with the pattern `<FILENAME>_H`
  (e.g. `IAF_COND_ALPHA_H`).
- Include order (separated by blank lines, alphabetical within each group):
  1. Corresponding header (in `.cpp` files)
  2. C system headers
  3. C++ standard library headers
  4. Third-party library headers (e.g., GSL, MPI)
  5. NEST project headers
- **Include What You Use:** include every header that defines a symbol you rely
  on directly. Do not rely on transitive inclusion through other headers.

### Namespaces

- All NEST kernel symbols live in the `nest` namespace.
- Do not use `using namespace` in header files.
- Do not indent the body of a namespace.
- Follow the closing brace with a comment:

\code{.cpp}
namespace nest
{
// code
} // namespace nest
\endcode

### Classes and structs

- Use `struct` only for passive objects that carry data; use `class` for everything else.
- Access modifiers (`public`, `protected`, `private`) are not indented.
- Do not implement methods inside the class definition. Small `inline` methods
  go after the class definition; other methods go in the `.cpp` file.
- Template keyword and parameter list appear on a separate line; spaces inside
  angle brackets:

\code{.cpp}
template< typename T >
class MyClass : public T
{
public:
  // code
private:
  // code
};
\endcode

## Documentation Comments

All public API must be documented with Doxygen comments. Use Doxygen-style
comments in header (`.h`) files only — avoid them in `.cpp` files. Do not
duplicate the code in comments.

### Functions and classes (multi-line)

~~~
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
~~~

Functions and classes should use the multi-line style even for a single-line
comment. Use `@param`, `@returns`, `@throws`, `@note`, `@see` as appropriate.

### Member variables

~~~
//! Brief description of the variable.
long max_delay_;

long max_delay_; //!< Brief inline description (for short annotations).
~~~

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

- \ref devdoc_workflow "Developer documentation workflow"
- \ref devdoc_architecture "Architecture Overview"
- \ref devdoc_design_decisions "Design Decisions"

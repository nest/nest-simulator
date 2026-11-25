# Design Decisions {#devdoc_design_decisions}

This page records significant design and architectural decisions made during
NEST development. Each entry states the context, the options considered, the
decision taken, and the reasoning.

Use this log when a choice may otherwise seem puzzling to a new contributor, or
when the same question is likely to come up again.

---

## Template

Use the following structure when adding a new entry.

### [Short title of the decision]

**Date:** YYYY-MM-DD
**Status:** Accepted | Deprecated | Superseded by [link]

**Context:**
Describe the situation that required a decision. What problem needed solving?
What constraints existed?

**Options considered:**
1. Option A — brief description
2. Option B — brief description

**Decision:**
State the choice made.

**Rationale:**
Explain why this option was chosen over the alternatives.

**Consequences:**
Describe the impact of the decision: what becomes easier, what becomes harder,
what technical debt is incurred.

---

## Existing Decisions

<!-- Add entries below using the template above. -->


---

### Disentangle Kernel-Manager Include Tree (PR #3544)

**Date Created:** 2025-08-09
**Date Merged:**
**Status:** Accepted

**Context:**
`kernel_manager.h` included the headers of all subsystem managers directly.
This created a dense include graph that caused circular include dependencies
and prevented the compiler from inlining performance-critical functions.
Workarounds (`*_impl.h` files that re-included headers in a specific order)
were fragile and painful to maintain.

**Options considered:**
1. Reference-based managers — store each subsystem manager by reference in
   `KernelManager` and use forward declarations to break the include cycle.
2. Template-based inline globals — expose each manager through the variable
   template `kernel::manager< ManagerT >`, a reference to the global manager
   instance, so the full type is only needed at the call site.

**Decision:**
Template-based inline globals for manager access (option 2).

**Rationale:**
The template approach removed several of the manager-related `*_impl.h`
workaround files (for example `vp_manager_impl.h`, `io_manager_impl.h` and
`sp_manager_impl.h`) and allowed performance-critical accessor functions to be
inlined by the compiler. Other `*_impl.h` files remain where a complete type is
needed outside the `.cpp` file.
It also enforced Include What You Use (IWYU) discipline: each translation
unit must now explicitly include the header for every manager it uses,
making dependencies explicit. A benchmark comparing the two approaches showed
the reference-based option incurred a ~5% runtime slowdown; the template
approach recovered that regression.

**Consequences:**
250+ files were touched to add explicit manager includes. New code must
include the specific manager header it needs rather than relying on transitive
inclusion through `kernel_manager.h`. There was no external API change.

Performance-critical definitions were moved back into the manager headers
(see \ref devdoc_performance "Performance Guidelines").

Because the construction and destruction order of template-instantiated
globals is unspecified, the managers are not plain globals: their storage is
reserved at compile time, `kernel::create_managers()` constructs them in a
fixed order at the start of `init_nest()`, and they are never destroyed
(see \ref devdoc_kernel_managers "Kernel Managers").

**Reference:** [PR #3544](https://github.com/nest/nest-simulator/pull/3544)

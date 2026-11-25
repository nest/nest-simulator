# Developer Guides {#devdoc_index}

This section contains internal documentation for NEST developers. It covers
architecture decisions, design rationale, coding conventions, and other
material relevant to contributors working on the **NEST source code**. The entire
C++ codebase is rendered with Doxygen (see below).

Please ensure you are familiar with [all the relevant workflows for contributing
code and documentation](https://nest-simulator.readthedocs.io/en/latest/contribute/index.html#contribute-to-nest).

If you have ideas on improving these docs or if you find errors, please create an issue on GitHub!

## Contents

- \subpage devdoc_architecture "Architecture Overview" — High-level structure of
  the NEST simulator: subsystems, their responsibilities, and how they interact.

- \subpage devdoc_design_decisions "Design Decisions" — A log of significant
  architectural and API choices, with the context and reasoning behind them.

- \subpage devdoc_coding_conventions "Coding Conventions" — C++ style rules,
  naming schemes, and patterns used throughout the codebase.

- \subpage devdoc_include_guidelines "Include Guidelines" — The include-what-you-use
  principle, forward declarations, and checking includes.

- \subpage devdoc_kernel_managers "Kernel Managers" — How the kernel managers
  are accessed, constructed, and why their lifetime is controlled explicitly.

- \subpage devdoc_performance "Performance Guidelines" — Guidelines for
  performance-critical code, such as where to put inlined function definitions.

- \subpage devdoc_workflow "Developer Documentation Workflow" — How to write
  Doxygen comments, build docs locally, and review the CI artifact on pull requests.

### C++ code documentation

- \ref nest "nest namespace"
- [Class list](annotated.html)
- [All namespaces](namespaces.html)
- [File list](files.html)

# Performance Guidelines {#devdoc_performance}

This page collects guidelines for writing performance-critical C++ code in the
NEST kernel.

## Function inlining

Inlining performance-critical functions can have a large effect on performance,
as the additional function calls of non-inlined functions quickly become a
bottleneck. In general, however, it is recommended to put function definitions
in source files (`.cpp`) instead of header files (`.h`). There is therefore a
trade-off between performance and adhering to code style conventions. At the
same time, compilation times increase with more function definitions in header
files.

There are several solutions to this problem, including link-time optimization
(LTO) and explicitly marking function definitions in source files with the
`[[gnu::always_inline]]` attribute. Empirical measurements have shown, though,
that explicit inlining by moving the definitions into header files yields the
best performance in all cases and further removes the need for compiler-specific
attributes.

We therefore decided to put all performance-critical functions into header
files. Those functions are mostly located in the manager header files.

## See Also

- \ref devdoc_include_guidelines "Include Guidelines"
- \ref devdoc_kernel_managers "Kernel Managers"
- [Optimizing NEST for large-scale simulations](https://nest-simulator.readthedocs.io/en/latest/hpc/optimizing_nest.html)
  (user-facing guide on Read the Docs)

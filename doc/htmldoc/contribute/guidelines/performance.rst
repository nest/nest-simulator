.. _performance_guidelines:

Performance guidelines
======================

#. Function inlining
Inlining performance-critical functions can have a large effect on performance, as the additional function calls of
non-inlined functions quickly become a bottleneck. In general however, it is recommended to put function definitions in
source files (.cpp) instead of header files (.h). Therefore, there is a trade-off between performance and adhering to
code style conventions. At the same time, compilation times increase with more function definitions in header files.

There are several solutions to this problem, including using LTO (link-time optimization) and explicitly marking
function definitions in source files with the `[[gnu::always_inline]]` attribute. Empirical measurements have shown
though, that explicit inlining by moving the definitions into header files yields best performance in all cases and
further removes the need for compiler-specific attribute usage.
We therefore decided to put all performance-critical functions into header files. Those functions are mostly located in
the manager header files.

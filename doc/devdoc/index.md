# Developer Guides {#devdoc_index}

This section contains internal documentation for NEST developers. It covers
architecture decisions, design rationale, coding conventions, and other
material relevant to contributors working on the NEST source code.

## Contents

- \subpage devdoc_architecture "Architecture Overview" — High-level structure of
  the NEST simulator: subsystems, their responsibilities, and how they interact.

- \subpage devdoc_design_decisions "Design Decisions" — A log of significant
  architectural and API choices, with the context and reasoning behind them.

- \subpage devdoc_coding_conventions "Coding Conventions" — C++ style rules,
  naming schemes, and patterns used throughout the codebase.

## Contributing to This Documentation

Add or update pages in `doc/devdoc/` as markdown (`.md`) files. Each file
should begin with a level-1 heading followed by a Doxygen page label:

```markdown
# Page Title {#devdoc_my_page}
```

To nest the page under this index in the HTML tree nav, add it to this file
using `\subpage devdoc_my_page "Link text"`. For cross-references from other
pages (not intended as children), use `\ref devdoc_my_page "Link text"`.

Images go in `doc/devdoc/static/img/` and can be embedded with:

```markdown
![Alt text](static/img/my_image.png)
```

### Linking to C++ Symbols

From any markdown page you can link directly to C++ documentation using the
`\ref` command:

| Target | Syntax | Example |
|--------|--------|---------|
| Class | `\ref ClassName` | `\ref KernelManager` |
| Method | `\ref ClassName::method` | `\ref KernelManager::get_node` |
| Namespaced class | `\ref ns::ClassName` | `\ref nest::Node` |
| File page | `\ref path/to/file.h` | `\ref nestkernel/node.h` |
| `\defgroup` group | `\ref group_name` | `\ref user_interface` |

The path for file links must match what is listed in the Doxygen `INPUT`
setting relative to the source root (e.g. `nestkernel/kernel_manager.h`).
Group names are defined with `\defgroup` in header files and are used
extensively throughout the NEST kernel headers.

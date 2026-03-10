# Developer Documentation Workflow {#devdoc_workflow}

## What you need to know

For developer documentation of the C++ code, we use [Doxygen](http://doxygen.org/) comments
extensively throughout NEST. If you add or modify the code, please ensure you document your
changes with the correct Doxygen syntax.

Additional documentation for developers and contributors can be found on
[Read the Docs](https://nest-simulator.readthedocs.io/en/stable/developer_space/).

> **Note:** This workflow covers **developer documentation** (C++/Doxygen). For the
> **user documentation**, refer to the User documentation workflow on Read the Docs.

## GitHub Pages

The C++ developer documentation is deployed to GitHub Pages:

- https://nest.github.io/nest-simulator/index.html

You can view the latest master version along with the 2 most recent releases of NEST.

## GitHub workflow on pull request

If you create a pull request against `nest/nest-simulator` and have modified

- any C++ file (`*.cpp`, `*.h`),
- the Doxygen config file (`doc/fulldoc.conf.in`), or
- the Doxygen CSS file (`doc/developer_space/static/css/doxygen-awesome.css`),

then GitHub will build the C++ documentation and upload it as an artifact. You can download
and view the HTML pages locally.

### Where to find the artifact

1. Navigate to your pull request page on GitHub and select the **Checks** tab.
2. In the left column select the **Build and Deploy C++ Documentation** workflow
   (or wait for it to complete if it is still running).
3. This takes you to the workflow **Summary** page. In the **Artifacts** section at the
   bottom you will find a downloadable archive named `docs-<run_id>`.
4. Click the artifact name to download it as a ZIP file.
5. Extract the ZIP file and open `index.html` in your web browser.

> **Note:** If you modified `doc/fulldoc.conf.in` or `doc/developer_space/static/css/doxygen-awesome.css`
> in your PR, the artifact will reflect your modified configuration, allowing you to preview how
> the changes affect the documentation appearance.

## Local build

1. Install Doxygen and Graphviz.

   Linux:
   ```bash
   sudo apt install doxygen graphviz
   ```

   > **Note:** CI uses Doxygen **1.12.0**. The version shipped by `apt` is typically older
   > and may produce slightly different output. For exact parity with CI, download the
   > matching release from the [Doxygen GitHub releases page](https://github.com/doxygen/doxygen/releases/tag/Release_1_12_0).

   macOS ([Homebrew](https://brew.sh/)):
   ```bash
   brew install doxygen graphviz
   ```

2. Navigate to or create a `build` directory (see the NEST installation guide for details).

3. Add `-Dwith-devdoc=ON` to your CMake command:

   ```bash
   cmake -Dwith-devdoc=ON <path/to/source>
   ```

4. Generate HTML:

   ```bash
   make docs
   ```

5. Open the docs in a browser:

   The output is written to `<build>/doc/doxygen/html/`. From the build directory:
   ```bash
   xdg-open doc/doxygen/html/index.html   # Linux
   open doc/doxygen/html/index.html        # macOS
   ```

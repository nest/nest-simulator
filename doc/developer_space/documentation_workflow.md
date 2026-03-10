# Developer Documentation Workflow {#devdoc_workflow}

## Updating NEST source code (C++)

For developer documentation of the C++ code, we use [Doxygen](http://doxygen.org/) comments
extensively throughout NEST. If you add or modify the code, please ensure you document your
changes with the correct Doxygen syntax (see \ref devdoc_coding_conventions "Coding conventions").

If you want to update PyNEST (see [our Read the Docs pages](https://nest-simulator.readthedocs.io/en/latest/contribute/index.html)).

### Contribute to developer documentation

Add or update pages in `doc/developer_space/` as markdown (`.md`) files. Each file
should begin with a level-1 heading followed by a Doxygen page label:

```markdown
# Page Title {#devdoc_my_page}
```

To nest the page under this index in the HTML tree nav, add it to this file
using `\subpage devdoc_my_page "Link text"`. For cross-references from other
pages (not intended as children), use `\ref devdoc_my_page "Link text"`.

Images go in `doc/developer_space/static/img/` and can be embedded with:

```markdown
![Alt text](static/img/my_image.png)
```

### Linking to C++ Symbols

From any markdown page you can link directly to C++ documentation:

| Target | Syntax | Example |
|--------|--------|---------|
| Namespaced class | `ns::ClassName` | `nest::Node` |
| Method | `ns::ClassName::method()` | ` nest::SimulationManager::has_been_simulated()` |
| File page | ` path/to/file.h` | `nestkernel/node.h` |

The path for file links must match what is listed in the Doxygen `INPUT`
setting relative to the source root (e.g. `nestkernel/kernel_manager.h`).
Always qualify class and method names with their namespace; unqualified names
are not resolved from markdown pages. Method links require trailing `()`.

Additional documentation for developers and contributors can be found on
[Read the Docs](https://nest-simulator.readthedocs.io/en/latest/developer_space/),
including reviewer guidelines, git workflows etc.

## Documentation deployment: GitHub Pages

The C++ developer documentation is deployed to GitHub Pages:

- https://nest.github.io/nest-simulator/index.html

Note that these docs are re-built when a pull-request is merged into branch **main**, if
any of the following files were modified:

- any C++ file (`*.cpp`, `*.h`),
- the Doxygen config file (`doc/fulldoc.conf.in`),
- the Doxygen CSS file (`doc/developer_space/static/css/doxygen-awesome.css`), or
- any file under `doc/developer_space/` (including these markdown pages),


This means the docs can change at any time, as developers actively work on **main**.

## View the docs built on your pull request

If you create a pull request against `nest/nest-simulator` and have modified

- any C++ file (`*.cpp`, `*.h`),
- the Doxygen config file (`doc/fulldoc.conf.in`),
- the Doxygen CSS file (`doc/developer_space/static/css/doxygen-awesome.css`), or
- any file under `doc/developer_space/` (including these markdown pages),

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

> **Note:** The artifact reflects the full built output from your PR, including any changes to
> `doc/fulldoc.conf.in`, stylesheets, or markdown pages under `doc/developer_space/`.

## Local build

1. Install Doxygen and Graphviz.

   Linux:
   ```bash
   sudo apt install doxygen graphviz
   ```

   > **Note:** The version shipped by `apt` is typically older than what CI uses and may
   > produce slightly different output. For exact parity with CI, download the matching
   > release:
   > ```bash
   > DOXYGEN_VERSION=$(grep 'DOXYGEN_VERSION:' .github/workflows/cpp_docs.yml | awk '{print $2}')
   > DOXYGEN_TAG=$(echo "$DOXYGEN_VERSION" | tr '.' '_')
   > wget "https://github.com/doxygen/doxygen/releases/download/Release_${DOXYGEN_TAG}/doxygen-${DOXYGEN_VERSION}.linux.bin.tar.gz"
   > tar -xzf "doxygen-${DOXYGEN_VERSION}.linux.bin.tar.gz"
   > sudo install "doxygen-${DOXYGEN_VERSION}/bin/doxygen" /usr/local/bin/doxygen
   > ```

   macOS ([Homebrew](https://brew.sh/)):
   ```bash
   brew install doxygen graphviz
   ```

2. Navigate to or create a `build` directory (see the NEST installation guide for details).

3. Add `-Dwith-devdoc=ON` to your CMake command:

   ```bash
   cmake -Dwith-devdoc=ON <path/to/source>
   ```

4. **Optional — render PlantUML diagrams.**

   Without this step, `\startuml`...`\enduml` blocks (e.g. the subsystem diagram in
   `architecture.md`) will be silently skipped and left blank in the output.

   a. Install a Java runtime (needed to run PlantUML):

      ```bash
      sudo apt install default-jre-headless   # Linux
      brew install openjdk                     # macOS
      ```

   b. Download the PlantUML jar, using the same version as CI:

      ```bash
      PLANTUML_URL=$(grep 'PLANTUML_JAR_URL:' .github/workflows/cpp_docs.yml | awk '{print $2}')
      wget "$PLANTUML_URL" -O plantuml.jar
      ```

   c. After the `cmake` step, set `PLANTUML_JAR_PATH` in the generated config:

      ```bash
      sed -i "s|^PLANTUML_JAR_PATH.*|PLANTUML_JAR_PATH      = $(pwd)/plantuml.jar|" build/doc/fulldoc.conf
      ```

5. Generate HTML:

   ```bash
   make docs
   ```

6. Open the docs in a browser:

   The output is written to `<build>/doc/doxygen/html/`. From the build directory:
   ```bash
   xdg-open doc/doxygen/html/index.html   # Linux
   open doc/doxygen/html/index.html        # macOS
   ```

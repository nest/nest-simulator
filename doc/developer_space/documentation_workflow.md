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

Note that you are viewing the documentation in the main branch, which has
active development.

## GitHub workflow on pull request

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

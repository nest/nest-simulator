# NEST Simulator Installation

## Installation Summary

NEST Simulator has been successfully installed on this system.

### Installation Details

- **NEST Version**: 3.9.0-post0.dev0
- **Build Date**: Oct 21 2025 13:45:27
- **Platform**: x86_64-pc-linux (Linux 4.4.0)
- **Installation Directory**: /home/user/nest-install
- **Build Directory**: /home/user/nest-build
- **Source Directory**: /home/user/nest-simulator

### Build Configuration

- **Python bindings**: Enabled (Python 3.11.14)
- **Threading**: OpenMP enabled
- **MPI**: Not compiled
- **GSL**: Version 2.7.1
- **Readline**: Version 8.2
- **Boost**: Version 1.83.0
- **Built-in modelset**: full

### Environment Setup

To use NEST in your shell session, source the environment variables script:

```bash
source /home/user/nest-install/bin/nest_vars.sh
```

This script sets:
- PATH to include NEST executables
- PYTHONPATH for PyNEST module
- LD_LIBRARY_PATH for NEST libraries

### Installed Components

1. **NEST executable**: `/home/user/nest-install/bin/nest`
2. **PyNEST**: `/home/user/nest-install/lib/python3.11/site-packages/nest`
3. **Libraries**: `/home/user/nest-install/lib/nest/`
4. **Headers**: `/home/user/nest-install/include/nest/`
5. **Examples**: `/home/user/nest-install/share/doc/nest/examples/`
6. **SLI libraries**: `/home/user/nest-install/share/nest/sli/`

### Testing

#### NEST Command-Line
```bash
source /home/user/nest-install/bin/nest_vars.sh
nest --version
```

#### PyNEST
```bash
source /home/user/nest-install/bin/nest_vars.sh
python3 -c "import nest; print(nest.__version__)"
```

### Post-Installation Notes

1. NumPy was reinstalled using pip to resolve compatibility issues with the system package.
2. Some test suite warnings may appear related to numpy/scipy version compatibility, but these do not affect NEST functionality.
3. The test suite completed with 116 of 117 tests passing (1 known regression test issue).

### Dependencies Installed

System packages installed via apt:
- cmake, gsl-bin, libgsl-dev
- libboost-dev, cython3
- libreadline-dev, python3-all-dev
- python3-numpy, python3-scipy, python3-matplotlib
- python3-nose, python3-junitparser
- ipython3, python3-future
- openmpi-bin, libopenmpi-dev, python3-mpi4py
- python3-pip, python3-pytest
- python3-pytest-timeout, python3-pytest-xdist
- python3-pandas

Python packages installed via pip:
- numpy 2.3.4 (upgraded from system package)

### Additional Information

For more information about NEST:
- Documentation: https://nest-simulator.org/documentation
- Community: https://www.nest-simulator.org/community
- Issues: https://github.com/nest/nest-simulator/issues

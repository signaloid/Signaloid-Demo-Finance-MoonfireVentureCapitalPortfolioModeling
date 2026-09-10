# Source code:

## main.c
Top-level driver: parses command-line arguments, dispatches to the UxHw or
Monte Carlo kernel, and prints the results.

## kernel.c/h
Mode-dispatch layer shared by both execution modes: declares the
`calculateOutputUxHw()` / `calculateOutputMonteCarlo()` entry points called
from `main.c`, and the output-index-driven logic for computing the selected
output(s).

## moonfire-venture-capital-portfolio-uxhw.c/h
UxHw-mode implementation of the portfolio model: computes the portfolio
return distribution and the derived probability-of-loss/quantile outputs
using the UxHw API.

## moonfire-venture-capital-portfolio-monte-carlo.c/h
Native Monte Carlo implementation of the portfolio model: draws samples for
the portfolio return and computes the derived probability-of-loss/quantile
outputs from the sample set.

## utilities.c/h
These contain utility methods for parsing, setting, and reporting
the usage of demo-specific command-line arguments of C/C++ demo applications.
These methods call similar methods from `common.c` for handling
command-line arguments common to all of our C/C++ demo applications.

## common.c/h
These contain utility methods for parsing, setting, and reporting
the usage of command-line arguments common to all of our C/C++ demo applications,
as well as other methods that we commonly use across our
C/C++ demo applications, e.g., standard methods for I/O handling. These
source files are symlinks to the original files contained in the repository
[Signaloid-Demo-CommonUtilityRoutines](https://github.com/signaloid/Signaloid-Demo-CommonUtilityRoutines)
which is included as a submodule in `submodules/common`.

## uxhw.c/h
These contain methods that implement the probabilistic versions of the methods
in the UxHw API (e.g., `UxHwDoubleGaussDist`) and uses the GNU Scientific Library (GSL)
random number generators to achieve that. This allows building our C/C++ demo applications
natively (i.e., on conventional architectures) and running native Monte Carlo evaluations
of our C/C++ demo applications without modifying the source code.
These source files are symlinks to the original files and are contained in the repository
[Signaloid-Demo-UxHwCompatibilityForNativeExecution](https://github.com/signaloid/Signaloid-Demo-UxHwCompatibilityForNativeExecution)
which is included as a submodule in `submodules/compat`.

## config.mk
Signaloid cores use this file to identify the source codes they will use when
building the C/C++ demo application.

# To Build Natively on Non-Signaloid Platforms

From the repository root (not from `src/`), run:
```
make local-build
```
This builds all of the sources listed in `config.mk` (plus `uxhw.c`) and
produces the `demo-native-mc` executable at the repository root. See the
[Prerequisites](../README.md#prerequisites) section of the root `README.md`
for required dependencies.

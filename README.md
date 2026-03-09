# fprime-vorago

This repository contains common components and support software for the Vorago [VA41620][1]/[VA41630][2] microcontrollers running with no RTOS (i.e. on the bare metal). We anticipate the possibility of expanding this repository to support RTOSes and other Vorago microcontrollers.

This repository is maintained by a development group within JPL. Contact [Cel Skeggs][3] for more information.

 [1]: https://www.voragotech.com/products/microcontrollers/arm-cortex-m4-family/va41620
 [2]: https://www.voragotech.com/products/microcontrollers/arm-cortex-m4-family/va41630
 [3]: mailto:cel.a.skeggs@jpl.nasa.gov

Documentation may be available from the vendor. Check with your organization to see if you already have a copy of the documentation.

## Development Container

This project relies on [a Clang/LLVM ARM Toolchain specialized for the VA41630][4];

 [4]: https://github.com/fprime-community/llvm-vorago-arm-toolchain

We suggest building a development container for your project based on the
following prebuilt image:

    FROM ghcr.io/fprime-community/llvm-vorago-arm-toolchain:lvat-20.1.0.0

(If you do not need any additional software beyond what is already contained in
this image, you can use it directly.)

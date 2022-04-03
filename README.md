# Projet OpenCL

## Members
 - WALCAK Ladislas
 - QUETIER Thomas

## Useful variables
Inside the [CMake](CMakeLists.txt) file, you can find the following variables:
 - `filename`: Specify the text file to use which contains the terrain data. The file should be added to the `others` list to copy it to the build directory.  
    Depending on the size of the terrain, you should adjust the `GLOBAL_SIZE` and `LOCAL_SIZE` variables inside the [OpenCL version](opencl.cpp).
 - CMake `VERBOSE` flag: If set to `ON`, the CMake will print debug information and for the OpenCL version, will output the results into an external file.  
   You can change the name of the output file by setting the `OUTFILE` variable inside the [OpenCL version](opencl.cpp).
   Defaulted to `ON` on debug mode, and `OFF` in release mode.

## Custom targets
Two CMake targets are available by default (when using an IDE), to compile the two versions of the program:
 - `main`: Compile the CPU version.
 - `opencl`: Compile the GPU version.

An additional custom target `valgrind` is available to run the program with valgrind, for debug purposes.  
This target requires the `valgrind` package to be installed, and support the `--show-error-list=yes` flag (older version doesn't).
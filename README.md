## Building and Running Threaded Raytracing

Clone and then navigate to `./mpi-raytrace`

`cmake -S . -B build --preset=cpp-threads`
`cmake --build build`

Note that the program outputs the image to stdout.

`build/mpi-raytrace > image.ppm`

Optional arguments usable with `build/mpi-raytrace`

-w<UINT>: width of final image (default = 1920)
-h<UINT>: height of final image (default = 1080)
-r<UINT>: rays fired out of each pixel (default = 32)
-t<UINT>: number of threads executing the algorithm (default = std::thread::hardware_concurrency())

Example
`build/mpi-raytrace -w1280 -h720 -r2 -t4 > image.ppm`

## Building and Running MPI Raytracing

Clone and then navigate to `./mpi-raytrace`

`cmake -S . -B build --preset=mpi`
`cmake --build build`

`mpiexec build/mpi-raytrace > image.ppm`

-w<UINT>: width of final image (default = 1920)
-h<UINT>: height of final image (default = 1080)
-r<UINT>: rays fired out of each pixel (default = 32)
-n<UINT> = number of processes executing the algorithm (defualt = 1)

`mpiexec -nD build/mpi-raytrace -wA -hB -rC`

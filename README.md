# RealTimeRayTracing
A quick attempt to make Real Time Ray Tracing 

This is a Work in progress.

This Qt project should be a multiplatform Ray Tracing. It will use Qt 5.4 to create
GUI and also OpenGL context.

OpenCL 1.x will be used to parallelize the proccessing in order to reach (near) real time rendering.

For now, It has some boilerplate codes, few (not automated) tests and ground base for the algorithms.

Summary of technologies:

-GLM. Library for common computer graphics math.
-Qt 5.4. Qt Framework, the makefile should be generated with qmake, and we need its GUI library.
-OpenCL 1.x. Multiplatform parallel computations.

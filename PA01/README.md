A simple example of matrix use in OpenGL
========================================

Changes made
------------
Put the vertex shader and fragment shader in separate files.

Added a shader loader function that gets the shaders from the files.

Made the cube rotate with the glm function rotate().

Problems
--------
Had problems uploading files to github.

Building This Example
---------------------

*This example requires GLM*
*On ubuntu it can be installed with this command*

>$ sudo apt-get install libglm-dev

To build this example just 

>$ cd build
>$ make

The excutable will be put in bin

Mac OS X Things
---------------

*Mac OS X requires some changing of the headers*

*Also std::chrono may or may not work on OS X*

*Should that be the case use gettimeofday*

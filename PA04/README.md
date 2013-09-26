**NOTE**
----------
Used the tutorial from: 

http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/

This tutorial helped me write my loadObj function and getFileSize function


Changes made
------------
Took out rotation and orbit

Added Model Loader to load .obj file

Added command line arguments(only takes the executable and file name)

Added all .obj conditions/cases

**DID NOT DO QUADS 
 

Problems
--------
Had problems reading the object file

Had problems setting the vertices to the faces

Had problems with reading the file from the command prompt

Had problems getting the size of the different cases

Had problems writing the other cases


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

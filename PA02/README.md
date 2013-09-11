Changes made
------------
Added pull down menus when right click on mouse.
With pull down menu, you can start and stop the rotation of the cube.
When left click reverse rotation of the cube.
When 'R' or 'r' is pressed this also reverses the rotation of the cube.

Problems
--------
Had problems with starting and stoping the cube smoothly.
Had problems with reversing the rotation of the cube.

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

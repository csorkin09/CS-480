# Linux
CC=g++
LIBS= -lglut -lGLEW -lGL -lGLU -lassimp -I/usr/include/ImageMagick -lMagick++ -I/usr/local/include/bullet -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lSDL -lSDL_mixer

# For Macs uncomment the next line and comment out the previous one
#CC=clang++
#LIBS=  -framework OpenGL -framework GLUT -framework Cocoa -lGLEW -stdlib=libc++
# Assuming you want to use a recent compiler

# Compiler flags
CXXFLAGS= -g -W -std=c++0x

all: ../bin/Matrix

../bin/Matrix: ../src/main.cpp
	$(CC) $(CXXFLAGS) ../src/main.cpp ../src/mesh.cpp ../src/texture.cpp ../src/bullet.cpp ../src/gameManager.cpp ../src/ai.cpp ../src/audio.cpp -o ../bin/Matrix $(LIBS)

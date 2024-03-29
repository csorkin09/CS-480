//Assignment 4
#define _USE_MATH_DEFINES // Windows defines because I'm a slave to the man
#define _CRT_SECURE_NO_WARNINGS
#define MAX_SHADER_STR_LEN 1000
#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/glut.h> // doing otherwise causes compiler shouting
#include <GL/freeglut.h> // doing otherwise causes compiler shouting
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/color4.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include "mesh.h"
#include "bullet.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier
#include <vector>


#define NUM_OBJECTS 4
//--Evil Global variables
//Just for this example!
int w = 640, h = 480;// Window size
double rotationRate = M_PI/2; //radians
double spinRate = 90; //degrees
double orbitRate = 5;

GLuint program;// The GLSL program handle
GLuint vbo_geometry;// VBO handle for our geometry
GLuint vbo_array[NUM_OBJECTS];
GLuint vbo_geometry2;// VBO handle for our geometry
//uniform locations
GLint loc_mvpmat;// Location of the modelviewprojection matrix in the shader

//attribute locations
GLint loc_position;
GLint loc_color;

// Defines
#define MAX_PATH_LEN 200

//std::vector<unsigned int> vbo_index;
//transform matrices
glm::mat4 model[NUM_OBJECTS];//obj->world each object should have its own model matrix
glm::mat4 view;//world->eye
glm::mat4 projection;//eye->clip
glm::mat4 mvp[NUM_OBJECTS];//premultiplied modelviewprojection
glm::mat4 mvpMoon;//premultiplied modelviewprojection
CMesh mesh[NUM_OBJECTS];
CBullet bullet;

//Function Prototypes
void render();
void update();
void reshape(int n_w, int n_h);
void mouse(int button, int state, int x, int y);
void keyboard(unsigned char key, int x_pos, int y_pos);
void keyboardSpecial(int key, int x_pos, int y_pos);
void demo_menu(int id);
void idle();
bool initialize(char path[MAX_PATH_LEN]);
void cleanUp();
bool loadOBJ(Vertex geometry[], char path[MAX_PATH_LEN]);
char* shaderLoader(const char* shaderFile);
Vertex* geometry;

//--Random time things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;

	


void idle()
{
	glutPostRedisplay();
}

void demo_menu(int id)
{
	switch(id)
	{
	case 1:
		exit(0);
		break;
	case 2: //start rotation
		rotationRate = M_PI/2;
		break;
	case 3: //stop rotation
		rotationRate = 0;
		break;
	case 4: // start spinning
		spinRate = 90.0;
		break;
	case 5: // end spinning
		spinRate = 0.0;
		break;
	}
	glutPostRedisplay();
}



//--Main
int main(int argc, char **argv)
{
	char filePath[MAX_PATH_LEN];

    // Initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    // Name and create the Window
    glutCreateWindow("Matrix Example");

	glutCreateMenu(demo_menu);
	glutAddMenuEntry("quit", 1);
	glutAddMenuEntry("start rotation", 2);
	glutAddMenuEntry("stop rotation", 3);
	glutAddMenuEntry("start spinning", 4);
	glutAddMenuEntry("stop spinning", 5);
	glutAttachMenu(GLUT_LEFT_BUTTON);


    // Now that the window is created the GL context is fully set up
    // Because of that we can now initialize GLEW to prepare work with shaders
    GLenum status = glewInit();
    if( status != GLEW_OK)
    {
        std::cerr << "[F] GLEW NOT INITIALIZED: ";
        std::cerr << glewGetErrorString(status) << std::endl;
        return -1;
    }

    // Set all of the callbacks to GLUT that we need
    glutDisplayFunc(render);// Called when its time to display
    glutReshapeFunc(reshape);// Called if the window is resized
    glutIdleFunc(update);// Called if there is nothing else to do
    glutKeyboardFunc(keyboard);// Called if there is keyboard input
	glutSpecialFunc(keyboardSpecial);
	glutMouseFunc(mouse); // Called if there is mouse input

	//Get command line file path parameters
	if(argc > 1){
		strcpy(filePath, argv[1]);
	}
	else{ // no file use default
		#ifdef _WIN32
			sprintf(filePath, "C:/Users/Blindo/git/480-PA1/Assignment06/assets/buddha.obj");
		#else
			sprintf(filePath, "../assets/buddha.obj");
		#endif
	}

	std::cout << "Using file: " << filePath;

    // Initialize all of our resources(shaders, geometry)
    bool init = initialize(filePath);
   if(init)
    {
        t1 = std::chrono::high_resolution_clock::now();
        glutMainLoop();
   }

    // Clean up after ourselves

}	

//--Implementations
void render()
{
	//clear the screen
	glClearColor(0.0, 0.0, 0.2, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//enable the shader program
	glUseProgram(program);

	//set up the Vertex Buffer Object so it can be drawn
	glEnableVertexAttribArray(loc_position);
	glEnableVertexAttribArray(loc_color);
	
	for(int meshIndex=0; meshIndex<NUM_OBJECTS; meshIndex++){
		glBindBuffer(GL_ARRAY_BUFFER, vbo_array[meshIndex]); 
		//set pointers into the vbo for each of the attributes(position and color)
		
		glVertexAttribPointer( loc_position,//location of attribute
				    3,//number of elements
				    GL_FLOAT,//type
				    GL_FALSE,//normalized?
				    sizeof(Vertex),//stride
				    0);//offset
	
		glVertexAttribPointer( loc_color,
				    2,
				    GL_FLOAT,
				    GL_FALSE,
				    sizeof(Vertex),
				    (void*)offsetof(Vertex,color));
	
	
	     //upload the matrix to the shader
	     glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp[meshIndex]));
	     //premultiply the matrix for this example
	     mvp[meshIndex] = projection * view * model[meshIndex];
	     
	     for(unsigned int j=0;j<mesh[meshIndex].indecies.size() - 1;j++){
		     if(mesh[meshIndex].materialIndex.size() > j){
			unsigned int index = mesh[meshIndex].materialIndex[j];
			if(index < mesh[meshIndex].m_Textures.size() && mesh[meshIndex].m_Textures[index]){
				glBindTexture(GL_TEXTURE_2D, mesh[meshIndex].m_Textures[index]->m_textureObj);
			}
		     }		     
		     glDrawArrays(GL_TRIANGLES, mesh[meshIndex].indecies[j], mesh[meshIndex].indecies[j+1]-mesh[meshIndex].indecies[j]);//mode, starting index, count
	     }
	}
/*
	//glActiveTexture(GL_TEXTURE0);
		for(int i=0;i<mesh.indecies.size() - 1;i++){
		if(mesh.materialIndex.size() > i){
			int index = mesh.materialIndex[i];
			if(index < mesh.m_Textures.size() && mesh.m_Textures[index]){
				glBindTexture(GL_TEXTURE_2D, mesh.m	
				_Textures[index]->m_textureObj);
			}
		}
		glDrawArrays(GL_TRIANGLES, mesh.indecies[i], mesh.indecies[i+1]-mesh.indecies[i]);//mode, starting index, count
	 }
	*/
	//clean upshader_stat
	glDisableVertexAttribArray(loc_position);
	glDisableVertexAttribArray(loc_color);
		           
	//swap the buffers
	glutSwapBuffers();
}

void update()
{
	//total time
	static float spinAngle = 0.0;
	static float rotateAngle = 0.0;
	static float orbitAngle = 0.0;
	float dt = getDT();// if you have anything moving, use dt
	spinAngle += dt * rotationRate; //move through 90 degrees a second
	rotateAngle += dt*spinRate;
	orbitAngle += dt*orbitRate;

	// Update Fizzucks
	bullet.update(dt);
	
	// Sphere
	//model[3] = glm::translate(glm::mat4(1.0f), glm::vec3(4.0 * sin(spinAngle), 0.0, 4.0 * cos(spinAngle)));
	//model[3]=  glm::rotate(model[0], rotateAngle, glm::vec3(0.0, 1.0f, 0.0));
	model[3] = glm::translate(glm::mat4(1.0f), glm::vec3(bullet.sphere.x, bullet.sphere.y, bullet.sphere.z));
	
	// Box
	//model[1]= glm::translate(glm::mat4(1.0f),  glm::vec3(4.0 * sin(spinAngle), 0.0, 4.0 * cos(spinAngle)));
	//model[1] = glm::translate(model[1], glm::vec3(3.0 * sin(orbitAngle), 0.0, 3.0 * cos(orbitAngle)));
	//model[1] = glm::scale(model[1],glm::vec3(0.5f,0.5f,0.5f));
	model[1] = glm::translate(glm::mat4(1.0f), glm::vec3(bullet.box.x,bullet.box.y, bullet.box.z));
	
	// Cylinder
	//model[2]= glm::translate(glm::mat4(1.0f),  glm::vec3(0.0, 4.0 * sin(spinAngle), 4.0 * cos(spinAngle)));
	//model[2]=  glm::rotate(model[2], rotateAngle, glm::vec3(1.0f, 0.0f, 1.0f));
	model[2] = glm::translate(glm::mat4(1.0f), glm::vec3(bullet.cylinder.x,bullet.cylinder.y, bullet.cylinder.z));
	
	model[0] = glm::translate(glm::mat4(1.0f), glm::vec3(bullet.board.x,bullet.board.y, bullet.board.z));
	
	// Update the state of the scene
	glutPostRedisplay();//call the display callback
}


bool initialize(char path[MAX_PATH_LEN])
{    
    mesh[0].loadMesh("./board.obj");
    mesh[1].loadMesh("./box.obj");
    mesh[3].loadMesh("./sphere.obj");
    mesh[2].loadMesh("./cylinder.obj");
    
    for(int i=0;i<NUM_OBJECTS;i++){
    	glGenBuffers(1, &vbo_geometry);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*mesh[i].numVertex, mesh[i].geometry, GL_STATIC_DRAW);
	vbo_array[i] = vbo_geometry;
    
    }
	
    // Create a Vertex Buffer object to store this vertex info on the GPU

    //--Geometry done
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    const char *vs = shaderLoader("vs.txt");

    const char *fs = shaderLoader("fs.txt");

    //compile the shaders
    GLint shader_status;

    // Vertex shader first
    glShaderSource(vertex_shader, 1, &vs, NULL);
    glCompileShader(vertex_shader);
    //check the compile status
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] FAILED TO COMPILE VERTEX SHADER!" << std::endl;
        return false;
    }
    else{
		std::cout << "Compiled Vertex Shader" << std::endl;
	}

    // Now the Fragment shader
    glShaderSource(fragment_shader, 1, &fs, NULL);
    glCompileShader(fragment_shader);
    //check the compile status
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] FAILED TO COMPILE FRAGMENT SHADER!" << std::endl;
        return false;
    }else{
	cout << "Compiled Fragment Shader" << std::endl;
    }

    //Now we link the 2 shader objects into a program
    //This program is what is run on the GPU
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    
    //check if everything linked ok
    glGetProgramiv(program, GL_LINK_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] THE SHADER PROGRAM FAILED TO LINK" << std::endl;
        return false;
    }

    //Now we set the locations of the attributes and uniforms
    //this allows us to access them easily while rendering
    loc_position = glGetAttribLocation(program,
                    const_cast<const char*>("v_position"));
    if(loc_position == -1)
    {
        std::cerr << "[F] POSITION NOT FOUND" << std::endl;
        return false;
    }

    loc_color = glGetAttribLocation(program,
                    const_cast<const char*>("v_color"));
    if(loc_color == -1)
    {
        std::cerr << "[F] V_COLOR NOT FOUND" << std::endl;
        return false;
    }

    loc_mvpmat = glGetUniformLocation(program,
                    const_cast<const char*>("mvpMatrix"));
    if(loc_mvpmat == -1)
    {
        std::cerr << "[F] MVPMATRIX NOT FOUND" << std::endl;
        return false;
    }
    
    //--Init the view and projection matrices
    //  if you will be having a moving camera the view matrix will need to more dynamic
    //  ...Like you should update it before you render more dynamic 
    //  for this project having them static will be fine
    view = glm::lookAt( glm::vec3(0.0, 8.0, -16.0), //Eye Position
                        glm::vec3(0.0, 0.0, 0.0), //Focus point
                        glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up

    projection = glm::perspective( 45.0f, //the FoV typically 90 degrees is good which is what this is set to
                                   float(w)/float(h), //Aspect Ratio, so Circles stay Circular
                                   0.01f, //Distance to the near plane, normally a small value like this
                                   100.0f); //Distance to the far plane, 

    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //and its done
    return true;
}



void reshape(int n_w, int n_h)
{
    w = n_w;
    h = n_h;
    //Change the viewport to be correct
    glViewport( 0, 0, w, h);
    //Update the projection matrix as well
    //See the init function for an explaination
    projection = glm::perspective(45.0f, float(w)/float(h), 0.01f, 100.0f);

}
void mouse(int button, int state, int x, int y)
{
	if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN){
		rotationRate = rotationRate * -1;
	}
}

void keyboardSpecial(int key, int x_pos, int y_pos)
{
    if(key == GLUT_KEY_LEFT){ 
		bullet.sphere.force(5,0,0);
	}
	else if(key == GLUT_KEY_RIGHT){ 
		bullet.sphere.force(-5.0,0,0);
	}
	else if(key == GLUT_KEY_UP){ 
		bullet.sphere.force(0,0,5);
	}
	else if(key == GLUT_KEY_DOWN){
		bullet.sphere.force(0,0,-5);
	}
}



void keyboard(unsigned char key, int x_pos, int y_pos)
{
    if(key == 27){
        exit(0);
    }
}

char* shaderLoader(const char* shaderFile){
	//opens files
	std::ifstream file;
	file.open(shaderFile, std::ios::in);
	//our temporary buffer to hold file content
	char* buffer;
	if (!file){
		//minor error check
		std::cerr << "File Fail To Open" << std::endl;
	} else {
		/* use seek to search for end of file 
	           This method is used instead of string length because I wanted to avoid
		   reopening the file and inserting white space.
		 */
		file.seekg(0, std::ios::end);
		long len = file.tellg();
		//moved seek cursor back to beginning of file
		file.seekg(std::ios::beg);
		if (len > 0){
			//read file and store in c-string
			int i = 0; //index
			buffer = new char[len+1];
			while (file.good()){
				buffer[i] = file.get();
				if (!file.eof()){
					i++;
				}
			}
			buffer[i] = '\0';
		} else {
			std::cerr << "Empty File" << std::endl;
		}
	}
	file.close();
	return buffer;
}

void cleanUp()
{
    // Clean up, Clean up
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo_geometry);
}

//returns the time delta
float getDT()
{
    float ret;
    t2 = std::chrono::high_resolution_clock::now();
    ret = std::chrono::duration_cast< std::chrono::duration<float> >(t2-t1).count();
    t1 = std::chrono::high_resolution_clock::now();
    return ret;
}

//////////////////////////////////
// Authors: Albert Wohletz	//
//	    Collin Sorkin	//
//	    Brian Catudan	//
// Date: November 1, 2013	//
//////////////////////////////////

#include <GL/glew.h> 
#include <GL/glut.h> 
#include <GL/freeglut.h> 
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/color4.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier
#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include "mesh.h"
#include "gameManager.h"

// Global Variable summoned from the warp.
#define NUM_OBJECTS 4
int w = 640, h = 480;// Window size
double rotationRate = M_PI/2; //radians
double spinRate = 90; //degrees
double orbitRate = 5;
GLuint program;
GLuint vbo_geometry;
GLuint vbo_array[NUM_OBJECTS];
GLuint vbo_geometry2;
GLint loc_mvpmat;
GLint loc_position;
GLint loc_color;
float camx = 0;
float camy = 8;
float camz = -16;


// Defines
#define MAX_PATH_LEN 200
#define BOARD 3
#define PUCK 1
#define P1 2
#define P2 0

// GLU matrices
glm::mat4 model[NUM_OBJECTS];//obj->world each object should have its own model matrix
glm::mat4 view;//world->eye
glm::mat4 projection;//eye->clip
glm::mat4 mvp[NUM_OBJECTS];//premultiplied modelviewprojection
CMesh mesh[NUM_OBJECTS];
CGameManager gameManager;

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
void setOrthographicProjection();
void writeText(int x, int y, char* text);
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;
void printText(float, float, char* , void *, float, float, float, float);
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
	case 2: //Restart
		gameManager.p1Score = 0;
		gameManager.p2Score = 0;
		gameManager.reset();
		gameManager.isPaused = false;
		break;
	case 3: //resume
		gameManager.isPaused = false;
		break;
	case 4: // pause
		gameManager.isPaused = true;
		break;
	case 5: // Toggle AI
		gameManager.ai.isEnabled = !gameManager.ai.isEnabled; 
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
	glutCreateWindow("Extreme Air Hockey 9000");
	
	// Create Menu
	glutCreateMenu(demo_menu);
	glutAddMenuEntry("Exit", 1);
	glutAddMenuEntry("Restart", 2);
	glutAddMenuEntry("Resume", 3);
	glutAddMenuEntry("Pause", 4);
	glutAddMenuEntry("Toggle AI", 5);
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
	     
	     // Bind and draw meshes
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

	
	//clean upshader_stat
	glDisableVertexAttribArray(loc_position);
	glDisableVertexAttribArray(loc_color);

	// Write text.
	//setOrthographicProjection();

	if(gameManager.gameOver){	
			printText(-0.45f, 0.9f, "HIGH SCORE LIST", GLUT_BITMAP_HELVETICA_18, 0.0f, 1.0f, 0.0f, 0.0f);
		for(int i=0; i<10; i++){
			printText(-0.45f, 0.85f-(float)i/10, gameManager.gameOverStrings[i], GLUT_BITMAP_HELVETICA_18, 1.0f, 0.0f, 0.0f, 0.0f);
		}
	}
	else{
		char scoreStr[100];
		sprintf(scoreStr,"score: %i-%i",gameManager.p1Score,gameManager.p2Score);
		printText(-0.45f, 0.9f, scoreStr, GLUT_BITMAP_HELVETICA_18, 1.0f, 1.0f, 1.0f, 0.0f);
	}
	
	
	//swap the buffers
	glutSwapBuffers();
}

void update()
{
	if(!gameManager.isPaused && !gameManager.gameOver){
		//total time
		float dt = getDT();// if you have anything moving, use dt
		
		// Look at me bro
		view = glm::lookAt( glm::vec3(camx, camy, camz), //Eye Position
                        glm::vec3(0.0, 0.0, 0.0), //Focus point
                        glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up
        
		// Update Game/Fizzucks
		gameManager.tick(dt);
		
		// PUCK
		model[PUCK] = glm::translate(glm::mat4(1.0f), glm::vec3(gameManager.bullet.puck.x,gameManager.bullet.puck.y, gameManager.bullet.puck.z));
		
		// PADDLE1
		model[P1] = glm::translate(glm::mat4(1.0f), glm::vec3(gameManager.bullet.paddle1.x,gameManager.bullet.paddle1.y, gameManager.bullet.paddle1.z));
		
		// PADDLE2
		model[P2] = glm::translate(glm::mat4(1.0f), glm::vec3(gameManager.bullet.paddle2.x,gameManager.bullet.paddle2.y, gameManager.bullet.paddle2.z));
		
		// BOARD
		model[BOARD] = glm::translate(glm::mat4(1.0f), glm::vec3(gameManager.bullet.board.x,gameManager.bullet.board.y, gameManager.bullet.board.z));
		
		// Update the state of the scene
		glutPostRedisplay();//call the display callback
	}
}


bool initialize(char path[MAX_PATH_LEN])
{    
    // Draw objects
    mesh[BOARD].loadMesh("../assets/big.obj");
    mesh[PUCK].loadMesh("../assets/puck.obj");
    mesh[P1].loadMesh("../assets/paddle.obj");
    mesh[P2].loadMesh("../assets/paddle.obj");
    
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
    
    view = glm::lookAt( glm::vec3(camx, camy, camz), //Eye Position
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
	
}

void keyboardSpecial(int key, int x_pos, int y_pos)
{
	
    if(key == GLUT_KEY_LEFT){ 
		gameManager.bullet.paddle1.force(400,0,0);
	}
	else if(key == GLUT_KEY_RIGHT){ 
		gameManager.bullet.paddle1.force(-400.0,0,0);
	}
	else if(key == GLUT_KEY_UP){ 
		gameManager.bullet.paddle1.force(0,0,400.0);
	}
	else if(key == GLUT_KEY_DOWN){
		gameManager.bullet.paddle1.force(0,0,-400.0);
	}
}



void keyboard(unsigned char key, int x_pos, int y_pos)
{
	if(key == 27){
		exit(0);
	}
	else if(key == 'W' || key == 'w'){ // W = UP
		gameManager.bullet.paddle2.force(0,1,20.0);		
	}
	else if(key == 'a' || key == 'A'){ // A = LEFT
		gameManager.bullet.paddle2.force(20,1,0);		
	}
	else if(key == 'S' || key == 's'){ // S = DOWN
		gameManager.bullet.paddle2.force(0,1,-20.0);		
	}
	else if(key == 'D' || key == 'd'){ // D = RIGHT
		gameManager.bullet.paddle2.force(0,1,-20.0);
	}
	
	else if(key == 'I' || key == 'i')
	{
		camy += 1;
		if(camy >= 17)
		{
			camy = 17;
		}
	}
	else if(key == 'K' || key == 'k')
	{
		camy -= 1;
		if(camy <= 5)
		{
			camy = 5;
		}
	}

	else if(key == 'J' || key == 'j')
	{
		camx += 1;
		if(camx >= 9)
		{
			camx = 9;
		}

	}
	else if(key == 'L' || key == 'l')
	{
		camx -= 1;
		if(camx <= -9)
		{
			camx = -9;
		}
	}

	if(key == 'P' || key == 'p')
	{
		camz += 1;
		if(camz >= -13)
		{
			camz = -13;
		}

	}
	else if(key == 'O' || key == 'o')
	{
		camz -= 1;
		if(camz <= -19)
		{
			camz = -19;
		}
	}

	if(key == 'U' || key == 'u')
	{
		camx = 0;
		camy = 8;
		camz = -16;
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

void printText(float x, float y, char* text, void * font, 
                              float r, float g, float b, float a)
{
  // disable shaders
  glUseProgram(0);

  // set color of text
  glColor3f(r,g,b);

  // set position of text
  glRasterPos2f(x,y);
  
  // iterate through string to print text
  while (*text) 
  {
    glutBitmapCharacter(font, *text);
    text++;
  }

  // re-enable shaders
  glUseProgram(program);
}

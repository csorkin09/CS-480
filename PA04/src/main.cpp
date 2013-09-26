#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/glut.h> // doing otherwise causes compiler shouting
#include <iostream>
#include <chrono>
#include <fstream>
#include <sstream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier


//--Data types
//This object will define the attributes of a vertex(position, color, etc...)
struct Vertex
{
    GLfloat position[3];
    GLfloat color[3];
};

//--Evil Global variables
//Just for this example!
int w = 640, h = 480;// Window size
GLuint program;// The GLSL program handle
GLuint vbo_geometry;// VBO handle for our geometry

//uniform locations
GLint loc_mvpmat;// Location of the modelviewprojection matrix in the shader

//attribute locations
GLint loc_position;
GLint loc_color;

//transform matrices
glm::mat4 model;//obj->world each object should have its own model matrix
glm::mat4 moon;//moon matrix
glm::mat4 view;//world->eye
glm::mat4 projection;//eye->clip
glm::mat4 mvp;//premultiplied modelviewprojection

//--GLUT Callbacks
void render();
void update();
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);
void arrowKeys(int key, int x_pos, int y_pos);
void demo_menu(int id);
void mouse(int button, int state, int x, int y);

//--Resource management
bool initialize();
void cleanUp();

// Loader Functions
std::string shaderLoader(const std::string &fileName);
bool loadObj(char* fileName, Vertex geometry[]);
int getFileSize(char* fileName);

//--Random time things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;
int stop;
int reverse;
int flip;
char* file;
int vertexSize;


//--Main
int main(int argc, char **argv)
{

    // load file from command line
    //check to see if file was inputed 
    if(argc == 2)
    {
      file = argv[argc-1];
    }
    else
    {
     std::cout<<"You have not entered the right amount of command line arguments!"<<std::endl;
     return 0;
    }

    // Initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    // Name and create the Window
    glutCreateWindow("Matrix Example");

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
    glutSpecialFunc(arrowKeys);// Called if there is a special keyboard input


    //Menu
    glutCreateMenu(demo_menu);
    glutAddMenuEntry("Start Rotation", 1);
    glutAddMenuEntry("Stop Rotation", 2);
    glutAddMenuEntry("Quit", 3);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    glutMouseFunc(mouse);

    // Initialize all of our resources(shaders, geometry)
    bool init = initialize();
    if(init)
    {
        t1 = std::chrono::high_resolution_clock::now();
        glutMainLoop();
    }

    // Clean up after ourselves
    cleanUp();
    return 0;
}

//--Implementations
void render()
{
    //--Render the scene

    //clear the screen
    glClearColor(0.0, 0.0, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // PLANET Matrix

    //premultiply the matrix for this example
    mvp = projection * view * model;

    //enable the shader program
    glUseProgram(program);

    //upload the matrix to the shader
    glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));

    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(loc_position);
    glEnableVertexAttribArray(loc_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    //set pointers into the vbo for each of the attributes(position and color)
    glVertexAttribPointer( loc_position,//location of attribute
                           3,//number of elements
                           GL_FLOAT,//type
                           GL_FALSE,//normalized?
                           sizeof(Vertex),//stride
                           0);//offset

    glVertexAttribPointer( loc_color,
                           3,
                           GL_FLOAT,
                           GL_FALSE,
                           sizeof(Vertex),
                           (void*)offsetof(Vertex,color));

    glDrawArrays(GL_TRIANGLES, 0, vertexSize);//mode, starting index, count


    //clean up
    glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_color);
                           
    //swap the buffers
    glutSwapBuffers();
}

void update()
{
    //total time
    static float angle = 0.0;
    static float secondAngle = 0.0;

    float dt = getDT();// if you have anything moving, use dt.

    angle += dt * M_PI/2; //move through 90 degrees a second
    
    //change orbit direction
    if(flip == 1)
    {
      angle += dt * M_PI/2;
    }
    
    if(flip == 0)
    {
      angle += -2*(dt * M_PI/2);
    }

    //model = glm::translate(glm::mat4(1.0f), glm::vec3(4.0 * sin(angle), 0.0, 4.0 * cos(angle)));

    //rotate model
    if(stop == 1)
    {
      secondAngle += dt*M_PI/2;
    }

     if(reverse == 1 && stop == 1)
     {
      secondAngle += -2*(dt*M_PI/2);
     }

    //model = glm::rotate(model, 50*secondAngle, glm::vec3(0,1,0));

    // Update the state of the scene
    glutPostRedisplay();//call the display callback
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

void keyboard(unsigned char key, int x_pos, int y_pos)
{
    // Handle keyboard input
    if(key == 27)//ESC
    {
        exit(0);
    }

    //reverses rotation when 'R' or 'r' is pressed
    if(key == 114 || key == 82)//'R' or 'r'
    {
      if(reverse == 0)
      {
       reverse = 1;
      }
      else
      {
      reverse = 0;
      }
    }

    //counter-clockwise orbit when 'A' or 'a' is pressed
    if(key == 65 || key == 97)
    {
      flip = 1;
    }
    //clockwise orbit when 'D' or 'd' is pressed
    if(key == 68 || key == 100)
    {
      flip = 0;
    }

}

void arrowKeys(int key, int x_pos, int y_pos)
{

   //reverses rotation when Right Arrow Key is pressed
   if(key == GLUT_KEY_RIGHT)
   {
     reverse = 1;
   } 
   //reverses rotation when Left Arrow Key is pressed
   if(key == GLUT_KEY_LEFT)
   {
     reverse = 0;
   }

}


void mouse(int button, int state, int x, int y)
{
  //reverses rotation when the left mouse button is clicked
  if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
  {
    if(reverse == 0)
    {
     reverse = 1;
    }
    else
    {
      reverse = 0;
    }
  }
}

void demo_menu(int id)
{
  //Pull down menu options
  switch(id)
  {
    case 1:
    stop = 1; 
    break;

    case 2:
    stop = 0;
    break;

    case 3:
    exit(0);
    break;
  }

  glutPostRedisplay();

}

bool initialize()
{
    // Initialize basic geometry and shaders for this example

    //this defines a cube, this is why a model loader is nice
    //you can also do this with a draw elements and indices, try to get that working
    char *fileName = file;
    int size = getFileSize(fileName);

    Vertex geometry[size]; 

    if(loadObj(fileName, geometry) == false)
    {
	std::cout<<"Cannot load OBJ!"<<std::endl;
	return false;
    }


    // Create a Vertex Buffer object to store this vertex info on the GPU
    glGenBuffers(1, &vbo_geometry);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    glBufferData(GL_ARRAY_BUFFER, vertexSize*sizeof(Vertex), geometry, GL_STATIC_DRAW);

    //--Geometry done

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    //Shader Sources
    // Put these into files and write a loader in the future
    // Note the added uniform!

    std::string vs_string = shaderLoader("../bin/vs.txt");
    std::string fs_string = shaderLoader("../bin/fs.txt");

    const char* vs = vs_string.c_str();
    const char* fs = fs_string.c_str();

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

    // Now the Fragment shader
    glShaderSource(fragment_shader, 1, &fs, NULL);
    glCompileShader(fragment_shader);
    //check the compile status
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] FAILED TO COMPILE FRAGMENT SHADER!" << std::endl;
        return false;
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

// Loader Functions
std::string shaderLoader(const std::string &fileName)
{
  std::ifstream inf;

  inf.open(fileName.c_str(), std::ifstream::in);

  std::stringstream buffer;

  buffer << inf.rdbuf();

  inf.close();

  return buffer.str();

}
////////////////////////////////*****NOTE******///////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
// My object loader was made with the help                                      //
// of the tutorial on this website:                                             // 
//                                                                              //
// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/ //
//////////////////////////////////////////////////////////////////////////////////


//loads the obj from the file
bool loadObj(char *fileName, Vertex geometry[])
{
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec3> my_temp;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;
    glm::vec3 tempVec3;
    int flag1, flag2, flag3 = 0;

    

    FILE* file = fopen(fileName, "r");

    if(file == NULL)
    {
      std::cout<<"Cannot open file!"<<std::endl;
      return false;
    }

    while(1)
    {
        char lineHeader[128];

        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);

        if(res == EOF)
        {
            break; // EOF = End Of File. Quit the loop.
	}

        //parse lineHeader

        //parse all the "v"
        if(strcmp(lineHeader, "v") == 0)
        {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
            flag1 = 1;
	}
        //parse all the "vt"
	else if(strcmp(lineHeader, "vt") == 0 )
	{
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y);
            temp_uvs.push_back(uv);
            flag2 = 1;
	}
        //parse all the "vn"
	else if(strcmp(lineHeader, "vn" ) == 0)
	{
            glm::vec3 normal;
    	    fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
    	    temp_normals.push_back(normal);
            flag3 = 1; 
	}
        //parse all the "f"
	else if(strcmp(lineHeader, "f") == 0)
	{
    	    std::string vertex1, vertex2, vertex3;
    	    unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];


            if(flag1 == 1 && flag3 == 1)
            {
              fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
    	      
    	        vertexIndices.push_back(vertexIndex[0]);
    	  	vertexIndices.push_back(vertexIndex[1]);
    		vertexIndices.push_back(vertexIndex[2]);
    		normalIndices.push_back(normalIndex[0]);
    		normalIndices.push_back(normalIndex[1]);
    		normalIndices.push_back(normalIndex[2]);
            }

           else if(flag1 == 1 && flag2 == 1 && flag3 == 1)
            {
              fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
    	      
    	        vertexIndices.push_back(vertexIndex[0]);
    	  	vertexIndices.push_back(vertexIndex[1]);
    		vertexIndices.push_back(vertexIndex[2]);
                uvIndices.push_back(uvIndex[0]);
    	  	uvIndices.push_back(uvIndex[1]);
    		uvIndices.push_back(uvIndex[2]);
    		normalIndices.push_back(normalIndex[0]);
    		normalIndices.push_back(normalIndex[1]);
    		normalIndices.push_back(normalIndex[2]);
            }

            else if(flag1 == 1 && flag2 == 1)
            {
              fscanf(file, "%d/%d %d/%d %d/%d\n", &vertexIndex[0], &uvIndex[0], &vertexIndex[1], &uvIndex[1], &vertexIndex[2], &uvIndex[2]);
    	      
    	        vertexIndices.push_back(vertexIndex[0]);
    	  	vertexIndices.push_back(vertexIndex[1]);
    		vertexIndices.push_back(vertexIndex[2]);
                uvIndices.push_back(uvIndex[0]);
    	  	uvIndices.push_back(uvIndex[1]);
    		uvIndices.push_back(uvIndex[2]);
            }

            else if(flag1 == 1)
            {
              fscanf(file, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
    	      
    	        vertexIndices.push_back(vertexIndex[0]);
    	  	vertexIndices.push_back(vertexIndex[1]);
    		vertexIndices.push_back(vertexIndex[2]);
            }

	}
    }
    
    Vertex tempGeo[vertexIndices.size()];

    for(unsigned int i=0; i<vertexIndices.size(); i++)
    {
	temp_vertices.begin();
	tempVec3 = temp_vertices.front();
	
        tempGeo[i].position[0] = tempVec3.x;
        tempGeo[i].position[1] = tempVec3.y;
        tempGeo[i].position[2] = tempVec3.z;

	temp_vertices.erase(temp_vertices.begin());	
    }

    int flag = 0;
    int index;

    for(unsigned int i=0; i<vertexIndices.size(); i++)
    {
	index = vertexIndices[i];

        geometry[i].position[0] = tempGeo[index-1].position[0];
        geometry[i].position[1] = tempGeo[index-1].position[1];
        geometry[i].position[2] = tempGeo[index-1].position[2];

        if(flag == 0)
        {
        geometry[i].color[0] = 0.0;
        geometry[i].color[1] = 0.0;
        geometry[i].color[2] = 1.0;
        flag = 1;
        }

        else if(flag == 1)
        {
        geometry[i].color[0] = 0.0;
        geometry[i].color[1] = 1.0;
        geometry[i].color[2] = 0.0;
        flag = 2;
        }

        else if(flag == 2)
        {
        geometry[i].color[0] = 1.0;
        geometry[i].color[1] = 0.0;
        geometry[i].color[2] = 0.0;
        flag = 3;
        }

        else if(flag == 3)
        {
        geometry[i].color[0] = 1.0;
        geometry[i].color[1] = 0.0;
        geometry[i].color[2] = 1.0;
        flag = 4;
        }

        else if(flag == 4)
        {
        geometry[i].color[0] = 1.0;
        geometry[i].color[1] = 1.0;
        geometry[i].color[2] = 0.0;
        flag = 5;
        }

        else if(flag == 5)
        {
        geometry[i].color[0] = 0.0;
        geometry[i].color[1] = 1.0;
        geometry[i].color[2] = 1.0;
        flag = 0;
        }
    
    }


    return true;
}

//gets the size of the file
int getFileSize(char* fileName)
{
   std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
   std::vector<glm::vec3> temp_vertices;
   std::vector<glm::vec2> temp_uvs;
   std::vector<glm::vec3> temp_normals;
   int flag1, flag2, flag3 = 0;
   

   FILE* file = fopen(fileName, "r");

   if(file == NULL)
   {
     printf("Cannot open file!\n");
     return false;
   }

   while(1)
   {
     char lineHeader[128];
     //read the first word of the line in the file
     int res = fscanf(file, "%s", lineHeader);
     if(res == EOF)
     {
       break;
     }

        if(strcmp(lineHeader, "v") == 0)
        {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
            flag1 = 1;
	}
        //parse all the "vt"
	else if(strcmp(lineHeader, "vt") == 0 )
	{
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y);
            temp_uvs.push_back(uv);
            flag2 = 1;
	}
        //parse all the "vn"
	else if(strcmp(lineHeader, "vn" ) == 0)
	{
            glm::vec3 normal;
    	    fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
    	    temp_normals.push_back(normal);
            flag3 = 1; 
	}

     //line parsing
     if(strcmp(lineHeader, "f") == 0)
	{
    	    std::string vertex1, vertex2, vertex3;
    	    unsigned int vertexIndex[3], uvIndex[3], normalIndex[3]; 

    	    if(flag1 == 1 && flag3 == 1)
            {
              fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
    	      
    	        vertexIndices.push_back(vertexIndex[0]);
    	  	vertexIndices.push_back(vertexIndex[1]);
    		vertexIndices.push_back(vertexIndex[2]);
    		normalIndices.push_back(normalIndex[0]);
    		normalIndices.push_back(normalIndex[1]);
    		normalIndices.push_back(normalIndex[2]);
            }

           else if(flag1 == 1 && flag2 == 1 && flag3 == 1)
            {
              fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
    	      
    	        vertexIndices.push_back(vertexIndex[0]);
    	  	vertexIndices.push_back(vertexIndex[1]);
    		vertexIndices.push_back(vertexIndex[2]);
                uvIndices.push_back(uvIndex[0]);
    	  	uvIndices.push_back(uvIndex[1]);
    		uvIndices.push_back(uvIndex[2]);
    		normalIndices.push_back(normalIndex[0]);
    		normalIndices.push_back(normalIndex[1]);
    		normalIndices.push_back(normalIndex[2]);
            }

            else if(flag1 == 1 && flag2 == 1)
            {
              fscanf(file, "%d/%d %d/%d %d/%d\n", &vertexIndex[0], &uvIndex[0], &vertexIndex[1], &uvIndex[1], &vertexIndex[2], &uvIndex[2]);
    	      
    	        vertexIndices.push_back(vertexIndex[0]);
    	  	vertexIndices.push_back(vertexIndex[1]);
    		vertexIndices.push_back(vertexIndex[2]);
                uvIndices.push_back(uvIndex[0]);
    	  	uvIndices.push_back(uvIndex[1]);
    		uvIndices.push_back(uvIndex[2]);
            }

            else if(flag1 == 1)
            {
              fscanf(file, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
    	      
    	        vertexIndices.push_back(vertexIndex[0]);
    	  	vertexIndices.push_back(vertexIndex[1]);
    		vertexIndices.push_back(vertexIndex[2]);
                uvIndices.push_back(uvIndex[0]);
    	  	uvIndices.push_back(uvIndex[1]);
    		uvIndices.push_back(uvIndex[2]);
            }
	}
    }

    vertexSize = vertexIndices.size();
    return vertexSize;
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

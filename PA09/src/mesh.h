#ifndef _MESH_H_
#define _MESH_H_

#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/glut.h> // doing otherwise causes compiler shouting
#include <GL/freeglut.h> // doing otherwise causes compiler shouting
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/color4.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier
#include "texture.h"

using namespace std;
struct Vertex
{
    GLfloat position[3];
    GLfloat color[2];
};

class CMesh{
	public:
		Vertex* geometry;
		std::vector<int> indecies;
		int numVertex;

		CMesh();
		~CMesh();
		bool loadMesh(const std::string& path);
		void render();
		std::vector<Texture*> m_Textures;
		unsigned int numOfVert;
		std::vector<unsigned int> materialIndex;

	//private:
		bool initFromScene(const aiScene* pScene, const std::string& path);
		int initMesh(unsigned int Index, const aiMesh* paiMesh);
		bool initMaterials(const aiScene* pScene, const std::string& path);
		void clear();
};

#endif
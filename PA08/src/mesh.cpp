#include "mesh.h"
CMesh::CMesh(){
}
CMesh::~CMesh(){
}

bool CMesh::loadMesh(const std::string& path){
	clear();

	Assimp::Importer importer;
	const aiScene* ais = importer.ReadFile(path, aiProcess_Triangulate); // returns an aiSceneClass

	if(ais == NULL){
		std::cout << importer.GetErrorString();
		return false;
	}
	else{
		initFromScene(ais, path);
		return true;
	}
}

bool CMesh::initFromScene(const aiScene* pScene, const std::string& path){
	numVertex = 0;
	int currentVertex=0;

	for(unsigned int i=0; i< pScene->mNumMeshes;i++){
		numVertex += pScene->mMeshes[i]->mNumFaces*3;
	}
	geometry = new Vertex[numVertex]; 
    m_Textures.resize(pScene->mNumMaterials);

	indecies.push_back(0);
    for (unsigned int i = 0; i < pScene->mNumMeshes ; i++) {
	const aiMesh* paiMesh = pScene->mMeshes[i];        
	currentVertex += initMesh(currentVertex, paiMesh);
	indecies.push_back(currentVertex);
    }
		
    return initMaterials(pScene, path);
}
int CMesh::initMesh(unsigned int index, const aiMesh* paiMesh){\
	//Loads the triangluated faces
	
	materialIndex.push_back(paiMesh->mMaterialIndex);
	std::vector< unsigned int> Indices;
	for (unsigned int i = 0 ; i < paiMesh->mNumFaces ; i++) {
		const aiFace& Face = paiMesh->mFaces[i];
		Indices.push_back(Face.mIndices[0]);
		Indices.push_back(Face.mIndices[1]);
		Indices.push_back(Face.mIndices[2]);
	}
	numOfVert = Indices.size();
	
	for(unsigned int i=0; i< numOfVert; i++){
		const aiVector3D* vertex = &(paiMesh->mVertices[Indices[i]]);
		geometry[i+index].position[0] = vertex->x;
		geometry[i+index].position[1] = vertex->y;
		geometry[i+index].position[2] = vertex->z;	

		if(paiMesh->HasTextureCoords(0)){
			geometry[i+index].color[0] = paiMesh->mTextureCoords[0][Indices[i]].x;
			geometry[i+index].color[1] = 1-paiMesh->mTextureCoords[0][Indices[i]].y;
		}
		else{
			geometry[i+index].color[0] = .5;
			geometry[i+index].color[1] = .5;
		}
	}
	return numOfVert;
}
bool CMesh::initMaterials(const aiScene* pScene, const std::string& fileName){
 // Extract the directory part from the file name
    std::string::size_type SlashIndex = fileName.find_last_of("/");
    std::string dir;
	bool ret;

    if (SlashIndex == std::string::npos) {
        dir = ".";
    }
    else if (SlashIndex == 0) {
        dir = "/";
    }
    else {
        dir = fileName.substr(0, SlashIndex);
    }


	for(unsigned int i = 0; i< pScene->mNumMaterials; i++){
		const aiMaterial* pMaterial = pScene->mMaterials[i];
		aiString path;
	        m_Textures[i] = NULL;
        	if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
	            aiString Path;

            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                std::string FullPath = dir + "/" + Path.data;
                m_Textures[i] = new Texture(GL_TEXTURE_2D, FullPath.c_str());

                if (!m_Textures[i]->Load()) {
                    printf("Error loading texture '%s'\n", FullPath.c_str());
                    delete m_Textures[i];
                    m_Textures[i] = NULL;
                    ret = false;
                }
            }
        } 
		if (!m_Textures[i]) {
		#ifdef _WIN32
			m_Textures[i] = new Texture(GL_TEXTURE_2D, "C:/Users/Blindo/git/480-PA1/Assignment06/white.png");
		#else
			m_Textures[i] = new Texture(GL_TEXTURE_2D, "../assets/white.png");	
		#endif
          ret = m_Textures[i]->Load();
       }
		//
		//aiColor3D color (0.f,0.f,0.f);
		//paiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
	}  	

    return ret;
}
void CMesh::clear(){

}

void CMesh::render(){
	


}

#PA07: Assimp Model Loader w/Textures

#Team
- Brian Catudan
- Collin Sorkin(submitted under Collin's account)

 
***Required Features***
- Loads models with texture images.
- Takes two command line arguments one for the object file and one for the texture file.
***********************

***Other Features***
- Uses Assimp to load models.
- Uses devIL to handle textures.  
********************

***Notes***
- Uses Assimp 3.0.
- Uses devIL 1.7
- obj file has to have vt coordinates(texture coordinates)
- texture images need to be in bin folder
***********

***Bugs***
- Running this program without a command argument for the object file will seg fault program. Not really a bug but the program is not robust.
**********

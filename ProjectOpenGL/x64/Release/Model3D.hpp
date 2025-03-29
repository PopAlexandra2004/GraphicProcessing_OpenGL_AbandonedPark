#ifndef Model3D_hpp
#define Model3D_hpp

#include "Mesh.hpp"
#include "tiny_obj_loader.h"
#include "stb_image.h"
#include <iostream>
#include <string>
#include <vector>

namespace gps {

    class Model3D {

    public:
        ~Model3D();

        // Load a model from an OBJ file
        void LoadModel(std::string fileName);
        void LoadModel(std::string fileName, std::string basePath);

        // Render the loaded model
        void Draw(gps::Shader shaderProgram);

    private:
        std::vector<gps::Mesh> meshes;         // Group of mesh objects
        std::vector<gps::Texture> loadedTextures; // Store loaded textures to avoid duplicates

        // Parses an .obj file and fills in the model data structure
        void ReadOBJ(std::string fileName, std::string basePath);

        // Retrieves a texture by path and type (ambient, diffuse, specular)
        gps::Texture LoadTexture(std::string path, std::string type);

        // Reads pixel data and loads a texture into OpenGL
        GLuint ReadTextureFromFile(const char* file_name);
    };

}

#endif /* Model3D_hpp */

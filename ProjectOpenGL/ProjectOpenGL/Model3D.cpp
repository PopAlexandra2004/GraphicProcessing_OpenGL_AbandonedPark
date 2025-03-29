#include "Model3D.hpp"

namespace gps {

    void Model3D::LoadModel(std::string fileName) {
        std::string basePath = fileName.substr(0, fileName.find_last_of('/')) + "/";
        ReadOBJ(fileName, basePath);
    }

    void Model3D::LoadModel(std::string fileName, std::string basePath) {
        ReadOBJ(fileName, basePath);
    }

    void Model3D::Draw(gps::Shader shaderProgram) {
        for (size_t i = 0; i < meshes.size(); i++) {
            meshes[i].Draw(shaderProgram);
        }
    }

    void Model3D::ReadOBJ(std::string fileName, std::string basePath) {
        std::cout << "[DEBUG] Loading Model: " << fileName << std::endl;
        std::cout << "Loading Model: " << fileName << std::endl;

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string err;

        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, fileName.c_str(), basePath.c_str(), true);
        if (!err.empty()) {
            std::cerr << "TinyOBJ Error: " << err << std::endl;
        }
        if (!ret) {
            std::cerr << "Failed to load OBJ file: " << fileName << std::endl;
            return;
        }
        std::cout << "[DEBUG] Model Loaded Successfully!" << std::endl;
        std::cout << "[DEBUG] Vertex Count: " << attrib.vertices.size() / 3 << std::endl;
        std::cout << "[DEBUG] Face Count: " << shapes.size() << std::endl;
        std::cout << "[DEBUG] Material Count: " << materials.size() << std::endl;
        std::cout << "Shapes Count: " << shapes.size() << ", Materials Count: " << materials.size() << std::endl;

        for (size_t s = 0; s < shapes.size(); s++) {
            std::vector<gps::Vertex> vertices;
            std::vector<GLuint> indices;
            std::vector<gps::Texture> textures;

            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                int fv = shapes[s].mesh.num_face_vertices[f];

                for (size_t v = 0; v < fv; v++) {
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                    glm::vec3 vertexPosition = {
                        attrib.vertices[3 * idx.vertex_index + 0],
                        attrib.vertices[3 * idx.vertex_index + 1],
                        attrib.vertices[3 * idx.vertex_index + 2]
                    };

                    glm::vec3 vertexNormal = { 0.0f, 0.0f, 0.0f };
                    if (idx.normal_index >= 0) {
                        vertexNormal = {
                            attrib.normals[3 * idx.normal_index + 0],
                            attrib.normals[3 * idx.normal_index + 1],
                            attrib.normals[3 * idx.normal_index + 2]
                        };
                    }

                    glm::vec2 vertexTexCoords = { 0.0f, 0.0f };
                    if (idx.texcoord_index >= 0) {
                        vertexTexCoords = {
                            attrib.texcoords[2 * idx.texcoord_index + 0],
                            attrib.texcoords[2 * idx.texcoord_index + 1]
                        };
                    }

                    gps::Vertex currentVertex;
                    currentVertex.Position = vertexPosition;
                    currentVertex.Normal = vertexNormal;
                    currentVertex.TexCoords = vertexTexCoords;

                    vertices.push_back(currentVertex);
                    indices.push_back(static_cast<GLuint>(index_offset + v));
                }
                index_offset += fv;
            }

            if (!materials.empty() && !shapes[s].mesh.material_ids.empty()) {
                int materialId = shapes[s].mesh.material_ids[0];

                if (materialId >= 0) {
                    gps::Material currentMaterial;
                    currentMaterial.ambient = {
                        materials[materialId].ambient[0],
                        materials[materialId].ambient[1],
                        materials[materialId].ambient[2]
                    };
                    currentMaterial.diffuse = {
                        materials[materialId].diffuse[0],
                        materials[materialId].diffuse[1],
                        materials[materialId].diffuse[2]
                    };
                    currentMaterial.specular = {
                        materials[materialId].specular[0],
                        materials[materialId].specular[1],
                        materials[materialId].specular[2]
                    };

                    std::string texturePaths[] = {
                        materials[materialId].ambient_texname,
                        materials[materialId].diffuse_texname,
                        materials[materialId].specular_texname
                    };
                    std::string textureTypes[] = { "ambientTexture", "diffuseTexture", "specularTexture" };

                    for (int i = 0; i < 3; i++) {
                        if (!texturePaths[i].empty()) {
                            gps::Texture currentTexture = LoadTexture(basePath + texturePaths[i], textureTypes[i]);
                            textures.push_back(currentTexture);
                        }
                    }
                }
            }

            meshes.push_back(gps::Mesh(vertices, indices, textures));
        }
    }

    gps::Texture Model3D::LoadTexture(std::string path, std::string type) {
        for (const auto& loadedTexture : loadedTextures) {
            if (loadedTexture.path == path) {
                return loadedTexture;
            }
        }

        gps::Texture currentTexture;
        currentTexture.id = ReadTextureFromFile(path.c_str());
        currentTexture.type = type;
        currentTexture.path = path;

        loadedTextures.push_back(currentTexture);
        return currentTexture;
    }

    GLuint Model3D::ReadTextureFromFile(const char* file_name) {
        int width, height, nrChannels;
        unsigned char* data = stbi_load(file_name, &width, &height, &nrChannels, 4);
        if (!data) {
            std::cerr << "ERROR: Failed to load texture: " << file_name << std::endl;
            return 0;
        }

        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(data);
        return textureID;
    }

    Model3D::~Model3D() {
        for (auto& texture : loadedTextures) {
            glDeleteTextures(1, &texture.id);
        }

        for (auto& mesh : meshes) {
            GLuint VBO = mesh.getBuffers().VBO;
            GLuint EBO = mesh.getBuffers().EBO;
            GLuint VAO = mesh.getBuffers().VAO;
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &EBO);
            glDeleteVertexArrays(1, &VAO);
        }
    }

}

//
// Shader.cpp
//

#include "Shader.hpp"
#include <iostream>
#include <algorithm> // For std::replace
#include <sys/stat.h> // For file existence check

namespace gps {

    // Helper function to check if a file exists
    bool fileExists(const std::string& filename) {
        struct stat buffer;
        return (stat(filename.c_str(), &buffer) == 0);
    }

    // Function to read shader file
    std::string Shader::readShaderFile(std::string fileName) {
        std::replace(fileName.begin(), fileName.end(), '\\', '/'); // Normalize path slashes

        char fullPath[512];
        _fullpath(fullPath, fileName.c_str(), 512); // Get absolute path
        std::cerr << "[DEBUG] Trying to load: " << fullPath << std::endl;

        std::ifstream shaderFile(fullPath);
        if (!shaderFile) {
            std::cerr << "[ERROR] Shader file still not found at: " << fullPath << std::endl;
            return "";
        }

        std::stringstream shaderStringStream;
        shaderStringStream << shaderFile.rdbuf();
        return shaderStringStream.str();
    }



    // Function to log shader compilation errors
    void Shader::shaderCompileLog(GLuint shaderId) {
        GLint success;
        GLchar infoLog[512];

        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shaderId, 512, NULL, infoLog);
            std::cerr << "[ERROR] Shader compilation failed:\n" << infoLog << std::endl;
        }
    }

    // Function to log shader linking errors
    void Shader::shaderLinkLog(GLuint shaderProgramId) {
        GLint success;
        GLchar infoLog[512];

        glGetProgramiv(shaderProgramId, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgramId, 512, NULL, infoLog);
            std::cerr << "[ERROR] Shader linking failed:\n" << infoLog << std::endl;
        }
    }

    // Function to load and compile shaders
    void Shader::loadShader(std::string vertexShaderFileName, std::string fragmentShaderFileName) {
        // Read vertex shader
        std::string v = readShaderFile(vertexShaderFileName);
        if (v.empty()) return;

        const GLchar* vertexShaderString = v.c_str();
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderString, NULL);
        glCompileShader(vertexShader);
        shaderCompileLog(vertexShader);

        GLint vertexSuccess;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexSuccess);
        if (!vertexSuccess) {
            std::cerr << "[ERROR] Vertex shader compilation failed. Skipping linking." << std::endl;
            glDeleteShader(vertexShader);
            return;
        }

        // Read fragment shader
        std::string f = readShaderFile(fragmentShaderFileName);
        if (f.empty()) {
            glDeleteShader(vertexShader);
            return;
        }

        const GLchar* fragmentShaderString = f.c_str();
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderString, NULL);
        glCompileShader(fragmentShader);
        shaderCompileLog(fragmentShader);

        GLint fragmentSuccess;
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragmentSuccess);
        if (!fragmentSuccess) {
            std::cerr << "[ERROR] Fragment shader compilation failed. Skipping linking." << std::endl;
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return;
        }

        // Link shaders
        this->shaderProgram = glCreateProgram();
        glAttachShader(this->shaderProgram, vertexShader);
        glAttachShader(this->shaderProgram, fragmentShader);
        glLinkProgram(this->shaderProgram);
        shaderLinkLog(this->shaderProgram);

        // Clean up shaders
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    // Function to use the shader program
    void Shader::useShaderProgram() {
        glUseProgram(this->shaderProgram);
    }

}

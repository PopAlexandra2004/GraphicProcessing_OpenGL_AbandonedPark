#ifndef RAIN_HPP
#define RAIN_HPP

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Shader.hpp"

// Structure to hold properties of a single raindrop
struct RainDrop {
    glm::vec3 position;    // Position of the raindrop
    glm::vec3 velocity;    // Falling speed & direction
    float spawnDelay;      // Delay before raindrop starts falling
    bool isFalling;        // Flag to check if raindrop is active
};

// Class to manage the rain effect
class RainSystem {
public:
    // Constructor
    RainSystem(int maxDropsCount, gps::Shader rainShader);

    // Initializes the rain system
    void init();

    // Updates raindrop positions over time
    void update(float deltaTime);

    // Uploads raindrop data to the GPU
    void uploadToGPU();

    // Renders the rain effect
    void draw(const glm::mat4& projection, const glm::mat4& view);

    // Cleans up GPU resources
    void destroy();

private:
    // Generates a random position above the scene to spawn raindrops
    glm::vec3 randomSpawnAbove();

    gps::Shader rainShader;  // Shader for rendering rain

    std::vector<RainDrop> drops;  // List of raindrops

    GLuint VAO = 0;  // Vertex Array Object
    GLuint VBO = 0;  // Vertex Buffer Object
    int maxDrops;    // Maximum number of raindrops in the scene
    bool isInitialized;  // Flag to track if the system is initialized
};

#endif // RAIN_HPP


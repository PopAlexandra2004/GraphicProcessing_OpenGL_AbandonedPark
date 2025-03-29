#include "Rain.hpp"
#include <cstdlib>  // For rand()
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

// Constructor: Initializes the rain system
RainSystem::RainSystem(int maxDropsCount, gps::Shader rainShader)
    : maxDrops(maxDropsCount), rainShader(rainShader), isInitialized(false)
{
    drops.resize(maxDrops);
}

// Initializes the rain system
void RainSystem::init() {
    // Generate raindrops with random positions and speeds
    for (int i = 0; i < maxDrops; i++) {
        drops[i].position = randomSpawnAbove();

        float fallSpeed = 100.0f + static_cast<float>(rand() % 100);  // Random fall speed
        drops[i].velocity = glm::vec3(0.0f, -fallSpeed, 0.0f);

        drops[i].spawnDelay = static_cast<float>(rand() % 3000) / 1000.0f;
        drops[i].isFalling = false;
    }

    // Generate OpenGL buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER,
        2 * maxDrops * sizeof(glm::vec3),
        nullptr,
        GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    isInitialized = true;  // Mark as initialized
}

// Updates raindrop positions
void RainSystem::update(float deltaTime) {
    if (!isInitialized) return;

    for (int i = 0; i < maxDrops; i++) {
        auto& d = drops[i];

        if (!d.isFalling) {
            d.spawnDelay -= deltaTime;
            if (d.spawnDelay <= 0.0f) {
                d.isFalling = true;
            }
        }

        if (d.isFalling) {
            d.position += d.velocity * deltaTime;
            if (d.position.y < -10.0f) {  // Reset when below a threshold
                d.position = randomSpawnAbove();

                float fallSpeed = 100.0f + static_cast<float>(rand() % 100);
                d.velocity = glm::vec3(0.0f, -fallSpeed, 0.0f);

                d.spawnDelay = static_cast<float>(rand() % 3000) / 1000.0f;
                d.isFalling = false;
            }
        }
    }
}

// Uploads updated raindrop positions to the GPU
void RainSystem::uploadToGPU() {
    if (!isInitialized) return;

    std::vector<glm::vec3> linePoints(2 * maxDrops);

    for (int i = 0; i < maxDrops; i++) {
        glm::vec3 head = drops[i].position;
        glm::vec3 tail = head + glm::vec3(0.0f, -2.0f, 0.0f);  // Length of raindrop

        linePoints[2 * i] = head;
        linePoints[2 * i + 1] = tail;
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferSubData(GL_ARRAY_BUFFER,
        0,
        linePoints.size() * sizeof(glm::vec3),
        linePoints.data());

    glBindVertexArray(0);
}

// Draws the rain effect using shaders
void RainSystem::draw(const glm::mat4& projection, const glm::mat4& view) {
    if (!isInitialized) return;

    rainShader.useShaderProgram();

    GLint rainColorLoc = glGetUniformLocation(rainShader.shaderProgram, "rainColor");
    glUniform3f(rainColorLoc, 0.5f, 0.5f, 1.0f);  // Blue color for raindrops

    GLint projLoc = glGetUniformLocation(rainShader.shaderProgram, "projection");
    GLint viewLoc = glGetUniformLocation(rainShader.shaderProgram, "view");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, 2 * maxDrops);  // Render as lines
    glBindVertexArray(0);
}

// Cleans up GPU resources
void RainSystem::destroy() {
    if (!isInitialized) return;

    if (VAO) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (VBO) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }

    drops.clear();
    isInitialized = false;  // Mark as uninitialized
}

// Generates a random spawn position above the scene
glm::vec3 RainSystem::randomSpawnAbove() {
    // Generate a random position above the scene
    float x = static_cast<float>(rand() % 200 - 100); // Adjust range as needed
    float y = static_cast<float>(rand() % 50 + 50);   // Adjust height as needed
    float z = static_cast<float>(rand() % 200 - 100); // Adjust range as needed
    return glm::vec3(x, y, z);
}

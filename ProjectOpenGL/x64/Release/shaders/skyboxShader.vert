#version 410 core

layout (location = 0) in vec3 vertexPosition;
out vec3 textureCoordinates;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    // Remove translation from view matrix (keep rotation only)
    mat4 skyboxView = mat4(mat3(view));

    // Apply projection & view (without translation)
    vec4 tempPos = projection * skyboxView * vec4(vertexPosition, 1.0);

    // Ensure proper depth
    gl_Position = tempPos.xyww;

    // Pass position as texture coordinates for sampling
    textureCoordinates = vertexPosition;
}

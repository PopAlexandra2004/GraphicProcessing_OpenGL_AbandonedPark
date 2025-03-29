#version 410 core

in vec3 textureCoordinates;
out vec4 color;

uniform samplerCube skybox;

void main()
{
    vec3 correctedCoords = textureCoordinates;

    // Rotate UP face by 180° (flip X and Z)
    if (abs(textureCoordinates.y) > 0.99) { 
        correctedCoords.x = -correctedCoords.x;
        correctedCoords.z = -correctedCoords.z;
    }

    color = texture(skybox, correctedCoords);
}

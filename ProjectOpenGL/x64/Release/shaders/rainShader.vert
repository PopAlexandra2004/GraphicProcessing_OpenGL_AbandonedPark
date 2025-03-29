#version 410 core

layout(location = 0) in vec3 vertexPosition; // Position of each raindrop

uniform mat4 projection;
uniform mat4 view;
uniform float time;  // Time variable for animation

out vec3 fragPosition; // Pass position to fragment shader

void main() {
    // Simulate rain falling using time
    vec3 animatedPosition = vertexPosition;
    animatedPosition.y -= mod(time * 50.0, 200.0); // Make rain reset after falling 200 units

    // Transform the raindrop's position
    gl_Position = projection * view * vec4(animatedPosition, 1.0);

    // Pass data to fragment shader
    fragPosition = animatedPosition;
}

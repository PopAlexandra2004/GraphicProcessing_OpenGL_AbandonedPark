#version 410 core

in vec3 fragPosition;  // Received from vertex shader
out vec4 fragColor;

uniform vec3 rainColor;
uniform float time;

void main() {
    // Adjust transparency based on height for a fading effect
    float fadeFactor = smoothstep(0.0, 1.0, fragPosition.y / 100.0);
    
    // Apply slight pulsation based on time (optional)
    float alpha = 0.5 + 0.3 * sin(time * 2.0);

    // Final color output with fading effect
    fragColor = vec4(rainColor, alpha * fadeFactor);
}

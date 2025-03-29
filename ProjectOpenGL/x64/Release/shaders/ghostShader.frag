#version 410 core
in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform sampler2D texture_diffuse1;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform bool useTransparency;

void main()
{
    vec4 texColor = texture(texture_diffuse1, TexCoords);

    // If transparency is enabled, discard fully transparent pixels
    if (useTransparency && texColor.a < 0.1)
        discard;

    // Simple lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.1);

    vec3 diffuse = diff * lightColor;
    vec3 result = diffuse * texColor.rgb;

    FragColor = vec4(result, texColor.a); // Preserve texture alpha
}

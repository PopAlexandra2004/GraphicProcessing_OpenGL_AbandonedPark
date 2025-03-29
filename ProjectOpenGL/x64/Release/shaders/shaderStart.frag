#version 410 core 

// =============================
//      Inputs from Vertex Shader
// =============================
in vec3 fPosition;           
in vec3 fNormal;             
in vec2 fTexCoords;          
in vec4 fPosEye;             
in vec4 fragPosLightSpace;    

out vec4 fColor;

// =============================
//      Uniforms
// =============================

// Directional Light (Sun)
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform float lightBrightness;
uniform int enableDirectionalLight;  // ✅ Added

// Point Lights
#define MAX_POINT_LIGHTS 4  
uniform int numPointLights;
uniform vec3 pointLightPositions[MAX_POINT_LIGHTS];
uniform vec3 pointLightColor;
uniform float pointLightAmbient;
uniform float pointLightDiffuse;
uniform float pointLightSpecular;
uniform float constantAtt;
uniform float linearAtt;
uniform float quadraticAtt;
uniform int enablePointLight;  // ✅ Added

// Textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

// Shadow Mapping
uniform sampler2D shadowMap;
const float shadowBias = 0.002;  
const float shadowIntensity = 0.5;

// Fog
uniform int enableFog;      
uniform float fogDensity;    
uniform vec4 fogColor;      

// =============================
//      Functions
// =============================

// Compute Shadow Factor
float computeShadow() {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0) return 0.0;  // Outside shadow map range

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    return (currentDepth - shadowBias > closestDepth) ? shadowIntensity : 0.0;
}

// Compute Fog Factor
float computeFogFactor() {
    float distance = length(fPosEye.xyz);
    float fogFactor = exp(-distance * fogDensity * fogDensity);  
    return clamp(fogFactor, 0.0, 1.0);
}

// Compute Directional Light (Sun)
vec3 CalcDirectionalLight(vec3 normalEye, vec3 viewDirEye) {
    float ambientStrength = 0.05;  
    vec3 ambient = ambientStrength * lightColor;

    float diff = max(dot(normalEye, normalize(lightDir)), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 reflection = reflect(-normalize(lightDir), normalEye);
    float specCoeff = pow(max(dot(viewDirEye, reflection), 0.0), 32.0);
    vec3 specular = specCoeff * lightColor;

    return (ambient + diffuse + specular) * lightBrightness;
}

// Compute Point Light Contribution
vec3 CalcPointLight(vec3 normalEye, vec3 fragPosEye, vec3 viewDirEye, vec3 lightPosEye) {
    vec3 lightVec = lightPosEye - fragPosEye;
    float distance = max(length(lightVec), 0.01);
    vec3 L = normalize(lightVec);

    vec3 ambient = pointLightAmbient * pointLightColor;
    float diff = max(dot(normalEye, L), 0.0);
    vec3 diffuse = pointLightDiffuse * diff * pointLightColor;

    vec3 reflection = reflect(-L, normalEye);
    float specCoeff = pow(max(dot(viewDirEye, reflection), 0.0), 32.0);
    vec3 specular = pointLightSpecular * specCoeff * pointLightColor;

    float attenuation = 1.0 / (constantAtt + linearAtt * distance + quadraticAtt * distance * distance + 0.01);

    return (ambient + diffuse + specular) * attenuation;
}

// =============================
//      Main Shader Execution
// =============================
void main() {
    vec3 normalEye = normalize(fNormal);
    vec3 viewDirEye = normalize(-fPosEye.xyz);

    // Get texture color (fallback if missing)
    vec3 baseColor = texture(diffuseTexture, fTexCoords).rgb;
    if (baseColor == vec3(0.0)) baseColor = vec3(1.0, 0.0, 0.0); // Debug: force red if texture fails

    vec3 specMap = texture(specularTexture, fTexCoords).rgb;

    // Compute Shadow Factor
    float shadow = computeShadow();

    // Compute Directional Lighting (Only If Enabled)
    vec3 dirLight = vec3(0.0);
    if (enableDirectionalLight == 1) {
        dirLight = CalcDirectionalLight(normalEye, viewDirEye);
    }

    // Compute Point Lights Contribution (Only If Enabled)
    vec3 pointLightsAccum = vec3(0.0);
    if (enablePointLight == 1) {
        for (int i = 0; i < numPointLights; i++) {
            pointLightsAccum += CalcPointLight(normalEye, fPosEye.xyz, viewDirEye, pointLightPositions[i]);
        }
        pointLightsAccum = baseColor * pointLightsAccum + specMap * pointLightsAccum;
    }

    // Final Lighting Result
    vec3 finalColor = (dirLight * baseColor) + (pointLightsAccum);

    // Apply Fog (if enabled)
    if (enableFog == 1) {
        float fogFactor = computeFogFactor();
        finalColor = mix(fogColor.rgb, finalColor, fogFactor);
    }

    fColor = vec4(finalColor, 1.0);
}

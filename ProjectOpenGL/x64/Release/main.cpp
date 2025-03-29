#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif
#pragma comment(lib, "winmm.lib")

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"
#include "Rain.hpp" 

#include <iostream>
#include <windows.h>

int glWindowWidth = 1280.0f;
int glWindowHeight = 720.0f;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;
float lastTimeStamp = 0.0;

const unsigned int SHADOW_WIDTH = 4096;
const unsigned int SHADOW_HEIGHT = 4096;

// Matrices
glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;

// ----------------------------------------------------------------------
// The "sun" (directional light)
// ----------------------------------------------------------------------
float lightAngle = 0.0f;
glm::mat4 lightRotation;

glm::vec3 baseSunPos(-140.0f, 60.0f, 70.0f);
glm::vec3 sceneCenter(0.0f, 0.0f, 0.0f);

glm::vec3  lightDir;
GLuint     lightDirLoc;

glm::vec3 lightColor;
GLuint     lightColorLoc;

// ----------------------------------------------------------------------
// Camera
// ----------------------------------------------------------------------
gps::Camera myCamera(
    glm::vec3(20.0f, 80.0f, 5.5f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f)
);
float cameraSpeed = 0.5f;
bool pressedKeys[1024];
bool firstMouse = true;
float lastX = glWindowWidth / 2.0f;
float lastY = glWindowHeight / 2.0f;
float yaw = -90.0f;
float pitch = 0.0f;

// ----------------------------------------------------------------------
// Models
// ----------------------------------------------------------------------
gps::Model3D scene;
gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D ghost;
// ----------------------------------------------------------------------
// Ghosty
// ----------------------------------------------------------------------
bool ghostAnimationActive = false; // Tracks if the animation is running
float ghostTime = 0.0f; // Timer for animation
float ghostHeight = 2.0f; // Adjust floating effect intensity
float ghostSpeed = 2.0f; // Controls animation speed
float ghostRotation = 5.0f; // Controls sway effect
float ghostX = 0.0f, ghostY =0.0f, ghostZ = 0.0f; // Adjust to place the ghost correctly
bool ghostOnlyAnimation = false; // Controls if ghost moves but POV stays normal

float deltaTimeGhost;
// ----------------------------------------------------------------------
// Shaders
// ----------------------------------------------------------------------
gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;
gps::Shader skyboxShader;
gps::Shader rainShader;
gps::Shader ghostShader;
// ----------------------------------------------------------------------
// Shadows
// ----------------------------------------------------------------------
GLuint shadowMapFBO;
GLuint depthMapTexture;
glm::mat4 lightSpaceTrMatrix;
bool showDepthMap = false;

// ----------------------------------------------------------------------
// Point Lights 
// ----------------------------------------------------------------------
bool enableDirectionalLight = true; // Directional Light starts ON

//glm::vec3 foodTruckPosition = glm::vec3(-20.0f, 1.0f, 10.0f);

#define NUM_OF_POINT_LIGHTS 3
glm::vec3 gPointLightPositions[3] = {
    glm::vec3(314.17, 60.12, 172.50),  // First light (left side, lowered)
    glm::vec3(314.17, 60.12, 222.50),  // Middle light (lowered slightly)
    glm::vec3(314.17, 60.12, 272.50)   // Right light (lowered and slightly adjusted)
};


glm::vec3 gPointLightColor = glm::vec3(1.0f, 0.9f, 0.7f);
float gPointLightAmbient = 0.2f;
float gPointLightDiffuse = 1.0f;
float gPointLightSpecular = 1.0f;
float gConstantAtt = 1.0f;
float gLinearAtt = 0.02f;
float gQuadraticAtt = 0.001f;
bool  pointLightEnabled = false;


// ----------------------------------------------------------------------
// Skybox
// ----------------------------------------------------------------------
std::vector<const GLchar*> faces;
gps::SkyBox mySkyBox;

// ----------------------------------------------------------------------
// Camera animation
// ----------------------------------------------------------------------
std::vector<glm::vec3> cameraPathPoints = {
    glm::vec3(-220.463f, 76.1223f, 60.5421f),
    glm::vec3(-188.463f, 68.1223f, 55.5421f),
    glm::vec3(-135.346f, 18.5052f, 21.6539f),
    glm::vec3(-79.5788f, 26.3867f, -9.06033f),
    glm::vec3(177.952f, 93.1747f, -34.8625f)
};

int  currentPathIndex = 0;
bool cameraAnimationActive = false;
float travelTime = 3.0f;
float pauseTime = 4.0f;
float segmentTimer = 0.0f;
bool isTraveling = true;

// ----------------------------------------------------------------------
// Night / Day Mode
// ----------------------------------------------------------------------
bool nightMode = false;
glm::vec3 daySunColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 nightSunColor = glm::vec3(0.2f, 0.2f, 0.4f);

// ----------------------------------------------------------------------
// ----------[ Fog, Wind, Rain ]-------------------------------
// ----------------------------------------------------------------------
bool  fogEnabled = false;
float gFogDensity = 0.02f;
glm::vec4 gFogColor = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

bool windEnabled = false;
bool rainEnabled = false;

RainSystem gRain(0, rainShader);

// ----------------------------------------------------------------------
// Check for GL errors
// ----------------------------------------------------------------------
GLenum glCheckError_(const char* file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
        case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
        case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FBO";   break;
        default:                               error = "UNKNOWN";       break;
        }
        std::cerr << "[OpenGL Error] " << error << " | " << file << " (" << line << ")\n";
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

// ----------------------------------------------------------------------
// Day / Night handler
// ----------------------------------------------------------------------
void handleDayAndNightMode() {
    faces.clear();
    if (nightMode) {
        faces.push_back("assets/skybox/night/night_east.jpg");
        faces.push_back("assets/skybox/night/night_west.jpg");
        faces.push_back("assets/skybox/night/night_uppp.jpg");
        faces.push_back("assets/skybox/night/night_down.jpg");
        faces.push_back("assets/skybox/night/night_back.jpg");
        faces.push_back("assets/skybox/night/night_front.jpg");
    }
    else {
        faces.push_back("assets/skybox/day/day_west.jpg");
        faces.push_back("assets/skybox/day/day_east.jpg");
        faces.push_back("assets/skybox/day/day_up.jpg");
        faces.push_back("assets/skybox/day/day_down.jpg");
        faces.push_back("assets/skybox/day/day_back.jpg");
        faces.push_back("assets/skybox/day/day_front.jpg");
    }

    mySkyBox.Load(faces);

    lightColor = nightMode ? nightSunColor : daySunColor;
    myCustomShader.useShaderProgram();
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
}


// ----------------------------------------------------------------------
// Mouse, Window & Keyboard callbacks
// ----------------------------------------------------------------------
void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse) {
        firstMouse = false;
        lastX = (float)xpos;
        lastY = (float)ypos;
    }

    float x_offset = (float)xpos - lastX;
    float y_offset = lastY - (float)ypos;
    lastX = (float)xpos;
    lastY = (float)ypos;

    float sensitivity = 0.1f;
    x_offset *= sensitivity;
    y_offset *= sensitivity;

    yaw += x_offset;
    pitch += y_offset;

    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    myCamera.rotate(pitch, yaw);
}
void windowResizeCallback(GLFWwindow* window, int width, int height)
{
    glWindowWidth = width;
    glWindowHeight = height;

    glfwGetFramebufferSize(window, &retina_width, &retina_height);
    glViewport(0, 0, retina_width, retina_height);

    float aspectRatio = (float)retina_width / (float)retina_height;
    projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 1000.0f);

    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

bool ghostPOV = false; // Track if we are in ghost mode

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        showDepthMap = !showDepthMap;
        std::cout << "[INFO] showDepthMap = "
            << (showDepthMap ? "true" : "false") << std::endl;
    }

    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        static int lightMode = 0; // 0 = Both ON, 1 = Directional Only, 2 = Point Only, 3 = Both OFF
        lightMode = (lightMode + 1) % 4;

        if (lightMode == 0) {
            std::cout << "[INFO] Both Directional & Point Lights Enabled\n";
            enableDirectionalLight = true;
            pointLightEnabled = true;
        }
        else if (lightMode == 1) {
            std::cout << "[INFO] Directional Light ONLY\n";
            enableDirectionalLight = true;
            pointLightEnabled = false;
        }
        else if (lightMode == 2) {
            std::cout << "[INFO] Point Light ONLY\n";
            enableDirectionalLight = false;
            pointLightEnabled = true;
        }
        else {
            std::cout << "[INFO] ALL Lights OFF\n";
            enableDirectionalLight = false;
            pointLightEnabled = false;
        }
    }

    if (key == GLFW_KEY_K && action == GLFW_PRESS) {
        pointLightEnabled = !pointLightEnabled;
        std::cout << "[INFO] Food Truck Lights = "
            << (pointLightEnabled ? "ON" : "OFF") << std::endl;
    }



    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        fogEnabled = !fogEnabled;
        std::cout << "[INFO] Fog enabled = "
            << (fogEnabled ? "true" : "false") << std::endl;
    }

    if (key == GLFW_KEY_G && action == GLFW_PRESS)
    {
        windEnabled = !windEnabled;
        std::cout << "[INFO] Wind enabled = "
            << (windEnabled ? "true" : "false") << std::endl;
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        rainEnabled = !rainEnabled;
        std::cout << "[INFO] Rain enabled = "
            << (rainEnabled ? "true" : "false") << std::endl;

        if (rainEnabled) {
            gRain.init();
        }
        else {
            gRain.destroy();
        }
    }

    if (key == GLFW_KEY_N && action == GLFW_PRESS) {
        nightMode = !nightMode;
        std::cout << "[INFO] Night mode = " << (nightMode ? "ON" : "OFF") << std::endl;
        handleDayAndNightMode();
    }

    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        cameraAnimationActive = true;
        currentPathIndex = 0;
        segmentTimer = 0.0f;
        isTraveling = true;
        myCamera.setCameraPosition(cameraPathPoints[0]);
        std::cout << "[INFO] Starting camera animation.\n";
    }

    if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
        ghostAnimationActive = !ghostAnimationActive; // Toggle animation
        ghostTime = 0.0f; // Reset animation timer when restarting
        ghostPOV = ghostAnimationActive; // Enable/disable ghost POV

        if (ghostPOV) {
            std::cout << "[INFO] Switched to Ghost POV" << std::endl;
            myCamera.setCameraPosition(glm::vec3(100.0f, 100.0f, ghostZ)); // Move higher above ghost
            myCamera.setCameraFront(glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f))); // Look downward
        }

        else {
            std::cout << "[INFO] Returning to Normal View" << std::endl;
            myCamera.setCameraPosition(glm::vec3(20.0f, 80.0f, 5.5f)); // Restore camera
        }

    }

    if (key == GLFW_KEY_X && action == GLFW_PRESS) {
        ghostAnimationActive = !ghostAnimationActive; // Toggle animation
        ghostOnlyAnimation = ghostAnimationActive; // Enable only ghost animation
        ghostPOV = false; // Ensure POV does not follow ghost

        std::cout << "[INFO] Ghost animation " << (ghostAnimationActive ? "STARTED" : "STOPPED") << std::endl;
    }
    // Movement keys should only affect the ghost if in ghost POV
    if (ghostPOV) {
        float ghostMoveSpeed = 0.5f;
        if (key == GLFW_KEY_W && action == GLFW_PRESS) ghostZ -= ghostMoveSpeed;
        if (key == GLFW_KEY_S && action == GLFW_PRESS) ghostZ += ghostMoveSpeed;
        if (key == GLFW_KEY_A && action == GLFW_PRESS) ghostX -= ghostMoveSpeed;
        if (key == GLFW_KEY_D && action == GLFW_PRESS) ghostX += ghostMoveSpeed;
    }



    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)
            pressedKeys[key] = true;
        else if (action == GLFW_RELEASE)
            pressedKeys[key] = false;
    }

}

void processMovement()
{
    float currentSpeed = cameraSpeed;

    if (ghostPOV) {
        float ghostMoveSpeed = 0.5f;

        if (pressedKeys[GLFW_KEY_W]) ghostZ -= ghostMoveSpeed; // Move forward
        if (pressedKeys[GLFW_KEY_S]) ghostZ += ghostMoveSpeed; // Move backward
        if (pressedKeys[GLFW_KEY_A]) ghostX -= ghostMoveSpeed; // Move left
        if (pressedKeys[GLFW_KEY_D]) ghostX += ghostMoveSpeed; // Move right

        myCamera.setCameraPosition(glm::vec3(ghostX, 5.0f, ghostZ)); // Sync camera to ghost
    }
    else {
        // Normal camera movement
        if (pressedKeys[GLFW_KEY_W]) myCamera.move(gps::MOVE_FORWARD, currentSpeed);
        if (pressedKeys[GLFW_KEY_S]) myCamera.move(gps::MOVE_BACKWARD, currentSpeed);
        if (pressedKeys[GLFW_KEY_A]) myCamera.move(gps::MOVE_LEFT, currentSpeed);
        if (pressedKeys[GLFW_KEY_D]) myCamera.move(gps::MOVE_RIGHT, currentSpeed);
    }

    if (pressedKeys[GLFW_KEY_J]) {
        lightAngle -= 1.0f;
    }
    if (pressedKeys[GLFW_KEY_L]) {
        lightAngle += 1.0f;
    }

    if (pressedKeys[GLFW_KEY_1]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    if (pressedKeys[GLFW_KEY_2]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }

    if (pressedKeys[GLFW_KEY_3]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

}

// ----------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------
bool initOpenGLWindow()
{
    if (!glfwInit()) {
        std::cerr << "[ERROR] Could not start GLFW\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

#if defined (__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight,
        "GP Project - Pop Alexandra",
        nullptr, nullptr);
    if (!glWindow) {
        std::cerr << "ERROR: Could not open window with GLFW3\n";
        glfwTerminate();
        return false;
    }

    glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
    glfwSetKeyCallback(glWindow, keyboardCallback);
    glfwSetCursorPosCallback(glWindow, mouseCallback);
    glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwMakeContextCurrent(glWindow);
    glfwSwapInterval(1);

#if !defined (__APPLE__)
    glewExperimental = GL_TRUE;
    glewInit();
#endif

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* ver = glGetString(GL_VERSION);
    std::cout << "Renderer: " << renderer << "\n";
    std::cout << "OpenGL version supported: " << ver << "\n";

    glfwGetWindowSize(glWindow, &glWindowWidth, &glWindowHeight);
    glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);
    glViewport(0, 0, retina_width, retina_height);

    return true;
}

void initOpenGLState() {
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glViewport(0, 0, retina_width, retina_height);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glDisable(GL_CULL_FACE);

    glEnable(GL_FRAMEBUFFER_SRGB);
}

void initSkybox() {
    faces.push_back("assets/skybox/day/day_west.jpg");
    faces.push_back("assets/skybox/day/day_east.jpg");
    faces.push_back("assets/skybox/day/day_up.jpg");
    faces.push_back("assets/skybox/day/day_down.jpg");             
    faces.push_back("assets/skybox/day/day_back.jpg");
    faces.push_back("assets/skybox/day/day_front.jpg");

    mySkyBox.Load(faces);
}

void initObjects() {
    scene.LoadModel("C:/Users/andra/source/repos/ProjectOpenGL/ProjectOpenGL/assets/model/scene/zscene.obj");
    lightCube.LoadModel("C:/Users/andra/source/repos/ProjectOpenGL/ProjectOpenGL/assets/model/scene/cube.obj");
    screenQuad.LoadModel("C:/Users/andra/source/repos/ProjectOpenGL/ProjectOpenGL/assets/model/scene/quad.obj");
    ghost.LoadModel("C:/Users/andra/source/repos/ProjectOpenGL/ProjectOpenGL/assets/model/scene/Halloween/Halloween_Ghost.obj");
}

void initShaders() {
    myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
    myCustomShader.useShaderProgram();

    lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
    lightShader.useShaderProgram();

    screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
    screenQuadShader.useShaderProgram();

    depthMapShader.loadShader("shaders/shadow.vert", "shaders/shadow.frag");
    depthMapShader.useShaderProgram();

    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();

    rainShader.loadShader("shaders/rainShader.vert", "shaders/rainShader.frag");
    rainShader.useShaderProgram();

	ghostShader.loadShader("shaders/ghostShader.vert", "shaders/ghostShader.frag");
	ghostShader.useShaderProgram();

    gRain = RainSystem(50000, rainShader);
}

void initUniforms() {
    myCustomShader.useShaderProgram();

    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    float aspectRatio = (float)glWindowWidth / (float)glWindowHeight;
    projection = glm::perspective(glm::radians(45.0f),
        aspectRatio,
        0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");

    lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"),
        1, GL_FALSE, glm::value_ptr(projection));

    depthMapShader.useShaderProgram();
    glUniform1i(glGetUniformLocation(depthMapShader.shaderProgram, "shadowMap"), 0);
}

void initFBO() {
    glGenFramebuffers(1, &shadowMapFBO);

    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT,
        0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    float borderColor[] = { 1.0f,1.0f,1.0f,1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR::FRAMEBUFFER:: Shadow map framebuffer is not complete!\n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// ----------------------------------------------------------------------
// Light Space Matrix
// ----------------------------------------------------------------------
glm::mat4 computeLightSpaceTrMatrix() {
    glm::mat4 rotateSun = glm::rotate(glm::mat4(1.0f),
        glm::radians(lightAngle),
        glm::vec3(0.0f, 5.0f, 0.0f));

    glm::vec3 currentSunPos = glm::vec3(rotateSun * glm::vec4(baseSunPos, 1.0f));

    glm::mat4 lightView = glm::lookAt(
        currentSunPos,
        sceneCenter,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    float orthoSize = 50.0f;
    float nearPlane = -100.0f;
    float farPlane = 300.0f;

    glm::mat4 lightProjection = glm::ortho(
        -orthoSize, orthoSize,
        -orthoSize, orthoSize,
        nearPlane, farPlane
    );

    lightSpaceTrMatrix = lightProjection * lightView;
    return lightSpaceTrMatrix;
}

// ----------------------------------------------------------------------
// Camera Animation
// ----------------------------------------------------------------------
void updateCameraAnimation(float deltaTime) {
    if (!cameraAnimationActive) {
        return;
    }

    if (currentPathIndex >= (int)cameraPathPoints.size() - 1) {
        cameraAnimationActive = false;
        return;
    }

    segmentTimer += deltaTime;

    if (isTraveling) {
        float t = segmentTimer / travelTime;
        if (t >= 1.0f) {
            t = 1.0f;
            myCamera.setCameraPosition(cameraPathPoints[currentPathIndex + 1]);

            isTraveling = false;
            segmentTimer = 0.0f;
        }
        else {
            glm::vec3 startPos = cameraPathPoints[currentPathIndex];
            glm::vec3 endPos = cameraPathPoints[currentPathIndex + 1];
            glm::vec3 newPos = glm::mix(startPos, endPos, t);
            myCamera.setCameraPosition(newPos);
        }
    }
    else {
        if (segmentTimer >= pauseTime) {
            segmentTimer = 0.0f;
            isTraveling = true;
            currentPathIndex++;

            if (currentPathIndex >= (int)cameraPathPoints.size() - 1) {
                cameraAnimationActive = false;
            }
        }
    }
}

// ----------------------------------------------------------------------
// Scene drawing logic here
// ----------------------------------------------------------------------
void drawScene(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    glm::mat4 rotScene = glm::mat4(1.0f);

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"),
        1, GL_FALSE, glm::value_ptr(rotScene));

    if (!depthPass) {
        glm::mat3 normMat = glm::mat3(glm::inverseTranspose(view * rotScene));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normMat));
    }

    scene.Draw(shader);
}
void drawGhost(gps::Shader& shader) {
    shader.useShaderProgram();

    // Update animation transformation
    glm::mat4 modelGhost = glm::mat4(1.0f);

    if (ghostAnimationActive) {
        // Floating effect (up and down motion)
        float yOffset = ghostHeight * sin(ghostSpeed * ghostTime);

        // Swaying effect (subtle rotation)
        float rotationAngle = ghostRotation * cos(ghostSpeed * ghostTime * 0.5f);

        // Pulsating scale effect
        float scaleFactor = 1.0f + 0.1f * sin(ghostSpeed * ghostTime * 2.0f); // Oscillates between 0.9 and 1.1

        // Apply transformations
        modelGhost = glm::translate(modelGhost, glm::vec3(ghostX, yOffset, ghostZ));
        modelGhost = glm::rotate(modelGhost, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        modelGhost = glm::scale(modelGhost, glm::vec3(scaleFactor)); // Scaling effect added

        // If in ghost POV, sync the camera with the ghost's position
        if (ghostPOV) {
            myCamera.setCameraPosition(glm::vec3(ghostX, ghostY + 2.5f, ghostZ));
        }
    }
    else {
        // Default position when animation is off
        modelGhost = glm::translate(modelGhost, glm::vec3(ghostX, 0.0f, ghostZ));
    }

    // **Increase Ghost Size** (Try adjusting the values)
    modelGhost = glm::scale(modelGhost, glm::vec3(30.0f, 30.0f, 30.0f)); // Adjust scale factor here

    // Send transformation matrix to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelGhost));

    // Send normal matrix for proper lighting calculations
    glm::mat3 ghostNormalMatrix = glm::mat3(glm::inverseTranspose(view * modelGhost));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(ghostNormalMatrix));

    // Set the isGhost uniform to enable ghost-specific effects in the shader
    glUniform1i(glGetUniformLocation(shader.shaderProgram, "isGhost"), 1);

    // Draw the ghost model
    ghost.Draw(shader);
}


void updateDeltaTimeGhost(double elapsedSeconds) {
    if (ghostAnimationActive) {
        ghostTime += static_cast<float>(elapsedSeconds); // Accumulate elapsed time
    }
}

// ----------------------------------------------------------------------
// Main render function run at each frame
// ----------------------------------------------------------------------


void renderScene(float deltaTime) {
    // 1) Day / Night mode set light brightness
    myCustomShader.useShaderProgram();
    float brightness = nightMode ? 0.5f : 1.0f;
    myCustomShader.useShaderProgram();
    glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "lightBrightness"), brightness);

    // 2) SHADOW PASS: Render scene from the sun's POV
    depthMapShader.useShaderProgram();

    GLint timeShadowLoc = glGetUniformLocation(depthMapShader.shaderProgram, "time");
    float currentTime = (float)glfwGetTime();
    glUniform1f(timeShadowLoc, currentTime);

    GLint windShadowLoc = glGetUniformLocation(depthMapShader.shaderProgram, "enableWind");
    glUniform1i(windShadowLoc, (windEnabled ? 1 : 0));

    glm::mat4 lightSpace = computeLightSpaceTrMatrix();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram,
        "lightSpaceTrMatrix"),
        1, GL_FALSE, glm::value_ptr(lightSpace));

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    drawScene(depthMapShader, true);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 3) Set up point lights 
    myCustomShader.useShaderProgram();

    GLint numPLoc = glGetUniformLocation(myCustomShader.shaderProgram, "numPointLights");
    glUniform1i(numPLoc, NUM_OF_POINT_LIGHTS);

    glm::vec3 plPosEye[NUM_OF_POINT_LIGHTS];
    for (int i = 0; i < NUM_OF_POINT_LIGHTS; i++) {
        glm::vec4 eyeSpacePos = view * glm::vec4(gPointLightPositions[i], 1.0f);
        plPosEye[i] = glm::vec3(eyeSpacePos);
    }

    GLint plArrayLoc = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightPositions");
    glUniform3fv(plArrayLoc, NUM_OF_POINT_LIGHTS, glm::value_ptr(plPosEye[0]));

    glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightColor"),
        1, glm::value_ptr(gPointLightColor));

    glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightAmbient"),
        gPointLightAmbient);
    glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightDiffuse"),
        gPointLightDiffuse);
    glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightSpecular"),
        gPointLightSpecular);
    glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "constantAtt"),
        gConstantAtt);
    glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "linearAtt"),
        gLinearAtt);
    glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "quadraticAtt"),
        gQuadraticAtt);

    glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "enablePointLight"),
        (pointLightEnabled ? 1 : 0));
    glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "enableDirectionalLight"), (enableDirectionalLight ? 1 : 0));


    // 4) If user wants to see the Depth Map, show it and return
    if (showDepthMap) {
        glViewport(0, 0, retina_width, retina_height);
        glClear(GL_COLOR_BUFFER_BIT);
        screenQuadShader.useShaderProgram();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

        glDisable(GL_DEPTH_TEST);
        screenQuad.Draw(screenQuadShader);
        glEnable(GL_DEPTH_TEST);
        return;
    }

    // 5) NORMAL PASS: Render the scene from the camera
    glViewport(0, 0, retina_width, retina_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    myCustomShader.useShaderProgram();

    GLint timeMainLoc = glGetUniformLocation(myCustomShader.shaderProgram, "time");
    glUniform1f(timeMainLoc, currentTime);

    GLint windMainLoc = glGetUniformLocation(myCustomShader.shaderProgram, "enableWind");
    glUniform1i(windMainLoc, (windEnabled ? 1 : 0));

    GLint rainEnabled = glGetUniformLocation(myCustomShader.shaderProgram, "rainEnabled");
    glUniform1i(rainEnabled, (rainEnabled ? 1 : 0));

    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram,
        "lightSpaceTrMatrix"),
        1, GL_FALSE, glm::value_ptr(lightSpace));

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

    glm::mat4 rotateSun = glm::rotate(glm::mat4(1.0f),
        glm::radians(lightAngle),
        glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec3 currentSunPos = glm::vec3(rotateSun * glm::vec4(baseSunPos, 1.0f));

    glm::vec3 newLightDir = glm::normalize(sceneCenter - currentSunPos);
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(newLightDir));

    // 5) Pass the fog uniforms
    glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "enableFog"),
        (fogEnabled ? 1 : 0));
    glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity"),
        gFogDensity);
    glUniform4fv(glGetUniformLocation(myCustomShader.shaderProgram, "fogColor"),
        1, glm::value_ptr(gFogColor));

    // 6) Draw the scene and the ghost 
    drawScene(myCustomShader, false);
    drawGhost(myCustomShader);

    // 7) Draw the rain
    if (rainEnabled) {
        gRain.update(deltaTime);

        gRain.uploadToGPU();

        lightShader.useShaderProgram();

        glm::mat4 view = myCamera.getViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"),
            1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"),
            1, GL_FALSE, glm::value_ptr(projection));

        GLint colorLoc = glGetUniformLocation(lightShader.shaderProgram, "objectColor");
        if (colorLoc >= 0) {
            glUniform3f(colorLoc, 0.65f, 0.65f, 1.0f);
        }

        gRain.draw(projection, view);
    }

    // 8) Draw the "sun" cube
    lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"),
        1, GL_FALSE, glm::value_ptr(view));

    model = glm::mat4(1.0f);
    model = glm::translate(model, currentSunPos);
    model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"),
        1, GL_FALSE, glm::value_ptr(model));

    lightCube.Draw(lightShader);

    // 9) Draw skybox
    mySkyBox.Draw(skyboxShader, view, projection);

}

void cleanup() {
    glDeleteTextures(1, &depthMapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &shadowMapFBO);

    glfwDestroyWindow(glWindow);
    glfwTerminate();
}

void debugCameraPosition() {
    glm::vec3 camPos = myCamera.getCameraPosition();
    std::cout << "Camera position: ("
        << camPos.x << ", "
        << camPos.y << ", "
        << camPos.z << ")" << std::endl;
}

int main(int argc, const char* argv[])
{
    if (!initOpenGLWindow()) {
        glfwTerminate();
        return 1;
    }

    initOpenGLState();
    initObjects();
    initSkybox();
    initShaders();
    initUniforms();
    initFBO();

    glCheckError();

    lastTimeStamp = glfwGetTime();

    while (!glfwWindowShouldClose(glWindow)) {
        double currentTimeStamp = glfwGetTime();
        float deltaTime = static_cast<float>(currentTimeStamp - lastTimeStamp);
        updateDeltaTimeGhost(currentTimeStamp - lastTimeStamp);
        lastTimeStamp = currentTimeStamp;

        processMovement();
        updateCameraAnimation(deltaTime);
        renderScene(deltaTime);
        //debugCameraPosition();

        glfwPollEvents();
        glfwSwapBuffers(glWindow);
    }

    cleanup();
    return 0;
}
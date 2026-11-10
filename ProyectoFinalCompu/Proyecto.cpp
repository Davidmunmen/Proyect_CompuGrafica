//Proyecto final
#include <iostream>
#include <cmath>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include "stb_image.h"
#include <vector>
#include <algorithm>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//Load Models
#include "SOIL2/SOIL2.h"

// Other includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Texture.h"
#include <vector>

// Function prototypes
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();
void Animation();

// Window dimensions
const GLuint WIDTH = 1000, HEIGHT = 800;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera (un poco más atrás y alto para ver el arco)
Camera camera(glm::vec3(0.0f, 2.0f, 6.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;

// Light attributes
glm::vec3 lightPos(0.0f, 2.0f, 0.0f);

// Positions of the point lights
glm::vec3 pointLightPositions[] = {
    glm::vec3(0.0f, 2.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f)
};
glm::vec3 todasLasVelas[] = {
    // Arco 1 (Frente)
    glm::vec3(0.7f, -0.09f, 2.3f),  // Vela Derecha
    glm::vec3(-0.7f, -0.09f, 2.3f), // Vela Izquierda
    // Arco 2 (Centro)
    glm::vec3(0.7f, -0.09f, -2.0f), // Vela Derecha
    glm::vec3(-0.7f, -0.09f, -2.0f),// Vela Izquierda
    // Arco 3 (Fondo)
    glm::vec3(0.7f, -0.09f, -6.0f), // Vela Derecha
    glm::vec3(-0.7f, -0.09f, -6.0f),// Vela Izquierda
    // Velas extra cerca de la ofrenda (por si acaso)
    glm::vec3(0.7f, -0.09f, -10.0f),
    glm::vec3(-0.7f, -0.09f, -10.0f)
};
const int NUM_VELAS_TOTAL = 8;
const int MAX_LUCES = 4;

glm::vec3 velaPositions[] = {
    glm::vec3(1.0f, -0.09f, 2.3f),
    glm::vec3(0.7f, -0.09f, 2.3f),
    glm::vec3(-1.0f, -0.09f, 2.3f),
    glm::vec3(-0.7f, -0.09f, 2.3f)
};

// Deltatime
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// Variables de animación jerárquica para las extremidades del personaje
GLfloat rotacionBrazos = 0.0f;
GLfloat rotacionPiernas = 0.0f;

// Variables para la animación de baile de la mujer
float anguloBaile = 0.0f;           // torso (contra-balanceo)
float anguloFalda = 0.0f;           // cadera/falda (nodo padre de las piernas)
float rotacionPiernasMujer = 0.0f;  // paso leve en X
bool animandoBaile = false;

// Estados de la máquina de estados para el baile
int estadoBaile = 0;
const float MAX_BAILE = 2.0f;               
const float MAX_FALDA = 1.5f;               
const float MAX_PIERNA = 2.0f;              
const float VEL_BAILE = 6.0f;
const float VEL_FALDA = 4.5f;
const float VEL_PIERNA = 6.0f;            

// Si el muslo (musDer) se mueve con la pierna contraria, cambia este valor a false.
// true  = el modelo musDer esta modelado del lado x < 0 (lado derecho del personaje)
// false = el modelo musDer esta modelado del lado x > 0
const bool MUSDER_NATIVO_EN_X_NEGATIVO = true;


const glm::vec3 pivoteHombroDer(-0.5f, 4.0f, 0.0f);
const glm::vec3 pivoteHombroIzq(0.5f, 4.0f, 0.0f);
const glm::vec3 pivoteCaderaDer(-0.3f, 2.2f, 0.0f);
const glm::vec3 pivoteCaderaIzq(0.3f, 2.2f, 0.0f);
glm::vec3 pivoteCintura(0.0f, 2.4f, 0.0f);

// Rota alrededor de un pivote (con opción de eje)
glm::mat4 RotarEnPivote(const glm::mat4& base, const glm::vec3& pivote, float grados, glm::vec3 eje = glm::vec3(1.0f, 0.0f, 0.0f))
{
    glm::mat4 m = glm::translate(base, pivote);
    m = glm::rotate(m, glm::radians(grados), eje);
    m = glm::translate(m, -pivote);
    return m;
}

// Función para mover un valor hacia un objetivo suavemente (usada en la máquina de estados)
bool MoverHacia(float& valor, float objetivo, float velocidad)
{
    float paso = velocidad * deltaTime;
    if (valor < objetivo) { valor += paso; if (valor > objetivo) valor = objetivo; }
    else if (valor > objetivo) { valor -= paso; if (valor < objetivo) valor = objetivo; }
    return valor == objetivo;
}

int main()
{
    // Init GLFW
    glfwInit();

    // Create a GLFWwindow object
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Proyecto", nullptr, nullptr);

    if (nullptr == window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    // Callbacks
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);

    // GLEW
    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    // Viewport
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Shaders
    Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
    Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");
    Shader skyboxshader("Shader/SkyBox.vs", "Shader/SkyBox.frag");//skybox
    Shader papelShader("Shader/papel.vs", "Shader/papel.frag"); //papel picado


    // Modelos:  piso y arco
    Model Piso((char*)"Models/Arco/piso.obj");
    Model base((char*)"Models/Arco/base.obj");
    Model PisoA((char*)"Models/Arco/pisoA.obj");
    Model velas((char*)"Models/velas/velas.obj");
    Model ofrenda((char*)"Models/ofrenda/ofrenda.obj");
    Model copa((char*)"Models/COPA.obj");
    // Modelos del Personaje Hombre
    Model cabeza((char*)"Models/personaje/cabeza/cabeza.obj");
    Model biceps((char*)"Models/personaje/bicep/bicep.obj");
    Model cuerpo((char*)"Models/personaje/cuerpo/cuerpo.obj");
    Model manoDer((char*)"Models/personaje/manoDer/manoDer.obj");
    Model manoIzq((char*)"Models/personaje/manoIzq/manoIzq.obj");
    Model mascara((char*)"Models/personaje/mascara/mascara.obj");
    Model musDer((char*)"Models/personaje/musDer/musDer.obj");
    Model pieiz((char*)"Models/personaje/pieizq/PieIzq.obj");

    // Modelos de la Mujer
    Model torso_mujer((char*)"Models/personaje/torso_mujer/torso_mujer.obj");
    Model falda_mujer((char*)"Models/personaje/torso_mujer/falda_mujer.obj");

    //Modelos Abueitos

    Model mascaraA((char*)"Models/personaje/mascaraA/mascaraA.obj");


    //Cementerio
    
    Model tumba((char*)"Models/Cementerio/Tumba/TumbaF.obj");
    Model arcoP((char*)"Models/Cementerio/arcoP/arcoP.obj");
    Model campana((char*)"Models/Cementerio/campana/campana.obj");
    Model velass((char*)"Models/Cementerio/velass/velassimples.obj");
    Model cempa((char*)"Models/Cementerio/cempa/cempa.obj");
   
    // Set texture units
    lightingShader.Use();
    glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.difuse"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.specular"), 1);

    //================= Vértices papel picado =======================
    float quadVertices[] = {
         0.5f,  0.5f, 0.0f,   1.0f, 1.0f, // Arriba Derecha
         0.5f, -0.5f, 0.0f,   1.0f, 0.0f, // Abajo Derecha
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, // Abajo Izquierda
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f  // Arriba Izquierda
    };

    unsigned int quadIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    //Vertices skybox 
    GLfloat skyboxVertices[] = {
        // Positions
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    // ===================== CONFIGURACIÓN DEL SKYBOX (VAO, VBO y TEXTURAS) =====================
    GLuint skyboxVBO, skyboxVAO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

    //Load textures
    vector < const GLchar*> faces;
    faces.push_back("SkyBox/right.jpg");
    faces.push_back("SkyBox/left.jpg");
    faces.push_back("SkyBox/top.jpg");
    faces.push_back("SkyBox/bottom.jpg");
    faces.push_back("SkyBox/back.jpg");
    faces.push_back("SkyBox/front.jpg");

    GLuint cubmapTexture = TextureLoading::LoadCubemap(faces);

    // ============================Configuración papel =====================================
    glm::mat4 projection = glm::perspective(camera.GetZoom(),
        (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 200.0f);

    GLuint quadVAO, quadVBO, quadEBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glGenBuffers(1, &quadEBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    GLuint papelTexture1 = SOIL_load_OGL_texture(
        "Models/papel.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );

    GLuint papelTexture2 = SOIL_load_OGL_texture(
        "Models/papel2.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );

    GLuint papelTexture3 = SOIL_load_OGL_texture(
        "Models/papel3.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );

    GLuint humoTexture = SOIL_load_OGL_texture(
        "Models/humo.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();
        Animation();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);

        // ===================== SHADER DE ILUMINACIÓN =====================
        lightingShader.Use();

        glUniform1i(glGetUniformLocation(lightingShader.Program, "diffuse"), 0);

        GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
        glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

        // Directional light
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.12f, 0.12f, 0.15f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.15f, 0.15f, 0.15f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.05f, 0.05f, 0.05f);

        // ===== ASIGNACIÓN DINÁMICA DE LUCES =====
        glm::vec3 velaColor(1.0f, 0.6f, 0.2f);

        std::vector<std::pair<float, int>> distancias;
        for (int i = 0; i < NUM_VELAS_TOTAL; i++) {
            glm::vec3 posFlama = todasLasVelas[i] + glm::vec3(0.0f, 0.35f, 0.0f);
            float dist = glm::length(camera.GetPosition() - posFlama);
            distancias.push_back({ dist, i });
        }

        std::sort(distancias.begin(), distancias.end());

        for (int i = 0; i < MAX_LUCES; i++)
        {
            std::string base = "pointLights[" + std::to_string(i) + "].";
            int velaIndex = distancias[i].second;
            glm::vec3 lp = todasLasVelas[velaIndex] + glm::vec3(0.0f, 0.35f, 0.0f);

            float flicker = 0.80f
                + 0.15f * sin(currentFrame * 8.0f + velaIndex * 1.7f)
                + 0.05f * sin(currentFrame * 23.0f + velaIndex * 3.1f);
            float fade = glm::clamp(1.0f - (distancias[i].first - 8.0f) / 4.0f, 0.0f, 1.0f);

            glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "position").c_str()), lp.x, lp.y, lp.z);
            glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "ambient").c_str()), velaColor.x * 0.05f, velaColor.y * 0.05f, velaColor.z * 0.05f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "diffuse").c_str()), velaColor.x * flicker * fade * 0.6f, velaColor.y * flicker * fade * 0.6f, velaColor.z * flicker * fade * 0.6f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "specular").c_str()), 0.0f, 0.0f, 0.0f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "constant").c_str()), 1.0f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "linear").c_str()), 0.35f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "quadratic").c_str()), 0.44f);
        }

        // SpotLight
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.02f, 0.02f, 0.02f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), 0.05f, 0.05f, 0.08f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), 0.0f, 0.0f, 0.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.3f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.7f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(12.0f)));
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(18.0f)));

        // Material
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 5.0f);

        // Camera matrices
        glm::mat4 view = camera.GetViewMatrix();
        GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
        GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // ===================== PISO =====================
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.0f, 0.0f, -2.0f));
        model = glm::scale(model, glm::vec3(0.4f, 0.2f, 2.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        Piso.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, -2.0f));
        model = glm::scale(model, glm::vec3(0.4f, 0.2f, 2.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        Piso.Draw(lightingShader);


        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.0f, 0.0f, -12.0f));
        model = glm::scale(model, glm::vec3(0.4f, 0.2f, 2.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        Piso.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, -12.0f));
        model = glm::scale(model, glm::vec3(0.4f, 0.2f, 2.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        Piso.Draw(lightingShader);


        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.1f, -16.2f));
        model = glm::scale(model, glm::vec3(0.8f, 0.0f, 0.7f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        PisoA.Draw(lightingShader);


        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.1f, -12.7f));
        model = glm::scale(model, glm::vec3(0.8f, 0.0f, 0.7f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        PisoA.Draw(lightingShader);


        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.1f, -9.2f));
        model = glm::scale(model, glm::vec3(0.8f, 0.0f, 0.7f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        PisoA.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.1f, -5.7f));
        model = glm::scale(model, glm::vec3(0.8f, 0.0f, 0.7f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        PisoA.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.1f, -2.2f));
        model = glm::scale(model, glm::vec3(0.8f, 0.0f, 0.7f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        PisoA.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.1f, 1.3f));
        model = glm::scale(model, glm::vec3(0.8f, 0.0f, 0.7f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        PisoA.Draw(lightingShader);

       


        // ===================== ARCO =====================
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.1f, 1.8f));
        model = glm::scale(model, glm::vec3(1.1f, 1.1f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        base.Draw(lightingShader);



        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.1f, -2.5f));
        model = glm::scale(model, glm::vec3(1.0f, 1.1f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        base.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.1f, -6.5f));
        model = glm::scale(model, glm::vec3(1.1f, 1.1f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        base.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.1f, -10.5f));
        model = glm::scale(model, glm::vec3(1.1f, 1.1f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        base.Draw(lightingShader);


        // ===================== Cementerio =====================

        //--Tumbas---
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.4f, -0.2f, -12.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        tumba.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.4f, -0.2f, -12.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        tumba.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.4f, -0.2f, -13.5f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        tumba.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.4f, -0.2f, -13.5f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        tumba.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.4f, -0.2f, -15.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        tumba.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.4f, -0.2f, -15.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        tumba.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.4f, -0.2f, -16.5f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        tumba.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.4f, -0.2f, -16.5f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        tumba.Draw(lightingShader);

        //--Arco---
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.07f, -11.0f));
        model = glm::scale(model, glm::vec3(3.15f, 3.15f, 3.15f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        arcoP.Draw(lightingShader);

        //--Campana--
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 1.51f, -11.0f));
        model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        campana.Draw(lightingShader);

        //--Velass--

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -28.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        velass.Draw(lightingShader);

        //--Cempa--

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.001213f, -0.01825f, -14.27f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        cempa.Draw(lightingShader);

        //--ArcoCempa--

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.77f, -0.1f, -12.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        base.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.77f, -0.1f, -13.5f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        base.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.77f, -0.1f, -15.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        base.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.77f, -0.1f, -16.5f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        base.Draw(lightingShader);


        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.77f, -0.1f, -12.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        base.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.77f, -0.1f, -13.5f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        base.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.77f, -0.1f, -15.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        base.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.77f, -0.1f, -16.5f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        base.Draw(lightingShader);





        // ===================== VELAS =====================
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.7f, -0.09f, 2.3f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        velas.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.7f, -0.09f, 2.3f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        velas.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.7f, -0.09f, -2.0f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        velas.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.7f, -0.09f, -2.0f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        velas.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.7f, -0.09f, -6.0f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        velas.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.7f, -0.09f, -6.0f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        velas.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.7f, -0.09f, -10.0f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        velas.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.7f, -0.09f, -10.0f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        velas.Draw(lightingShader);
        // ===================== OFRENDA =====================
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.0f, 0.0f, -4.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        ofrenda.Draw(lightingShader);

        // ===================== PERSONAJE HOMBRE =====================
        glm::mat4 matrixModelCuerpo = glm::mat4(1.0f);
        matrixModelCuerpo = glm::translate(matrixModelCuerpo, glm::vec3(-2.0f, 0.75f, 0.0f));

        model = matrixModelCuerpo;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        cuerpo.Draw(lightingShader);
        cabeza.Draw(lightingShader);
        mascara.Draw(lightingShader);

        glm::mat4 matrixModelBrazoDer = RotarEnPivote(matrixModelCuerpo, pivoteHombroDer, rotacionBrazos);
        model = matrixModelBrazoDer;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        biceps.Draw(lightingShader);
        manoDer.Draw(lightingShader);

        glm::mat4 matrixModelBrazoIzq = RotarEnPivote(matrixModelCuerpo, pivoteHombroIzq, -rotacionBrazos);
        model = glm::translate(matrixModelBrazoIzq, glm::vec3(0.06f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        biceps.Draw(lightingShader);

        model = matrixModelBrazoIzq;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        manoIzq.Draw(lightingShader);

        glm::mat4 matrixModelPiernaDer = RotarEnPivote(matrixModelCuerpo, pivoteCaderaDer, rotacionPiernas);
        model = matrixModelPiernaDer;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        musDer.Draw(lightingShader);

        model = glm::scale(matrixModelPiernaDer, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pieiz.Draw(lightingShader);

        glm::mat4 matrixModelPiernaIzq = RotarEnPivote(matrixModelCuerpo, pivoteCaderaIzq, -rotacionPiernas);
        model = glm::scale(matrixModelPiernaIzq, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        musDer.Draw(lightingShader);

        model = matrixModelPiernaIzq;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pieiz.Draw(lightingShader);

        // ===================== PERSONAJE MUJER (Corregido con alineación de torso/vestido) =====================
        glm::mat4 matrixModelCuerpoMujer = glm::mat4(1.0f);
        matrixModelCuerpoMujer = glm::translate(matrixModelCuerpoMujer, glm::vec3(-1.3f, 0.6f, 0.0f));

        const glm::vec3 ejeZ(0.0f, 0.0f, 1.0f);

        // 1. Torso: se inclina desde la cintura (le agregamos un leve desplazamiento local en Z o X si es necesario centrarlo con la falda)
        glm::mat4 matrixTorsoMujer = RotarEnPivote(matrixModelCuerpoMujer, pivoteCintura, anguloBaile, ejeZ);
        // Desplazamiento correctivo local para meter el torso dentro del volumen de la falda (ajusta estos valores si requieres más o menos espacio)
 
        model = matrixTorsoMujer;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        torso_mujer.Draw(lightingShader);

        // 2. Falda: sincronizada con el ángulo del torso
        glm::mat4 matrixFaldaMujer = RotarEnPivote(matrixModelCuerpoMujer, pivoteCintura, anguloBaile, ejeZ);
        model = matrixFaldaMujer;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        falda_mujer.Draw(lightingShader);

        // 3. Cabeza (hereda del torso ya desplazado y sincronizado)
        model = matrixTorsoMujer;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        cabeza.Draw(lightingShader);

        // 4. Brazo derecho mujer (hijo del torso)
        glm::mat4 matrixModelBrazoDerMujer = RotarEnPivote(matrixTorsoMujer, pivoteHombroDer, rotacionBrazos);
        model = glm::translate(matrixModelBrazoDerMujer, glm::vec3(0.0f, 0.02f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        biceps.Draw(lightingShader);

        model = matrixModelBrazoDerMujer;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        manoDer.Draw(lightingShader);

        // Brazo izquierdo mujer (hijo del torso)
        glm::mat4 matrixModelBrazoIzqMujer = RotarEnPivote(matrixTorsoMujer, pivoteHombroIzq, -rotacionBrazos);
        model = glm::translate(matrixModelBrazoIzqMujer, glm::vec3(0.03f, 0.02f, 0.0f));
        model = glm::scale(model, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        biceps.Draw(lightingShader);

        model = glm::translate(matrixModelBrazoIzqMujer, glm::vec3(-0.03f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        manoIzq.Draw(lightingShader);

        // --- PIERNA DERECHA ---
        // Rotamos desde la cadera derecha y aplicamos una leve separación en X si es necesario
        glm::mat4 matrixPiernaDer = RotarEnPivote(matrixModelCuerpoMujer, pivoteCaderaDer, rotacionPiernasMujer);
        // Si el modelo del muslo nativo requiere un pequeño ajuste lateral de separación:
        matrixPiernaDer = glm::translate(matrixPiernaDer, glm::vec3(-0.13f, 0.0f, 0.0f));
		matrixPiernaDer = glm::scale(matrixPiernaDer, glm::vec3(0.9f, 0.8f, 0.9f)); // Ajuste de escala si es necesario
        model = matrixPiernaDer;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        musDer.Draw(lightingShader);

        // Pie derecho (hereda del muslo derecho)
        model = matrixPiernaDer;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pieiz.Draw(lightingShader);

        // --- PIERNA IZQUIERDA ---
        // Rotamos desde la cadera izquierda con signo opuesto para el contrapaso
        glm::mat4 matrixPiernaIzq = RotarEnPivote(matrixModelCuerpoMujer, pivoteCaderaIzq, -rotacionPiernasMujer);
        // Desplazamiento lateral simétrico para separarla del centro
        matrixPiernaIzq = glm::translate(matrixPiernaIzq, glm::vec3(0.00f, 0.0f, 0.0f));
		matrixPiernaIzq = glm::scale(matrixPiernaIzq, glm::vec3(0.9f, 0.8f, 0.9f)); // Ajuste de escala si es necesario

        model = matrixPiernaIzq;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        musDer.Draw(lightingShader);

        // Pie izquierdo (hereda del muslo izquierdo)
        model = matrixPiernaIzq;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pieiz.Draw(lightingShader);
        // ===================== PERSONAJE ABUELITO =====================

        //Numero 1------------
        glm::mat4 matrixModelCuerpoA = glm::mat4(1.0f);
        // Traslación general: mueve TODO el modelo (ej. -3.0f hacia la izquierda en X)
        matrixModelCuerpoA = glm::translate(matrixModelCuerpoA, glm::vec3(2.0f, 0.75f, -9.2f));

        // 1. Dibujar Cuerpo y Cabeza (Tamaño y posición normal)
        model = matrixModelCuerpoA;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        cuerpo.Draw(lightingShader);
        cabeza.Draw(lightingShader);

        // 2. Configurar y Dibujar la Máscara (Con escala independiente)
        glm::mat4 matrixModelMascara = matrixModelCuerpoA;
        // Cambia los valores de escala (1.5f) según el tamaño que necesites
        matrixModelMascara = glm::scale(matrixModelMascara, glm::vec3(0.1f, 0.1f, 0.1f));
        // matrixModelMascara = glm::translate(matrixModelMascara, glm::vec3(0.0f, 0.0f, 0.05f)); // Descomenta si necesitas ajustarla hacia adelante
        matrixModelMascara = glm::translate(matrixModelMascara, glm::vec3(0.2f, 3.0f, 0.8f));

        matrixModelMascara = glm::rotate(matrixModelMascara, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = matrixModelMascara;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        mascaraA.Draw(lightingShader);

        // 3. Dibujar Brazo Derecho
        glm::mat4 matrixModelBrazoDerA = RotarEnPivote(matrixModelCuerpoA, pivoteHombroDer, rotacionBrazos);
        model = matrixModelBrazoDerA;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        biceps.Draw(lightingShader);
        manoDer.Draw(lightingShader);

        // 4. Dibujar Brazo Izquierdo
        glm::mat4 matrixModelBrazoIzqA = RotarEnPivote(matrixModelCuerpoA, pivoteHombroIzq, -rotacionBrazos);
        model = glm::translate(matrixModelBrazoIzqA, glm::vec3(0.06f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        biceps.Draw(lightingShader);

        model = matrixModelBrazoIzqA;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        manoIzq.Draw(lightingShader);

        // 5. Dibujar Pierna Derecha
        glm::mat4 matrixModelPiernaDerA = RotarEnPivote(matrixModelCuerpoA, pivoteCaderaDer, rotacionPiernas);
        model = matrixModelPiernaDerA;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        musDer.Draw(lightingShader);

        model = glm::scale(matrixModelPiernaDerA, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pieiz.Draw(lightingShader);

        // 6. Dibujar Pierna Izquierda
        glm::mat4 matrixModelPiernaIzqA = RotarEnPivote(matrixModelCuerpoA, pivoteCaderaIzq, -rotacionPiernas);
        model = glm::scale(matrixModelPiernaIzqA, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        musDer.Draw(lightingShader);

        model = matrixModelPiernaIzqA;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pieiz.Draw(lightingShader);

        //numero 2------------------
        glm::mat4 matrixModelCuerpoA2 = glm::mat4(1.0f);
        // Traslación general: mueve TODO el modelo (ej. -3.0f hacia la izquierda en X)
        matrixModelCuerpoA2 = glm::translate(matrixModelCuerpoA2, glm::vec3(2.5f, 0.75f, -9.2f));

        // 1. Dibujar Cuerpo y Cabeza (Tamaño y posición normal)
        model = matrixModelCuerpoA2;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        cuerpo.Draw(lightingShader);
        cabeza.Draw(lightingShader);

        // 2. Configurar y Dibujar la Máscara (Con escala independiente)
        glm::mat4 matrixModelMascara2 = matrixModelCuerpoA2;
        // Cambia los valores de escala (1.5f) según el tamaño que necesites
        matrixModelMascara2 = glm::scale(matrixModelMascara2, glm::vec3(0.1f, 0.1f, 0.1f));
        // matrixModelMascara = glm::translate(matrixModelMascara, glm::vec3(0.0f, 0.0f, 0.05f)); // Descomenta si necesitas ajustarla hacia adelante
        matrixModelMascara2 = glm::translate(matrixModelMascara2, glm::vec3(0.2f, 3.0f, 0.8f));

        matrixModelMascara2 = glm::rotate(matrixModelMascara2, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = matrixModelMascara2;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        mascaraA.Draw(lightingShader);

        // 3. Dibujar Brazo Derecho
        glm::mat4 matrixModelBrazoDerA2 = RotarEnPivote(matrixModelCuerpoA2, pivoteHombroDer, rotacionBrazos);
        model = matrixModelBrazoDerA2;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        biceps.Draw(lightingShader);
        manoDer.Draw(lightingShader);

        // 4. Dibujar Brazo Izquierdo
        glm::mat4 matrixModelBrazoIzqA2 = RotarEnPivote(matrixModelCuerpoA2, pivoteHombroIzq, -rotacionBrazos);
        model = glm::translate(matrixModelBrazoIzqA2, glm::vec3(0.06f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        biceps.Draw(lightingShader);

        model = matrixModelBrazoIzqA2;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        manoIzq.Draw(lightingShader);

        // 5. Dibujar Pierna Derecha
        glm::mat4 matrixModelPiernaDerA2 = RotarEnPivote(matrixModelCuerpoA2, pivoteCaderaDer, rotacionPiernas);
        model = matrixModelPiernaDerA2;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        musDer.Draw(lightingShader);

        model = glm::scale(matrixModelPiernaDerA2, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pieiz.Draw(lightingShader);

        // 6. Dibujar Pierna Izquierda
        glm::mat4 matrixModelPiernaIzqA2 = RotarEnPivote(matrixModelCuerpoA2, pivoteCaderaIzq, -rotacionPiernas);
        model = glm::scale(matrixModelPiernaIzqA2, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        musDer.Draw(lightingShader);

        model = matrixModelPiernaIzqA2;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pieiz.Draw(lightingShader);
       
        // ===================== LÁMPARA =====================
        lampShader.Use();
        modelLoc = glGetUniformLocation(lampShader.Program, "model");
        viewLoc = glGetUniformLocation(lampShader.Program, "view");
        projLoc = glGetUniformLocation(lampShader.Program, "projection");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // ===================== COPA =====================
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.0f, -0.09f, 2.3f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        copa.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0f, -0.09f, 2.3f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        copa.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.7f, -0.09f, -7.0f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        copa.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.7f, -0.09f, -7.0f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        copa.Draw(lightingShader);

        // ===================== DIBUJO OFRENDA =====================
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.0f, 0.0f, -4.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        ofrenda.Draw(lightingShader);

        // ===================== DIBUJAR SKYBOX =====================
        glDepthFunc(GL_LEQUAL);
        skyboxshader.Use();

        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));

        glUniformMatrix4fv(glGetUniformLocation(skyboxshader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(skyboxshader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1i(glGetUniformLocation(skyboxshader.Program, "skybox"), 0);

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubmapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        glDepthFunc(GL_LESS);

        // ===================== PAPEL PICADO =====================
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        papelShader.Use();

        glm::mat4 viewPapel = camera.GetViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(papelShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(viewPapel));
        glUniformMatrix4fv(glGetUniformLocation(papelShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glUniform1i(glGetUniformLocation(papelShader.Program, "papelTex"), 0);
        glUniform1f(glGetUniformLocation(papelShader.Program, "opacidad"), 1.0f);

        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(quadVAO);

        GLuint texturas[] = { papelTexture1, papelTexture2, papelTexture3 };
        float posicionesZ[] = { 2.1f, -2.2f, -6.2f, -10.2f };

        for (int arco = 0; arco < 4; arco++) {
            for (int i = 0; i < 3; i++) {
                glBindTexture(GL_TEXTURE_2D, texturas[(arco + i) % 3]);

                float posX = -0.4f + (i * 0.4f);

                float tiempoViento = currentFrame * 0.8f;
                float fuerzaRafaga = pow(sin(tiempoViento), 3.0f);
                if (fuerzaRafaga < 0.0f) fuerzaRafaga = 0.0f;

                float oscilacionZ = fuerzaRafaga * 30.0f;
                float oscilacionX = fuerzaRafaga * 45.0f;

                float variacionZ = sin(currentFrame * 1.5f + (arco * 1.5f) + (i * 0.8f)) * 5.0f;
                float variacionX = sin(currentFrame * 1.2f + (arco * 2.0f) + (i * 1.2f)) * 5.0f;

                float anguloFinalZ = oscilacionZ + variacionZ;
                float anguloFinalX = oscilacionX + variacionX;

                float posY = 0.9f;
                if (i == 1) posY = 0.8f;

                glm::mat4 modelPapel = glm::mat4(1.0f);
                modelPapel = glm::translate(modelPapel, glm::vec3(posX, posY, posicionesZ[arco]));
                modelPapel = glm::rotate(modelPapel, glm::radians(anguloFinalZ), glm::vec3(0.0f, 0.0f, 1.0f));
                modelPapel = glm::rotate(modelPapel, glm::radians(anguloFinalX), glm::vec3(1.0f, 0.0f, 0.0f));
                modelPapel = glm::scale(modelPapel, glm::vec3(0.4f, 0.4f, 0.6f));

                glUniformMatrix4fv(glGetUniformLocation(papelShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelPapel));
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }

        // ===================== HUMO DEL COPAL =====================
        glBindTexture(GL_TEXTURE_2D, humoTexture);

        glm::vec3 copasPosiciones[] = {
            glm::vec3(1.0f, 0.0f, 2.3f),
            glm::vec3(-1.0f, 0.0f, 2.3f),
            glm::vec3(0.7f, 0.0f, -7.0f),
            glm::vec3(-0.7f, 0.0f, -7.0f)
        };

        for (int c = 0; c < 4; c++) {
            for (int h = 0; h < 3; h++) {
                float tiempoHumo = fmod(currentFrame * 0.3f + (h * 0.33f) + (c * 0.15f), 1.0f);

                float posY = tiempoHumo * 2.5f;
                float escalaY = 1.0f + (tiempoHumo * 1.5f);
                float escalaX = 0.5f + (tiempoHumo * 0.5f);

                // Se desvanece al llegar arriba
                float opacidadHumo = 1.0f - tiempoHumo;
                glUniform1f(glGetUniformLocation(papelShader.Program, "opacidad"), opacidadHumo);

                float vientoX = sin(currentFrame * 1.5f + h + c) * 0.2f;

                glm::mat4 modelHumo = glm::mat4(1.0f);
                modelHumo = glm::translate(modelHumo, glm::vec3(copasPosiciones[c].x + vientoX, posY, copasPosiciones[c].z));
                modelHumo = glm::rotate(modelHumo, glm::radians(tiempoHumo * 15.0f), glm::vec3(0.0f, 0.0f, 1.0f));

                // Escalamos
                modelHumo = glm::scale(modelHumo, glm::vec3(escalaX, escalaY, 1.0f));

                glUniformMatrix4fv(glGetUniformLocation(papelShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelHumo));
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }

        glBindVertexArray(0);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);


        // Swap buffers
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteBuffers(1, &quadEBO);
    glfwTerminate();
    return 0;
}

// Movimiento de camara y luces
void DoMovement()
{
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (keys[GLFW_KEY_T]) pointLightPositions[0].x += 0.01f;
    if (keys[GLFW_KEY_G]) pointLightPositions[0].x -= 0.01f;
    if (keys[GLFW_KEY_Y]) pointLightPositions[0].y += 0.01f;
    if (keys[GLFW_KEY_H]) pointLightPositions[0].y -= 0.01f;
    if (keys[GLFW_KEY_U]) pointLightPositions[0].z -= 0.01f;
    if (keys[GLFW_KEY_J]) pointLightPositions[0].z += 0.01f;
}

// Key callback
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }

    // M: enciende / apaga el baile de la mujer
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        animandoBaile = !animandoBaile;
    }

    // Z / X: ajusta la altura del pivote de la cintura (calibracion)
    if (action == GLFW_PRESS && (key == GLFW_KEY_Z || key == GLFW_KEY_X)) {
        pivoteCintura.y += (key == GLFW_KEY_Z ? 0.05f : -0.05f);
        std::cout << "pivoteCintura.y = " << pivoteCintura.y << std::endl;
    }
}

// Mouse callback
void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse)
    {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }

    GLfloat xOffset = xPos - lastX;
    GLfloat yOffset = lastY - yPos;
    lastX = xPos;
    lastY = yPos;

    camera.ProcessMouseMovement(xOffset, yOffset);
}

// Animación de la mujer controlada por máquina de estados
// - anguloBaile: pelvis; torso, falda y piernas heredan este giro en Z
// - anguloFalda: balanceo extra de la falda (relativo a la pelvis) para el "vuelo" del vestido
// - rotacionPiernasMujer: paso leve en X
void Animation() {
    if (!animandoBaile) {
        estadoBaile = 0;
    }

    bool llegoAng = false, llegoFalda = false, llegoPie = false;

    switch (estadoBaile) {
    case 0: // REPOSO: regresa suavemente todo a cero
        MoverHacia(anguloBaile, 0.0f, VEL_BAILE);
        MoverHacia(anguloFalda, 0.0f, VEL_FALDA);
        MoverHacia(rotacionPiernasMujer, 0.0f, VEL_PIERNA);
        if (animandoBaile) estadoBaile = 1;
        break;

    case 1: // INCLINACIÓN HACIA UN LADO (+)
        llegoAng = MoverHacia(anguloBaile, MAX_BAILE, VEL_BAILE);
        llegoFalda = MoverHacia(anguloFalda, MAX_FALDA, VEL_FALDA);
        llegoPie = MoverHacia(rotacionPiernasMujer, MAX_PIERNA, VEL_PIERNA);
        if (llegoAng && llegoFalda && llegoPie) estadoBaile = 2;
        break;

    case 2: // REGRESA AL CENTRO
        llegoAng = MoverHacia(anguloBaile, 0.0f, VEL_BAILE);
        llegoFalda = MoverHacia(anguloFalda, 0.0f, VEL_FALDA);
        llegoPie = MoverHacia(rotacionPiernasMujer, 0.0f, VEL_PIERNA);
        if (llegoAng && llegoFalda && llegoPie) estadoBaile = 3;
        break;

    case 3: // INCLINACIÓN HACIA EL OTRO LADO (-)
        llegoAng = MoverHacia(anguloBaile, -MAX_BAILE, VEL_BAILE);
        llegoFalda = MoverHacia(anguloFalda, -MAX_FALDA, VEL_FALDA);
        llegoPie = MoverHacia(rotacionPiernasMujer, -MAX_PIERNA, VEL_PIERNA);
        if (llegoAng && llegoFalda && llegoPie) estadoBaile = 4;
        break;

    case 4: // REGRESA AL CENTRO
        llegoAng = MoverHacia(anguloBaile, 0.0f, VEL_BAILE);
        llegoFalda = MoverHacia(anguloFalda, 0.0f, VEL_FALDA);
        llegoPie = MoverHacia(rotacionPiernasMujer, 0.0f, VEL_PIERNA);
        if (llegoAng && llegoFalda && llegoPie) estadoBaile = 1;
        break;
    }

}
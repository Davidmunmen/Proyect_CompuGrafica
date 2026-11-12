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
void ActualizarTourCamara();
void LimitarPosicionCamara();

// Window dimensions
const GLuint WIDTH = 1200, HEIGHT = 1000;
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
    // Velas extra cerca de la ofrenda
        glm::vec3(0.7f, -0.09f, -10.0f),
        glm::vec3(-0.7f, -0.09f, -10.0f),

        //Tumba1
        glm::vec3(0.9f, -0.09f, -1.2f),
        glm::vec3(-0.9f, -0.09f, -12.0f),

        //Tumba2
       glm::vec3(0.9f, -0.09f, -1.2f),
       glm::vec3(-0.9f, -0.09f, -14.0f),

       //Tumba3
       glm::vec3(0.9f, -0.09f, -1.2f),
       glm::vec3(-0.9f, -0.09f, -16.0f),
};
const int NUM_VELAS_TOTAL = 16;
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

// Variables para los Keyframes del fuego
#define MAX_FUEGO_FRAMES 3
int fuegoPlayIndex = 0;
float fuegoY = -0.09f;
float fuegoEscala = 1.0f;
float fuegoYInc = 0.0f;
float fuegoEscalaInc = 0.0f;
int i_fuego_curr_steps = 0;
int i_fuego_max_steps = 40;

struct FuegoFrame {
    float y;
    float escala;
    float yInc;
    float escalaInc;
} FuegoKeyFrames[MAX_FUEGO_FRAMES];
// Variables de animación jerárquica para las extremidades del personaje
GLfloat rotacionBrazos = 0.0f;
GLfloat rotacionPiernas = 0.0f;
// Variables para la animación de baile de la mujer
float anguloBaile = 0.0f;
float anguloFalda = 0.0f;
float rotacionPiernasMujer = 0.0f;
bool animandoBaile = false;

// Estados de la máquina de estados para el baile
int estadoBaile = 0;
const float MAX_BAILE = 2.0f;
const float MAX_FALDA = 1.5f;
const float MAX_PIERNA = 2.0f;
const float VEL_BAILE = 6.0f;
const float VEL_FALDA = 4.5f;
const float VEL_PIERNA = 6.0f;

const glm::vec3 pivoteHombroDer(-0.0f, 0.0f, 0.0f);
const glm::vec3 pivoteHombroIzq(0.0f, 0.0f, 0.0f);
const glm::vec3 pivoteCaderaDer(-0.0f, 0.0f, 0.0f);
const glm::vec3 pivoteCaderaIzq(0.0f, 0.0f, 0.0f);
glm::vec3 pivoteCintura(0.0f, 2.4f, 0.0f);
// ===================== VARIABLES PARA EL TOUR DE CÁMARA =====================
bool tourActivo = false;      //iniciamos en false para que el usuario decida si quiere iniciar el tour
bool tourPausado = false;     // Si el usuario presionó X para pausar
int estacionActual = 0;       // 0: Camino/Arco, 1: Hombre y Mujer, 2: Ofrenda, 3: Abuelitos, 4: Cementerio
float progresoEstacion = 0.0f;
bool esperandoAccion = false; // Si llegó a un punto de tarjeta y espera que el usuario presione X para continuar

//Variables para la animacion de la camapana
bool animandoCampana = false;
int estadoCampana = 0;
float anguloCampana = 0.0f;
float maxAnguloCampana = 25.0f;
float velCampanaActual = 90.0f;
const float VEL_CAMPANA_BASE = 90.0f;

// Variables para el baile de los abuelitos
bool animandoAbuelitos = false;
int estadoBaileAbuelitos = 0;

float rotacionBrazosAbuelitos = 0.0f;
float rotacionPiernasAbuelitos = 0.0f;
float inclinacionAbuelitos = 0.0f;

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
	Shader TemplateShader("Shader/Temp.vs", "Shader/Temp.frag"); //shader para las tarjetas informativas


    // Modelos:  piso y arco
    Model Piso((char*)"Models/Arco/piso.obj");
    Model base((char*)"Models/Arco/base.obj");
    Model PisoA((char*)"Models/Arco/pisoA.obj");
    Model velas((char*)"Models/velas/velas.obj");
    Model planta((char*)"Models/Planta/planta.obj");
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
    Model piederecho((char*)"Models/personaje/pieizq/PieIzq.obj"); 
    Model fuego((char*)"Models/Fuego/Fuego.obj");
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
   
    // Cargar texturas informativas para cada estación del tour
    GLuint tarjetaArco = SOIL_load_OGL_texture("Models/tarjetas/arco.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
    GLuint tarjetaPersonajes = SOIL_load_OGL_texture("Models/tarjetas/personajes.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
    GLuint tarjetaOfrenda = SOIL_load_OGL_texture("Models/tarjetas/ofrenda.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
    GLuint tarjetaAbuelitos = SOIL_load_OGL_texture("Models/tarjetas/abuelitos.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
    GLuint tarjetaCementerio = SOIL_load_OGL_texture("Models/tarjetas/cementerio.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
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


    GLuint indices[] =
    {  // Note that we start from 0!
        0,1,2,3,
        4,5,6,7,
        8,9,10,11,
        12,13,14,15,
        16,17,18,19,
        20,21,22,23,
        24,25,26,27,
        28,29,30,31,
        32,33,34,35
    };

    // ===================== CONFIGURACIÓN DEL SKYBOX (VAO, VBO y TEXTURAS) =====================
    // 
    //skybox
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

    // Configurar el plano del papel picado
    GLuint quadVAO, quadVBO, quadEBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glGenBuffers(1, &quadEBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    // Posición
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    // Coordenada de textura
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        // Cargar la textura 1 del papel picado
    GLuint papelTexture0= SOIL_load_OGL_texture(
		"Models/tarjetas/abuelitos.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );
    // Cargar la textura 1 del papel picado
    GLuint papelTexture1 = SOIL_load_OGL_texture(
        "Models/papel.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );

    // Cargar la textura 2 del papel picado
    GLuint papelTexture2 = SOIL_load_OGL_texture(
        "Models/papel2.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );

    // Cargar la textura 3 del papel picado
    GLuint papelTexture3 = SOIL_load_OGL_texture(
        "Models/papel3.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );

    GLuint humoTexture = SOIL_load_OGL_texture(
        "Models/humo.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Delta time
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // ===================== ANIMACIÓN KEYFRAMES DEL FUEGO =====================
// Inicializamos los frames la primera vez
        if (fuegoPlayIndex == 0 && i_fuego_curr_steps == 0 && fuegoYInc == 0.0f) {
            FuegoKeyFrames[0].y = -0.09f;   FuegoKeyFrames[0].escala = 1.0f;
            FuegoKeyFrames[1].y = -0.05f;   FuegoKeyFrames[1].escala = 1.3f;
            FuegoKeyFrames[2].y = -0.12f;   FuegoKeyFrames[2].escala = 0.8f;

            // Calculamos el primer incremento
            FuegoKeyFrames[0].yInc = (FuegoKeyFrames[1].y - FuegoKeyFrames[0].y) / i_fuego_max_steps;
            FuegoKeyFrames[0].escalaInc = (FuegoKeyFrames[1].escala - FuegoKeyFrames[0].escala) / i_fuego_max_steps;
        }

        if (i_fuego_curr_steps >= i_fuego_max_steps) {
            // Pasamos al siguiente frame
            fuegoPlayIndex++;
            if (fuegoPlayIndex >= MAX_FUEGO_FRAMES) {
                fuegoPlayIndex = 0; // Volvemos al inicio (bucle infinito)
            }

            // Calculamos los incrementos para el nuevo frame
            int siguienteFrame = (fuegoPlayIndex + 1) % MAX_FUEGO_FRAMES;
            FuegoKeyFrames[fuegoPlayIndex].yInc = (FuegoKeyFrames[siguienteFrame].y - FuegoKeyFrames[fuegoPlayIndex].y) / i_fuego_max_steps;
            FuegoKeyFrames[fuegoPlayIndex].escalaInc = (FuegoKeyFrames[siguienteFrame].escala - FuegoKeyFrames[fuegoPlayIndex].escala) / i_fuego_max_steps;

            i_fuego_curr_steps = 0;
        }
        else {
            // Aplicamos los incrementos actuales
            fuegoY += FuegoKeyFrames[fuegoPlayIndex].yInc;
            fuegoEscala += FuegoKeyFrames[fuegoPlayIndex].escalaInc;
            i_fuego_curr_steps++;
        }
        // Events
        glfwPollEvents();
        DoMovement();
        Animation();
        ActualizarTourCamara();
		LimitarPosicionCamara();

        // Clear
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // OpenGL options
        glEnable(GL_DEPTH_TEST);

        // ===================== SHADER DE ILUMINACIÓN =====================
        lightingShader.Use();

        glUniform1i(glGetUniformLocation(lightingShader.Program, "diffuse"), 0);

        GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
        glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

        // Directional light (LUZ GLOBAL MÁS BAJA)
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.12f, 0.12f, 0.15f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.15f, 0.15f, 0.15f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.05f, 0.05f, 0.05f);

        // ===== ASIGNACIÓN LUCES=====
        glm::vec3 velaColor(1.0f, 0.6f, 0.2f);
        // ===== ASIGNACIÓN DINÁMICA DE LUCES =====

        // 1. Calculamos las distancias
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

            // --- AMBIENTE
            glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "ambient").c_str()),
                velaColor.x * 0.05f, velaColor.y * 0.05f, velaColor.z * 0.05f);

            // --- DIFFUSE 
            glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "diffuse").c_str()),
                velaColor.x * flicker * fade * 0.6f, velaColor.y * flicker * fade * 0.6f, velaColor.z * flicker * fade * 0.6f);

            // --- SPECULAR APAGADO
            glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "specular").c_str()),
                0.0f, 0.0f, 0.0f);

            glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "constant").c_str()), 1.0f);

            //Cambio para aumentar el alcance de las luces 
            glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "linear").c_str()), 0.14f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "quadratic").c_str()), 0.07f);
        }
        //=====================LUZ VELAS=====================
        // SpotLight
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"),
            camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"),
            camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
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

        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.ambient"), 1.0f, 1.0f, 1.0f);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.0f, -0.0f, -2.2f));
        model = glm::scale(model, glm::vec3(0.4f, 0.2f, 2.1f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        Piso.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, -0.0f, -2.2f));
        model = glm::scale(model, glm::vec3(0.4f, 0.2f, 2.1f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        Piso.Draw(lightingShader);


        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.0f, 0.0f, -12.4f));
        model = glm::scale(model, glm::vec3(0.4f, 0.2f, 2.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        Piso.Draw(lightingShader);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, -12.4f));
        model = glm::scale(model, glm::vec3(0.4f, 0.2f, 2.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        Piso.Draw(lightingShader);


        

       

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.1f, -16.2f));
        model = glm::scale(model, glm::vec3(0.8f, 0.2f, 0.7f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        PisoA.Draw(lightingShader);


        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.1f, -12.7f));
        model = glm::scale(model, glm::vec3(0.8f, 0.2f, 0.7f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        PisoA.Draw(lightingShader);


        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.1f, -9.2f));
        model = glm::scale(model, glm::vec3(0.8f, 0.2f, 0.7f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        PisoA.Draw(lightingShader);


        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.1f, -5.7f));
        model = glm::scale(model, glm::vec3(0.85f, 0.2f, 0.7f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        PisoA.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.1f, -2.2f));
        model = glm::scale(model, glm::vec3(0.85f, 0.2f, 0.7f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        PisoA.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.1f, 1.3f));
        model = glm::scale(model, glm::vec3(0.85f, 0.2f, 0.7f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        PisoA.Draw(lightingShader);

        // Pintamos el material del piso de un tono más claro para que reciba la luz
        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.ambient"), 0.8f, 0.8f, 0.8f);
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
        model = glm::translate(model, glm::vec3(0.0f, -0.1f, -11.5f));
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
        glm::vec3 pivoteCampana(0.0f, 1.51f, -11.0f); // Punto de suspensión superior

        glm::mat4 matrixCampana = glm::mat4(1.0f);
        matrixCampana = glm::translate(matrixCampana, glm::vec3(0.0f, 1.51f, -11.0f));

        // Aplica la rotación pendular en Z alrededor del pivote superior
        matrixCampana = RotarEnPivote(matrixCampana, pivoteCampana, anguloCampana, glm::vec3(0.0f, 0.0f, 1.0f));
        matrixCampana = glm::scale(matrixCampana, glm::vec3(0.3f, 0.3f, 0.3f));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(matrixCampana));
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
// Planta 1
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-2.4f, 0.0f, 1.5f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    planta.Draw(lightingShader);

    // Planta 2
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-2.8f, 0.0f, 0.5f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    planta.Draw(lightingShader);

    // Planta 3
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-2.4f, 0.0f, -1.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.45f, 0.45f, 0.45f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    planta.Draw(lightingShader);

    // Planta 4
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-2.8f, 0.0f, -2.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.55f, 0.55f, 0.55f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    planta.Draw(lightingShader);

    // Planta 5
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.4f, 0.0f, 1.5f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    planta.Draw(lightingShader);

    // Planta 6
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.8f, 0.0f, 0.5f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    planta.Draw(lightingShader);

    // Planta 7
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.4f, 0.0f, -1.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.45f, 0.45f, 0.45f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    planta.Draw(lightingShader);

    // Planta 8
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.8f, 0.0f, -2.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.55f, 0.55f, 0.55f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    planta.Draw(lightingShader);

    // Planta 9
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(1.5f, -0.2f, -17.3f));
    //model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.55f, 0.55f, 0.55f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    planta.Draw(lightingShader);

    // Planta 10
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.5f, -0.2f, -17.3f));
    //model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.55f, 0.55f, 0.55f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    planta.Draw(lightingShader);

    // Planta 11
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.5f, -0.2f, -17.3f));
    //model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.55f, 0.55f, 0.55f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    planta.Draw(lightingShader);

    // Planta 12
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-0.5f, -0.2f, -17.3f));
    //model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.55f, 0.55f, 0.55f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    planta.Draw(lightingShader);

    // Planta 12
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-2.5f, -0.2f, -17.3f));
    //model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.45f, 0.45f, 0.45f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    planta.Draw(lightingShader);

    // Planta 12
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.5f, -0.2f, -17.3f));
    //model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.45f, 0.45f, 0.45f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    planta.Draw(lightingShader);

    // Planta 7
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.4f, 0.0f, -5.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.45f, 0.45f, 0.45f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    planta.Draw(lightingShader);

    // Planta 8
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.8f, 0.0f, -4.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.55f, 0.55f, 0.55f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    planta.Draw(lightingShader);

    // Planta 7
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(1.5f, 0.0f, -3.5f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.45f, 0.45f, 0.45f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    planta.Draw(lightingShader);

    // Planta 8
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(1.8f, 0.0f, -2.6f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.55f, 0.55f, 0.55f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    planta.Draw(lightingShader);

    // Planta 7
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(1.5f, 0.0f, -7.4f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.45f, 0.45f, 0.45f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    planta.Draw(lightingShader);

    // Planta 8
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.5f, 0.0f, -6.2f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.55f, 0.55f, 0.55f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    planta.Draw(lightingShader);

    // Planta 7
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.8f, 0.0f, -8.2f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.45f, 0.45f, 0.45f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    planta.Draw(lightingShader);

    // Planta 8
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-2.8f, 0.0f, -6.0));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.55f, 0.55f, 0.55f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    planta.Draw(lightingShader);


    // Plantaa Atras Izquierda
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-2.8f, 0.0f, -8.2f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.45f, 0.45f, 0.45f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    planta.Draw(lightingShader);

    // Planta Atras Izquierda
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-2.2f, 0.0f, -7.3));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.55f, 0.55f, 0.55f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    planta.Draw(lightingShader);

        // ===================== Velas y Fuego Animado =====================
        lightingShader.Use();

        // Arreglo con las posiciones de tus 6 bases de velas
        glm::vec3 velasPos[] = {
            glm::vec3(0.7f, -0.09f, 2.3f),
            glm::vec3(-0.7f, -0.09f, 2.3f),
            glm::vec3(-0.7f, -0.09f, -2.0f),
            glm::vec3(0.7f, -0.09f, -2.0f),
            glm::vec3(-0.7f, -0.09f, -6.0f),
            glm::vec3(0.7f, -0.09f, -6.0f),
            glm::vec3(0.7f, -0.09f, -11.0f),
            glm::vec3(-0.7f, -0.09f, -11.0f),
        };
       

        // Posiciones relativas de las 5 velitas (X, Y, Z)
        glm::vec3 posicionesVelitas[] = {
            glm::vec3(0.05f,  0.15f, -0.15f),   // Velita 1: Trasera Central
            glm::vec3(0.5f,   0.13f, -0.15f),   // Velita 2: Trasera Derecha
            glm::vec3(-0.4f,  0.06f, -0.15f),   // Velita 3: Trasera Izquierda
            glm::vec3(0.4f,   0.04f, 0.30f),    // Velita 4: Frontal Izquierda
            glm::vec3(-0.07f, 0.01f, 0.32f)     // Velita 5: Frontal Derecha 
        };

        // Bucle para las 8 bases de velas
        for (int v = 0; v < 8; v++) {
            // Dibujar la base de la vela
            model = glm::mat4(1.0f);
            model = glm::translate(model, velasPos[v]);
            model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            velas.Draw(lightingShader);

        //model = glm::mat4(1.0f);
        //model = glm::translate(model, glm::vec3(-0.7f, -0.09f, -10.0f));
        //model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        //velas.Draw(lightingShader);

        //model = glm::mat4(1.0f);
        //model = glm::translate(model, glm::vec3(0.7f, -0.09f, -10.0f));
        //model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        //velas.Draw(lightingShader);
        
            // Bucle para las 5 velitas de cada base
            for (int i = 0; i < 5; i++) {
                model = glm::mat4(1.0f);

                // Posición X y Z
                float posX = velasPos[v].x + (posicionesVelitas[i].x * 0.2f);
                float posZ = velasPos[v].z + (posicionesVelitas[i].z * 0.2f);

                // ALTURA INDIVIDUAL + ANIMACIÓN
                float alturaBase = posicionesVelitas[i].y;
                float alturaFuego = velasPos[v].y + alturaBase + 0.2f + fuegoY;

                model = glm::translate(model, glm::vec3(posX, alturaFuego, posZ));

                float escalaFinalFuego = 0.04f * fuegoEscala;
                model = glm::scale(model, glm::vec3(escalaFinalFuego, escalaFinalFuego, escalaFinalFuego));

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                fuego.Draw(lightingShader);
            }
        }
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


        // ================================================================
        // ABUELITO 1
        // ================================================================

        glm::mat4 matrixModelCuerpoA = glm::mat4(1.0f);
        matrixModelCuerpoA = glm::translate(
            matrixModelCuerpoA, glm::vec3(2.0f, 0.75f, -9.2f));

        // Inclinación general del modelo
        matrixModelCuerpoA = RotarEnPivote(
            matrixModelCuerpoA,
            pivoteCintura,
            inclinacionAbuelitos,
            glm::vec3(0.0f, 0.0f, 1.0f)
        );

        // Cuerpo y cabeza
        model = matrixModelCuerpoA;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        cuerpo.Draw(lightingShader);
        cabeza.Draw(lightingShader);

        // Máscara
        glm::mat4 matrixModelMascara = matrixModelCuerpoA;
        matrixModelMascara = glm::scale(matrixModelMascara, glm::vec3(0.1f, 0.1f, 0.1f));
        matrixModelMascara = glm::translate(matrixModelMascara, glm::vec3(0.2f, 3.0f, 0.8f));
        matrixModelMascara = glm::rotate(matrixModelMascara, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        model = matrixModelMascara;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        mascaraA.Draw(lightingShader);

        // Brazo derecho
        glm::mat4 matrixModelBrazoDerA =
            RotarEnPivote(matrixModelCuerpoA, pivoteHombroDer, rotacionBrazosAbuelitos);

        model = matrixModelBrazoDerA;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        biceps.Draw(lightingShader);
        manoDer.Draw(lightingShader);

        // Brazo izquierdo
        glm::mat4 matrixModelBrazoIzqA =
            RotarEnPivote(matrixModelCuerpoA, pivoteHombroIzq, -rotacionBrazosAbuelitos);

        model = glm::translate(matrixModelBrazoIzqA, glm::vec3(0.06f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        biceps.Draw(lightingShader);

        model = matrixModelBrazoIzqA;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        manoIzq.Draw(lightingShader);

        // Pierna derecha
        glm::mat4 matrixModelPiernaDerA =
            RotarEnPivote(matrixModelCuerpoA, pivoteCaderaDer, rotacionPiernasAbuelitos);

        model = matrixModelPiernaDerA;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        musDer.Draw(lightingShader);

        model = glm::scale(matrixModelPiernaDerA, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pieiz.Draw(lightingShader);

        // Pierna izquierda
        glm::mat4 matrixModelPiernaIzqA =
            RotarEnPivote(matrixModelCuerpoA, pivoteCaderaIzq, -rotacionPiernasAbuelitos);

        model = glm::scale(matrixModelPiernaIzqA, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        musDer.Draw(lightingShader);

        model = matrixModelPiernaIzqA;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pieiz.Draw(lightingShader);


        // ================================================================
        // ABUELITO 2
        // ================================================================

        glm::mat4 matrixModelCuerpoA2 = glm::mat4(1.0f);
        matrixModelCuerpoA2 = glm::translate(
            matrixModelCuerpoA2, glm::vec3(2.5f, 0.75f, -9.2f));

        matrixModelCuerpoA2 = RotarEnPivote(
            matrixModelCuerpoA2,
            pivoteCintura,
            inclinacionAbuelitos,
            glm::vec3(0.0f, 0.0f, 1.0f)
        );

        // Cuerpo y cabeza
        model = matrixModelCuerpoA2;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        cuerpo.Draw(lightingShader);
        cabeza.Draw(lightingShader);

        // Máscara
        glm::mat4 matrixModelMascara2 = matrixModelCuerpoA2;
        matrixModelMascara2 = glm::scale(matrixModelMascara2, glm::vec3(0.1f, 0.1f, 0.1f));
        matrixModelMascara2 = glm::translate(matrixModelMascara2, glm::vec3(0.2f, 3.0f, 0.8f));
        matrixModelMascara2 = glm::rotate(matrixModelMascara2, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        model = matrixModelMascara2;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        mascaraA.Draw(lightingShader);

        // Brazo derecho
        glm::mat4 matrixModelBrazoDerA2 =
            RotarEnPivote(matrixModelCuerpoA2, pivoteHombroDer, rotacionBrazosAbuelitos);

        model = matrixModelBrazoDerA2;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        biceps.Draw(lightingShader);
        manoDer.Draw(lightingShader);

        // Brazo izquierdo
        glm::mat4 matrixModelBrazoIzqA2 =
            RotarEnPivote(matrixModelCuerpoA2, pivoteHombroIzq, -rotacionBrazosAbuelitos);

        model = glm::translate(matrixModelBrazoIzqA2, glm::vec3(0.06f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        biceps.Draw(lightingShader);

        model = matrixModelBrazoIzqA2;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        manoIzq.Draw(lightingShader);

        // Pierna derecha
        glm::mat4 matrixModelPiernaDerA2 =
            RotarEnPivote(matrixModelCuerpoA2, pivoteCaderaDer, rotacionPiernasAbuelitos);

        model = matrixModelPiernaDerA2;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        musDer.Draw(lightingShader);

        model = glm::scale(matrixModelPiernaDerA2, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pieiz.Draw(lightingShader);

        // Pierna izquierda
        glm::mat4 matrixModelPiernaIzqA2 =
            RotarEnPivote(matrixModelCuerpoA2, pivoteCaderaIzq, -rotacionPiernasAbuelitos);

        model = glm::scale(matrixModelPiernaIzqA2, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        musDer.Draw(lightingShader);

        model = matrixModelPiernaIzqA2;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pieiz.Draw(lightingShader);


        // ================================================================
        // ABUELITO 3
        // ================================================================

        glm::mat4 matrixModelCuerpoA3 = glm::mat4(1.0f);
        matrixModelCuerpoA3 = glm::translate(
            matrixModelCuerpoA3, glm::vec3(-2.5f, 0.75f, -9.2f));

        matrixModelCuerpoA3 = RotarEnPivote(
            matrixModelCuerpoA3,
            pivoteCintura,
            inclinacionAbuelitos,
            glm::vec3(0.0f, 0.0f, 1.0f)
        );

        // Cuerpo y cabeza
        model = matrixModelCuerpoA3;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        cuerpo.Draw(lightingShader);
        cabeza.Draw(lightingShader);

        // Máscara
        glm::mat4 matrixModelMascara3 = matrixModelCuerpoA3;
        matrixModelMascara3 = glm::scale(matrixModelMascara3, glm::vec3(0.1f, 0.1f, 0.1f));
        matrixModelMascara3 = glm::translate(matrixModelMascara3, glm::vec3(0.2f, 3.0f, 0.8f));
        matrixModelMascara3 = glm::rotate(matrixModelMascara3, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        model = matrixModelMascara3;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        mascaraA.Draw(lightingShader);

        // Brazo derecho
        glm::mat4 matrixModelBrazoDerA3 =
            RotarEnPivote(matrixModelCuerpoA3, pivoteHombroDer, rotacionBrazosAbuelitos);

        model = matrixModelBrazoDerA3;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        biceps.Draw(lightingShader);
        manoDer.Draw(lightingShader);

        // Brazo izquierdo
        glm::mat4 matrixModelBrazoIzqA3 =
            RotarEnPivote(matrixModelCuerpoA3, pivoteHombroIzq, -rotacionBrazosAbuelitos);

        model = glm::translate(matrixModelBrazoIzqA3, glm::vec3(0.06f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        biceps.Draw(lightingShader);

        model = matrixModelBrazoIzqA3;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        manoIzq.Draw(lightingShader);

        // Pierna derecha
        glm::mat4 matrixModelPiernaDerA3 =
            RotarEnPivote(matrixModelCuerpoA3, pivoteCaderaDer, rotacionPiernasAbuelitos);

        model = matrixModelPiernaDerA3;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        musDer.Draw(lightingShader);

        model = glm::scale(matrixModelPiernaDerA3, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pieiz.Draw(lightingShader);

        // Pierna izquierda
        glm::mat4 matrixModelPiernaIzqA3 =
            RotarEnPivote(matrixModelCuerpoA3, pivoteCaderaIzq, -rotacionPiernasAbuelitos);

        model = glm::scale(matrixModelPiernaIzqA3, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        musDer.Draw(lightingShader);

        model = matrixModelPiernaIzqA3;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pieiz.Draw(lightingShader);


        // ================================================================
        // ABUELITO 4
        // ================================================================

        glm::mat4 matrixModelCuerpoA4 = glm::mat4(1.0f);
        matrixModelCuerpoA4 = glm::translate(
            matrixModelCuerpoA4, glm::vec3(-2.0f, 0.75f, -9.2f));

        matrixModelCuerpoA4 = RotarEnPivote(
            matrixModelCuerpoA4,
            pivoteCintura,
            inclinacionAbuelitos,
            glm::vec3(0.0f, 0.0f, 1.0f)
        );

        // Cuerpo y cabeza
        model = matrixModelCuerpoA4;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        cuerpo.Draw(lightingShader);
        cabeza.Draw(lightingShader);

        // Máscara
        glm::mat4 matrixModelMascara4 = matrixModelCuerpoA4;
        matrixModelMascara4 = glm::scale(matrixModelMascara4, glm::vec3(0.1f, 0.1f, 0.1f));
        matrixModelMascara4 = glm::translate(matrixModelMascara4, glm::vec3(0.2f, 3.0f, 0.8f));
        matrixModelMascara4 = glm::rotate(matrixModelMascara4, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        model = matrixModelMascara4;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        mascaraA.Draw(lightingShader);

        // Brazo derecho
        glm::mat4 matrixModelBrazoDerA4 =
            RotarEnPivote(matrixModelCuerpoA4, pivoteHombroDer, rotacionBrazosAbuelitos);

        model = matrixModelBrazoDerA4;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        biceps.Draw(lightingShader);
        manoDer.Draw(lightingShader);

        // Brazo izquierdo
        glm::mat4 matrixModelBrazoIzqA4 =
            RotarEnPivote(matrixModelCuerpoA4, pivoteHombroIzq, -rotacionBrazosAbuelitos);

        model = glm::translate(matrixModelBrazoIzqA4, glm::vec3(0.06f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        biceps.Draw(lightingShader);

        model = matrixModelBrazoIzqA4;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        manoIzq.Draw(lightingShader);

        // Pierna derecha
        glm::mat4 matrixModelPiernaDerA4 =
            RotarEnPivote(matrixModelCuerpoA4, pivoteCaderaDer, rotacionPiernasAbuelitos);

        model = matrixModelPiernaDerA4;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        musDer.Draw(lightingShader);

        model = glm::scale(matrixModelPiernaDerA4, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pieiz.Draw(lightingShader);

        // Pierna izquierda
        glm::mat4 matrixModelPiernaIzqA4 =
            RotarEnPivote(matrixModelCuerpoA4, pivoteCaderaIzq, -rotacionPiernasAbuelitos);

        model = glm::scale(matrixModelPiernaIzqA4, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        musDer.Draw(lightingShader);

        model = matrixModelPiernaIzqA4;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pieiz.Draw(lightingShader);



        // ===================== SEGUNDA MUJER (Lado Derecho) =====================

        // Usamos variables con un nombre distinto para no chocar con las de la mujer original
        glm::mat4 matrixModelMujer2 = glm::mat4(1.0f);
        // La posicionamos a la derecha (X = 1.3f) y giramos 180 grados
        matrixModelMujer2 = glm::translate(matrixModelMujer2, glm::vec3(1.3f, 0.6f, 0.0f));

        // 1. Torso
        glm::mat4 matrixTorsoMujer2 = RotarEnPivote(matrixModelMujer2, pivoteCintura, anguloBaile, glm::vec3(0.0f, 0.0f, 1.0f));
        model = matrixTorsoMujer2;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        torso_mujer.Draw(lightingShader);

        // 2. Falda
        glm::mat4 matrixFaldaMujer2 = RotarEnPivote(matrixModelMujer2, pivoteCintura, anguloBaile, glm::vec3(0.0f, 0.0f, 1.0f));
        model = matrixFaldaMujer2;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        falda_mujer.Draw(lightingShader);

        // 3. Cabeza
        model = matrixTorsoMujer2;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        cabeza.Draw(lightingShader);

        // 4. Brazo Derecho
        glm::mat4 brazoDerMujer2 = RotarEnPivote(matrixTorsoMujer2, pivoteHombroDer, rotacionBrazos);
        model = glm::translate(brazoDerMujer2, glm::vec3(0.0f, 0.02f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        biceps.Draw(lightingShader);

        model = brazoDerMujer2;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        manoDer.Draw(lightingShader);

        // Brazo Izquierdo
        glm::mat4 brazoIzqMujer2 = RotarEnPivote(matrixTorsoMujer2, pivoteHombroIzq, -rotacionBrazos);
        model = glm::translate(brazoIzqMujer2, glm::vec3(0.03f, 0.02f, 0.0f));
        model = glm::scale(model, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        biceps.Draw(lightingShader);

        model = glm::translate(brazoIzqMujer2, glm::vec3(-0.03f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        manoIzq.Draw(lightingShader);

        // Pierna Derecha
        glm::mat4 piernaDerMujer2 = RotarEnPivote(matrixModelMujer2, pivoteCaderaDer, rotacionPiernasMujer);
        piernaDerMujer2 = glm::translate(piernaDerMujer2, glm::vec3(-0.13f, 0.0f, 0.0f));
        piernaDerMujer2 = glm::scale(piernaDerMujer2, glm::vec3(0.9f, 0.8f, 0.9f));
        model = piernaDerMujer2;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        musDer.Draw(lightingShader);

        model = piernaDerMujer2;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pieiz.Draw(lightingShader);

        // Pierna Izquierda
        glm::mat4 piernaIzqMujer2 = RotarEnPivote(matrixModelMujer2, pivoteCaderaIzq, -rotacionPiernasMujer);
        piernaIzqMujer2 = glm::translate(piernaIzqMujer2, glm::vec3(0.00f, 0.0f, 0.0f));
        piernaIzqMujer2 = glm::scale(piernaIzqMujer2, glm::vec3(0.9f, 0.8f, 0.9f));
        model = piernaIzqMujer2;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        musDer.Draw(lightingShader);

        model = piernaIzqMujer2;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pieiz.Draw(lightingShader);

        // ===================== PERSONAJE HOMBRE =====================
        glm::mat4 matrixModelCuerpo2 = glm::mat4(1.0f);
        matrixModelCuerpo2 = glm::translate(matrixModelCuerpo2, glm::vec3(2.0f, 0.8f, 0.0f));

        model = matrixModelCuerpo2 ;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        cuerpo.Draw(lightingShader);
        cabeza.Draw(lightingShader);
        mascara.Draw(lightingShader);

        glm::mat4 matrixModelBrazoDer2 = RotarEnPivote(matrixModelCuerpo2, pivoteHombroDer, rotacionBrazos);
        model = matrixModelBrazoDer2;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        biceps.Draw(lightingShader);
        manoDer.Draw(lightingShader);

        glm::mat4 matrixModelBrazoIzq2 = RotarEnPivote(matrixModelCuerpo2, pivoteHombroIzq, -rotacionBrazos);
        model = glm::translate(matrixModelBrazoIzq2, glm::vec3(0.06f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        biceps.Draw(lightingShader);

        model = matrixModelBrazoIzq2;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        manoIzq.Draw(lightingShader);

        glm::mat4 matrixModelPiernaDer2 = RotarEnPivote(matrixModelCuerpo2, pivoteCaderaDer, rotacionPiernas);
        model = matrixModelPiernaDer2;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        musDer.Draw(lightingShader);

        model = glm::scale(matrixModelPiernaDer2, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pieiz.Draw(lightingShader);

        glm::mat4 matrixModelPiernaIzq2 = RotarEnPivote(matrixModelCuerpo2, pivoteCaderaIzq, -rotacionPiernas);
        model = glm::scale(matrixModelPiernaIzq2, glm::vec3(-1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        musDer.Draw(lightingShader);

        model = matrixModelPiernaIzq2;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pieiz.Draw(lightingShader);

        // ===================== LÁMPARA =====================
        lightingShader.Use();        
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
        model = glm::translate(model, glm::vec3(-1.0f, -0.09f, -10.0f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        copa.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.0f, -0.09f, -10.0f));
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

        // Le quitamos la traslación a la cámara para que el cielo nos siga
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));

        glUniformMatrix4fv(glGetUniformLocation(skyboxshader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(skyboxshader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // ¡El truco para que no se rompan las texturas!
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
        glDisable(GL_CULL_FACE);

        papelShader.Use();
		

        glm::mat4 viewPapel = camera.GetViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(papelShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(viewPapel));
        glUniformMatrix4fv(glGetUniformLocation(papelShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glUniform1i(glGetUniformLocation(papelShader.Program, "papelTex"), 0);
        glUniform1f(glGetUniformLocation(papelShader.Program, "opacidad"), 1.0f);

        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(quadVAO);

        // Arreglo de texturas
        GLuint texturas[] = { papelTexture1, papelTexture2, papelTexture3 };
        float posicionesZ[] = { 2.1f, -2.2f, -6.2f, -11.2f };
     

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

        // Arreglo con las posiciones de las 4 copas
        glm::vec3 copasPosiciones[] = {
            glm::vec3(1.0f, 0.0f, 2.3f),   // Copa 1
            glm::vec3(-1.0f, 0.0f, 2.3f),  // Copa 2
            glm::vec3(1.0f, 0.0f, -10.0f),  // Copa 3
            glm::vec3(-1.0f, 0.0f, -10.0f)  // Copa 4
        };

        // Bucle para dibujar el humo en cada una de las 4 copas
        for (int copa = 0; copa < 4; copa++) {

            // Bucle para hacer que cada columna de humo tenga 3 "nubes" para que se vea más denso
            for (int h = 0; h < 3; h++) {

                // Matemáticas para que suban infinitamente con un pequeño desfase
                float tiempoHumo = fmod(currentFrame * 0.3f + (h * 0.33f) + (copa * 0.15f), 1.0f);

                // Sube en Y
                float posY = 0.0f + (tiempoHumo * 2.5f);
                float escalaY = 1.0f + (tiempoHumo * 1.5f);
                float escalaX = 0.5f + (tiempoHumo * 0.5f);

                // Se desvanece al llegar arriba
                float opacidadHumo = 1.0f - tiempoHumo;
                glUniform1f(glGetUniformLocation(papelShader.Program, "opacidad"), opacidadHumo);

                // Movimiento del viento en X (le sumamos 'h' y 'copa' para que no se muevan igual)
                float vientoX = sin(currentFrame * 1.5f + h + copa) * 0.2f;

                glm::mat4 modelHumo = glm::mat4(1.0f);
                // Usamos las coordenadas de la copa actual
                modelHumo = glm::translate(modelHumo, glm::vec3(copasPosiciones[copa].x + vientoX, posY, copasPosiciones[copa].z));

                // Rotación ligera
                modelHumo = glm::rotate(modelHumo, glm::radians(tiempoHumo * 15.0f), glm::vec3(0.0f, 0.0f, 1.0f));

                // Escalamos
                modelHumo = glm::scale(modelHumo, glm::vec3(escalaX, escalaY, 1.0f));

                glUniformMatrix4fv(glGetUniformLocation(papelShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelHumo));
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }

        glBindVertexArray(0);
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        // ===================== TARJETA DESCRIPTIVA (CUADRO DE INFORMACIÓN) =====================
        if (esperandoAccion) {
            glDisable(GL_DEPTH_TEST); // Para que la tarjeta se dibuje siempre al frente en pantalla
            glDisable(GL_CULL_FACE);  // 
			papelShader.Use();
			TemplateShader.Use();

            // Configuramos una matriz ortográfica o proyectada fija frente a la cámara
            glm::mat4 projectionUI = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);
            glm::mat4 viewUI = camera.GetViewMatrix();

            glUniformMatrix4fv(glGetUniformLocation(TemplateShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(viewUI));
            glUniformMatrix4fv(glGetUniformLocation(TemplateShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projectionUI));


            // SELECCIONAR LA TEXTURA SEGÚN LA ESTACIÓN ACTUAL DEL TOUR
     
            GLuint texturaActualTarjeta = papelTexture0;// Valor por defecto

            switch (estacionActual) {
            case 0: texturaActualTarjeta = tarjetaArco; break;         // Estación 0: Arco y Cempasúchil
            case 1: texturaActualTarjeta = tarjetaPersonajes; break;   // Estación 1: Hombre y Mujer (Danza)
            case 2: texturaActualTarjeta = tarjetaOfrenda; break;      // Estación 2: Ofrenda
            case 3: texturaActualTarjeta = tarjetaAbuelitos; break;    // Estación 3: Abuelitos (Danza de los Viejitos)
            case 4: texturaActualTarjeta = tarjetaCementerio; break;   // Estación 4: Cementerio
            }

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texturaActualTarjeta);
            glUniform1i(glGetUniformLocation(TemplateShader.Program, "papelTex"), 0);
            glUniform1f(glGetUniformLocation(TemplateShader.Program, "opacidad"), 0.98f); // Casi totalmente opaca para leer perfecto

            // Posicionamos el cuadro de información justo enfrente de la cámara
            glm::mat4 modelTarjeta = glm::mat4(1.0f);
            glm::vec3 posTarjeta = camera.GetPosition() + (camera.GetFront() * 1.5f);
            modelTarjeta = glm::translate(modelTarjeta, posTarjeta);
            if (estacionActual == 2) {
                modelTarjeta = glm::rotate(modelTarjeta, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            }


            // Ajustamos el tamaño de la tarjeta en pantalla (ancho, alto, grosor)
            modelTarjeta = glm::scale(modelTarjeta, glm::vec3(1.3f, 0.8f, 1.0f));

            glUniformMatrix4fv(glGetUniformLocation(TemplateShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelTarjeta));

            glBindVertexArray(quadVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);

            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST); // Restauramos el depth test
        }
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

// Movimiento de cámara y luces
void DoMovement()
{
    // SI EL TOUR ESTÁ ACTIVO Y NO ESTÁ PAUSADO, BLOQUEAMOS EL TECLADO POR COMPLETO
    if (tourActivo && !tourPausado) {
        return;
    }
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
    //campana
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        animandoCampana = !animandoCampana;
        if (animandoCampana) {
            maxAnguloCampana = 25.0f;       // Fuerza inicial
            velCampanaActual = VEL_CAMPANA_BASE;
            estadoCampana = 1;              // Inicia el balanceo FSM
        }
    }
    // B: Inicia / detiene el baile de los abuelitos
    if (key == GLFW_KEY_B && action == GLFW_PRESS) {

        animandoAbuelitos = !animandoAbuelitos;

        if (animandoAbuelitos) {
            estadoBaileAbuelitos = 1;

            std::cout << "BAILE DE LOS ABUELITOS ACTIVADO" << std::endl;
        }
        else {
            estadoBaileAbuelitos = 0;

            std::cout << "BAILE DE LOS ABUELITOS DESACTIVADO" << std::endl;
        }
    }
    //Tecla para posicionar e iniciar tour S
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        tourActivo = !tourActivo;
        tourPausado = false;
        esperandoAccion = false;
        if (tourActivo && estacionActual > 4) {
            estacionActual = 0; // Reinicia si ya había acabado
            progresoEstacion = 0.0f;
        }
        std::cout << (tourActivo ? ">>> TOUR VIRTUAL INICIADO <<<" : ">>> TOUR VIRTUAL DETENIDO <<<") << std::endl;
    }
    // F: Pausar / Continuar el Tour de la Cámara (o avanzar si está esperando en una tarjeta)
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        if (esperandoAccion) {
            // Si está detenido mostrando una tarjeta, al presionar X continúa a la siguiente estación
            esperandoAccion = false;
            estacionActual++;
            progresoEstacion = 0.0f;
        }
        else {
            // Si está en movimiento, pausa o reanuda
            tourPausado = !tourPausado;
            std::cout << (tourPausado ? "Tour PAUSADO" : "Tour REANUDADO") << std::endl;
        }
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


void Animation() {

    // =========================================================================
    // 1. LÓGICA DEL BAILE DE LA MUJER
    // =========================================================================
    if (!animandoBaile) {
        estadoBaile = 0;
    }

    bool llegoAng = false, llegoFalda = false, llegoPie = false;

    switch (estadoBaile) {
    case 0:
        MoverHacia(anguloBaile, 0.0f, VEL_BAILE);
        MoverHacia(anguloFalda, 0.0f, VEL_FALDA);
        MoverHacia(rotacionPiernasMujer, 0.0f, VEL_PIERNA);
        if (animandoBaile) estadoBaile = 1;
        break;

    case 1:
        llegoAng = MoverHacia(anguloBaile, MAX_BAILE, VEL_BAILE);
        llegoFalda = MoverHacia(anguloFalda, MAX_FALDA, VEL_FALDA);
        llegoPie = MoverHacia(rotacionPiernasMujer, MAX_PIERNA, VEL_PIERNA);
        if (llegoAng && llegoFalda && llegoPie) estadoBaile = 2;
        break;

    case 2:
        llegoAng = MoverHacia(anguloBaile, 0.0f, VEL_BAILE);
        llegoFalda = MoverHacia(anguloFalda, 0.0f, VEL_FALDA);
        llegoPie = MoverHacia(rotacionPiernasMujer, 0.0f, VEL_PIERNA);
        if (llegoAng && llegoFalda && llegoPie) estadoBaile = 3;
        break;

    case 3:
        llegoAng = MoverHacia(anguloBaile, -MAX_BAILE, VEL_BAILE);
        llegoFalda = MoverHacia(anguloFalda, -MAX_FALDA, VEL_FALDA);
        llegoPie = MoverHacia(rotacionPiernasMujer, -MAX_PIERNA, VEL_PIERNA);
        if (llegoAng && llegoFalda && llegoPie) estadoBaile = 4;
        break;

    case 4:
        llegoAng = MoverHacia(anguloBaile, 0.0f, VEL_BAILE);
        llegoFalda = MoverHacia(anguloFalda, 0.0f, VEL_FALDA);
        llegoPie = MoverHacia(rotacionPiernasMujer, 0.0f, VEL_PIERNA);
        if (llegoAng && llegoFalda && llegoPie) estadoBaile = 1;
        break;
    }

    // =========================================================================
    // 2. LÓGICA DEL BAILE DE LOS ABUELITOS
    // =========================================================================

    bool llegoInclinacionAbuelitos = false;
    bool llegoBrazosAbuelitos = false;
    bool llegoPiernasAbuelitos = false;

    switch (estadoBaileAbuelitos)
    {
    case 0: // REPOSO
        MoverHacia(inclinacionAbuelitos, 0.0f, 15.0f);
        MoverHacia(rotacionBrazosAbuelitos, 0.0f, 30.0f);
        MoverHacia(rotacionPiernasAbuelitos, 0.0f, 20.0f);
        break;

    case 1: // INCLINACIÓN HACIA LA DERECHA
        llegoInclinacionAbuelitos =
            MoverHacia(inclinacionAbuelitos, 3.0f, 15.0f);

        llegoBrazosAbuelitos =
            MoverHacia(rotacionBrazosAbuelitos, 10.0f, 30.0f);

        llegoPiernasAbuelitos =
            MoverHacia(rotacionPiernasAbuelitos, 4.0f, 20.0f);

        if (llegoInclinacionAbuelitos &&
            llegoBrazosAbuelitos &&
            llegoPiernasAbuelitos)
        {
            estadoBaileAbuelitos = 2;
        }
        break;

    case 2: // INCLINACIÓN HACIA LA IZQUIERDA
        llegoInclinacionAbuelitos =
            MoverHacia(inclinacionAbuelitos, -3.0f, 15.0f);

        llegoBrazosAbuelitos =
            MoverHacia(rotacionBrazosAbuelitos, -10.0f, 30.0f);

        llegoPiernasAbuelitos =
            MoverHacia(rotacionPiernasAbuelitos, -4.0f, 20.0f);

        if (llegoInclinacionAbuelitos &&
            llegoBrazosAbuelitos &&
            llegoPiernasAbuelitos)
        {
            estadoBaileAbuelitos = 3;
        }
        break;

    case 3: // REGRESO AL CENTRO
        llegoInclinacionAbuelitos =
            MoverHacia(inclinacionAbuelitos, 0.0f, 15.0f);

        llegoBrazosAbuelitos =
            MoverHacia(rotacionBrazosAbuelitos, 0.0f, 30.0f);

        llegoPiernasAbuelitos =
            MoverHacia(rotacionPiernasAbuelitos, 0.0f, 20.0f);

        if (llegoInclinacionAbuelitos &&
            llegoBrazosAbuelitos &&
            llegoPiernasAbuelitos)
        {
            if (animandoAbuelitos)
            {
                estadoBaileAbuelitos = 1;
            }
            else
            {
                estadoBaileAbuelitos = 0;
            }
        }
        break;
    }

    // =========================================================================
    // 3. LÓGICA DE LA CAMPANA
    // =========================================================================

    if (!animandoCampana) {
        estadoCampana = 0;
    }

    bool llegoCampana = false;

    switch (estadoCampana) {
    case 0:
        MoverHacia(anguloCampana, 0.0f, VEL_CAMPANA_BASE);
        break;

    case 1:
        llegoCampana = MoverHacia(anguloCampana, maxAnguloCampana, velCampanaActual);
        if (llegoCampana) {
            estadoCampana = 2;
        }
        break;

    case 2:
        llegoCampana = MoverHacia(anguloCampana, -maxAnguloCampana, velCampanaActual);
        if (llegoCampana) {
            maxAnguloCampana *= 0.65f;
            velCampanaActual *= 0.85f;

            if (maxAnguloCampana < 0.3f) {
                animandoCampana = false;
                estadoCampana = 0;
            }
            else {
                estadoCampana = 1;
            }
        }
        break;
    }
}


void ActualizarTourCamara() {
    if (!tourActivo || tourPausado || esperandoAccion) return;

    // Posiciones donde se detendrá la cámara en cada estación
    glm::vec3 posiciones[] = {
        glm::vec3(0.0f, 1.8f, 1.8f),    // Estación 0: INICIO A MITAD DEL ARCO
        glm::vec3(0.0f, 1.6f, 1.0f),    // Estación 1: Modelos de Hombre y Mujer
        glm::vec3(1.5f, 1.5f, -4.0f),   // Estación 2: Altar / Ofrenda (Posicionada para ver hacia -X)
        glm::vec3(2.2f, 1.5f, -8.0f),   // Estación 3: Modelos Abuelitos
        glm::vec3(0.0f, 2.0f, -12.0f)   // Estación 4: Cementerio
    };

    // Puntos específicos a los que debe mirar la cámara en cada estación
    glm::vec3 objetivos[] = {
        glm::vec3(0.0f, 1.0f, -2.0f),   // Mirando hacia adelante por el túnel
        glm::vec3(0.0f, 1.0f, 0.0f),    // Mirando a los modelos
        glm::vec3(-2.0f, 1.0f, -4.0f),  // Estación 2: El Altar (ubicado en -2.0f, -4.0f) -> Queda enfrente mirando a -X
        glm::vec3(2.0f, 1.0f, -9.2f),   // Mirando a los abuelitos
        glm::vec3(0.0f, 1.0f, -15.0f)   // Mirando al cementerio
    };
    if (progresoEstacion >= 1.0f) {
        progresoEstacion = 1.0f;
        esperandoAccion = true;

    }
    int maxEstaciones = 4;
    if (estacionActual > maxEstaciones) {
        tourActivo = false; // Fin del recorrido
        return;
    }

    // Velocidad de movimiento de la cámara entre estaciones
    progresoEstacion += deltaTime * 0.4f;

    // Interpolar posición actual hacia la posición de la estación
    glm::vec3 posActual = camera.GetPosition();
    glm::vec3 posDestino = posiciones[estacionActual];
    glm::vec3 nuevaPos = glm::mix(posActual, posDestino, deltaTime * 2.0f);

    camera.SetPosition(nuevaPos);

    // Hacer que la cámara rote suavemente hacia el objetivo de la estación actual
    glm::vec3 objActual = objetivos[estacionActual];
    camera.PointTo(objActual);
}
// Función para limitar los movimientos de la cámara dentro de los límites del escenario
void LimitarPosicionCamara() {
    glm::vec3 pos = camera.GetPosition();

    // Límites en el Eje X 
    if (pos.x < -3.0f) pos.x = -3.0f;
    if (pos.x > 3.0f) pos.x = 3.0f;

    // Límites en el Eje Y 
    if (pos.y < 0.6f) pos.y = 0.6f;  
    if (pos.y > 4.0f) pos.y = 4.0f;

    // Límites en el Eje Z (Largo del escenario: desde el inicio del arco hasta el fondo del cementerio)
    if (pos.z < -17.5f) pos.z = -17.5f; // Límite en el fondo (Cementerio)
    if (pos.z > 3.5f) pos.z = 3.5f;  // Límite al frente (Entrada del arco)

    // Aplicamos la posición limitada de regreso a la cámara
    camera.SetPosition(pos);
}
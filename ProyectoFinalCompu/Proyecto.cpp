//Proyecto final
#include <iostream>
#include <cmath>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include "stb_image.h"

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

// Function prototypes
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();

// Window dimensions
const GLuint WIDTH = 1000, HEIGHT =800;
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

// Deltatime
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
// Variables de animación jerárquica para las extremidades del personaje
GLfloat rotacionBrazos = 0.0f;
GLfloat rotacionPiernas = 0.0f;

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

    // Modelos:  piso y arco
    Model Piso((char*)"Models/Arco/piso.obj");
    Model base((char*)"Models/Arco/base.obj");
    Model PisoA((char*)"Models/Arco/pisoA.obj");
    Model velas((char*)"Models/velas/velas.obj");
	Model ofrenda((char*)"Models/ofrenda/ofrenda.obj");
	Model cabeza((char*)"Models/personaje/cabeza/cabeza.obj");
	Model biceps((char*)"Models/personaje/bicep/bicep.obj");
	Model cuerpo((char*)"Models/personaje/cuerpo/cuerpo.obj");
	Model manoDer((char*)"Models/personaje/manoDer/manoDer.obj");
	Model manoIzq((char*)"Models/personaje/manoIzq/manoIzq.obj");
	Model mascara((char*)"Models/personaje/mascara/mascara.obj");
	Model musDer((char*)"Models/personaje/musDer/musDer.obj");
	Model piederecho((char*)"Models/personaje/piederecho/pieDer.obj");


    // Set texture units
    lightingShader.Use();
    glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.difuse"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.specular"), 1);

    glm::mat4 projection = glm::perspective(camera.GetZoom(),
        (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 200.0f);

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Delta time
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Events
        glfwPollEvents();
        DoMovement();

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

        // Directional light
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.6f, 0.6f, 0.6f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.6f, 0.6f, 0.6f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.3f, 0.3f, 0.3f);

        // Point light 1
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"),
            pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"),
            lightColor.x * 0.2f, lightColor.y * 0.2f, lightColor.z * 0.2f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"),
            lightColor.x, lightColor.y, lightColor.z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 1.0f, 1.0f, 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.09f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"), 0.032f);

        // SpotLight
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"),
            camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"),
            camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.2f, 0.2f, 0.8f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), 0.2f, 0.2f, 0.8f);
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
        model = glm::translate(model, glm::vec3(-2.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.4f, 0.2f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        Piso.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.4f, 0.2f,1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        PisoA.Draw(lightingShader);


        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.4f, 0.2f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        Piso.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 0.0f,-5.0f));
        model = glm::scale(model, glm::vec3(0.4f, 0.2f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        Piso.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.0f, 0.0f, -5.0f));
        model = glm::scale(model, glm::vec3(0.4f, 0.2f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        Piso.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -5.0f));
        model = glm::scale(model, glm::vec3(0.4f, 0.2f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        PisoA.Draw(lightingShader);

       
        // ===================== ARCO =====================
        model = glm::mat4(1.0f);
        // Ajusta aquí posición / escala / rotación si tu arco lo necesita:
         model = glm::translate(model, glm::vec3(0.0f, -0.1f, 1.8f));
        // model = glm::scale(model, glm::vec3(1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        base.Draw(lightingShader);

        model = glm::mat4(1.0f);
        // Ajusta aquí posición / escala / rotación si tu arco lo necesita:
        model = glm::translate(model, glm::vec3(0.0f, -0.1f, -6.0f));
        // model = glm::scale(model, glm::vec3(1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        base.Draw(lightingShader);

        // ===================== Velas =====================
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.0f, -0.09f, 2.3f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        velas.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.7f, -0.09f, 2.3f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        velas.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0f, -0.09f, 2.3f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        velas.Draw(lightingShader);
        model = glm::mat4(1.0f);

        model = glm::translate(model, glm::vec3(-0.7f, -0.09f,2.3f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        velas.Draw(lightingShader);


        //Dibujo de la ofrenda //
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.0f, 0.0f, -4.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        ofrenda.Draw(lightingShader);


        //Dibujo del la danza de //
        glm::mat4 matrixModelCuerpo = glm::mat4(1.0f);
        matrixModelCuerpo = glm::translate(matrixModelCuerpo, glm::vec3(0.0f, 0.0f, -2.0f)); // Posición general en el mundo

        // Dibujar Cuerpo
        model = matrixModelCuerpo;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        cuerpo.Draw(lightingShader);

        //// Cabeza (Hereda del cuerpo)
        //model = glm::translate(matrixModelCuerpo, glm::vec3(0.0f, 1.2f, 0.0f)); // Ajuste relativo sobre el cuerpo
        //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        //cabeza.Draw(lightingShader);

        //// Máscara (Hereda de la cabeza o directamente del cuerpo en la zona frontal)
        //model = glm::translate(matrixModelCuerpo, glm::vec3(0.0f, 1.2f, 0.2f));
        //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        //mascara.Draw(lightingShader);

        //// Bicep Derecho (Hereda del cuerpo y añade rotación de articulación)
        //glm::mat4 matrixModelBicepDer = glm::translate(matrixModelCuerpo, glm::vec3(-0.5f, 0.8f, 0.0f));
        //matrixModelBicepDer = glm::rotate(matrixModelBicepDer, glm::radians(rotacionBrazos), glm::vec3(1.0f, 0.0f, 0.0f));
        //model = matrixModelBicepDer;
        //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        //biceps.Draw(lightingShader);

        //// Mano Derecha (Hereda del Bicep Derecho - Jerarquía secundaria)
        //model = glm::translate(matrixModelBicepDer, glm::vec3(0.0f, -0.6f, 0.0f));
        //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        //manoDer.Draw(lightingShader);

        //// Bicep Izquierdo (Hereda del cuerpo)
        //glm::mat4 matrixModelBicepIzq = glm::translate(matrixModelCuerpo, glm::vec3(0.5f, 0.8f, 0.0f));
        //matrixModelBicepIzq = glm::rotate(matrixModelBicepIzq, glm::radians(-rotacionBrazos), glm::vec3(1.0f, 0.0f, 0.0f));
        //model = matrixModelBicepIzq;
        //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        //biceps.Draw(lightingShader);

        //// Mano Izquierda (Hereda del Bicep Izquierdo)
        //model = glm::translate(matrixModelBicepIzq, glm::vec3(0.0f, -0.6f, 0.0f));
        //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        //manoIzq.Draw(lightingShader);

        //// Muslo Derecho / Pierna (Hereda del cuerpo)
        //glm::mat4 matrixModelMusDer = glm::translate(matrixModelCuerpo, glm::vec3(-0.2f, -0.2f, 0.0f));
        //matrixModelMusDer = glm::rotate(matrixModelMusDer, glm::radians(rotacionPiernas), glm::vec3(1.0f, 0.0f, 0.0f));
        //model = matrixModelMusDer;
        //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        //musDer.Draw(lightingShader);

        //// Pie Derecho (Hereda del muslo derecho)
        //model = glm::translate(matrixModelMusDer, glm::vec3(0.0f, -0.8f, 0.0f));
        //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        //piederecho.Draw(lightingShader);

        
        
        // ===================== LÁMPARA (opcional) =====================
        // Si NO quieres ver el cubito de la luz, comenta este bloque completo.
        lampShader.Use();
        modelLoc = glGetUniformLocation(lampShader.Program, "model");
        viewLoc = glGetUniformLocation(lampShader.Program, "view");
        projLoc = glGetUniformLocation(lampShader.Program, "projection");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        model = glm::mat4(1.0f);
        model = glm::translate(model, pointLightPositions[0]);
        model = glm::scale(model, glm::vec3(0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));



        // (Si quitaste el VAO de vértices, comenta también el dibujo del cubo)
        // glBindVertexArray(VAO);
        // glDrawArrays(GL_TRIANGLES, 0, 36);
        // glBindVertexArray(0);

        // Swap buffers
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

// Movimiento de cámara y luces
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
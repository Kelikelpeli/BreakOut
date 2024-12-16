#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Shader.h" // Incluir la clase Shader

// Declaración de la función de callback para redimensionar
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// Declaración de la función para manejar entradas
void processInput(GLFWwindow* window);

int main()
{
    // Inicializar GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Configuración de GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Crear la ventana
    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Configurar el callback de redimensionamiento
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Inicializar GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Establecer el tamaño de la ventana gráfica
    glViewport(0, 0, 800, 600);

    // Datos de los vértices del triángulo
    float vertices[] = {
    0.5f, 0.5f, 0.0f,// top right
    0.5f, -0.5f, 0.0f,// bottom right
    -0.5f, -0.5f, 0.0f,// bottom left
    -0.5f, 0.5f, 0.0f// top left
    };
    unsigned int indices[] = { // note that we start from 0!
    0, 1, 3, // first triangle
    1, 2, 3 // second triangle
    };


    // Crear y vincular el VAO
    unsigned int NEW;
    glGenVertexArrays(1, &NEW);
    glBindVertexArray(NEW);

    // Crear e inicializar el VBO
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Configurar los atributos de vértice
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Desvincular el VAO
    glBindVertexArray(0);

    // Inicializar los shaders
    Shader ourShader("D:/Raquel/PG/OpenGL/BreakOut/src/vertexshader.vs", "D:/Raquel/PG/OpenGL/BreakOut/src/fragmentshader.fs");

    // Bucle principal del juego
    while (!glfwWindowShouldClose(window))
    {
        // Procesar entradas
        processInput(window);

        // Limpiar la pantalla con un color
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Verde oscuro azulado
        glClear(GL_COLOR_BUFFER_BIT);

        // Usar el shader
        ourShader.use();

        // Dibujar el triángulo
        glBindVertexArray(NEW);
      //  glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


        // Intercambiar buffers y manejar eventos
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Liberar recursos
    glDeleteVertexArrays(1, &NEW);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}

// Definición de la función de callback para redimensionar
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Definición de la función para manejar entradas
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

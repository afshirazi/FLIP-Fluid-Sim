#include <glad/glad.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "util.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

int main() {
    // Taken from learnopengl.com
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glViewport(0, 0, 800, 600);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    float floor_vertices[] = {
     -0.5f, 0.f,  0.5f,
     -0.5f, 0.f, -0.5f,
      0.5f, 0.f,  0.5f,
      0.5f, 0.f, -0.5f
    };
    unsigned int floor_indices[] = {
        0, 1, 2,
        1, 3, 2
    };

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_vertices), floor_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int shaderProg = createAndUseShaderProgram();

    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floor_indices), floor_indices, GL_STATIC_DRAW);

    glm::mat4 proj = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    proj = glm::perspective(glm::radians(55.0f), 8.f/6.f, 0.1f, 10.0f);
    model = glm::rotate(model, glm::radians(40.f), glm::vec3(1.f, 0.f, 0.f));
    model = glm::rotate(model, glm::radians(-35.0f), glm::vec3(0.f, 1.f, 0.f));
    model = glm::scale(model, glm::vec3(1.5, 1.5, 1.5));
    view = glm::translate(view, glm::vec3(0.f, 0.2f, -1.1f));

    glm::mat4 transform = proj * view * model;

    GLint transformLoc = glGetUniformLocation(shaderProg, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));


    while (!glfwWindowShouldClose(window))
    {
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(VAO);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
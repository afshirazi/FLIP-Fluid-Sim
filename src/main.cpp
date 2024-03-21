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

    GLfloat floor_vertices[] = {
     -0.5f, 0.f,  0.5f,
     -0.5f, 0.f, -0.5f,
      0.5f, 0.f,  0.5f,
      0.5f, 0.f, -0.5f
    };
    GLuint floor_indices[] = {
        0, 1, 2,
        1, 3, 2
    };

    GLfloat points_vertices[] = {
        -0.3f, 0.2f,  0.f,
        -0.25f, 0.2f,  0.f,
        -0.2f, 0.2f,  0.f,
        -0.15f, 0.2f,  0.f,
        -0.1f, 0.2f,  0.f,
        -0.3f, 0.2f,  0.1f,
        -0.25f, 0.2f,  0.1f,
        -0.2f, 0.2f,  0.1f,
        -0.15f, 0.2f,  0.1f,
        -0.1f, 0.2f,  0.1f,
        -0.3f, 0.2f,  0.2f,
        -0.25f, 0.2f,  0.2f,
        -0.2f, 0.2f,  0.2f,
        -0.15f, 0.2f,  0.2f,
        -0.1f, 0.2f,  0.2f,
    };

    GLuint floor_VAO;
    glGenVertexArrays(1, &floor_VAO);
    glBindVertexArray(floor_VAO);

    GLuint floor_VBO;
    glGenBuffers(1, &floor_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, floor_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_vertices), floor_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    
    GLuint floor_EBO;
    glGenBuffers(1, &floor_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floor_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floor_indices), floor_indices, GL_STATIC_DRAW);

    GLuint points_VAO;
    glGenVertexArrays(1, &points_VAO);
    glBindVertexArray(points_VAO);

    GLuint points_VBO;
    glGenBuffers(1, &points_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, points_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points_vertices), points_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    
    GLuint shaderProg = createAndUseShaderProgram();

    glm::mat4 proj = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 transform;
    GLint transformLoc = glGetUniformLocation(shaderProg, "transform");

    while (!glfwWindowShouldClose(window))
    {
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(floor_VAO);

        // Transformations for floor
        proj = glm::perspective(glm::radians(55.0f), 8.f / 6.f, 0.1f, 10.0f);
        model = glm::rotate(model, glm::radians(40.f), glm::vec3(1.f, 0.f, 0.f));
        model = glm::rotate(model, glm::radians(-35.0f), glm::vec3(0.f, 1.f, 0.f));
        model = glm::scale(model, glm::vec3(1.5, 1.5, 1.5));
        view = glm::translate(view, glm::vec3(0.f, 0.2f, -1.1f));

        transform = proj * view * model;
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0); // unbind
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
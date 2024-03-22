#include <glad/glad.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "util.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void applyGrav(GLfloat dt);

GLfloat particles_pos[] = {
        -0.3f, 0.2f,  0.f,
        -0.25f, 0.2f,  0.f,
        -0.2f, 0.2f,  0.f,
        -0.15f, 0.2f,  0.f,
        -0.1f, 0.2f,  0.f,
        -0.3f, 0.2f,  -0.1f,
        -0.25f, 0.2f,  -0.1f,
        -0.2f, 0.2f,  -0.1f,
        -0.15f, 0.2f,  -0.1f,
        -0.1f, 0.2f,  -0.1f,
        -0.3f, 0.2f,  -0.2f,
        -0.25f, 0.2f,  -0.2f,
        -0.2f, 0.2f,  -0.2f,
        -0.15f, 0.2f,  -0.2f,
        -0.1f, 0.2f,  -0.2f,
};

GLfloat particles_vel[] = {
        0.f, 0.f,  0.f,
        0.f, 0.f,  0.f,
        0.f, 0.f,  0.f,
        0.f, 0.f,  0.f,
        0.f, 0.f,  0.f,
        0.f, 0.f,  0.f,
        0.f, 0.f,  0.f,
        0.f, 0.f,  0.f,
        0.f, 0.f,  0.f,
        0.f, 0.f,  0.f,
        0.f, 0.f,  0.f,
        0.f, 0.f,  0.f,
        0.f, 0.f,  0.f,
        0.f, 0.f,  0.f,
        0.f, 0.f,  0.f,
};

int main() {
    // Some code taken from learnopengl.com
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

    // VAO for "floor"
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

    // VAO for particles
    GLuint particles_VAO;
    glGenVertexArrays(1, &particles_VAO);
    glBindVertexArray(particles_VAO);

    GLuint particles_VBO;
    glGenBuffers(1, &particles_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, particles_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particles_pos), particles_pos, GL_STREAM_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    GLuint shaderProg = createAndUseShaderProgram();

    glm::mat4 proj = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 floor_transform;
    glm::mat4 particles_transform;

    // Transformations for floor
    proj = glm::perspective(glm::radians(55.0f), 8.f / 6.f, 0.1f, 10.0f);
    model = glm::rotate(model, glm::radians(40.f), glm::vec3(1.f, 0.f, 0.f));
    model = glm::rotate(model, glm::radians(-35.0f), glm::vec3(0.f, 1.f, 0.f));
    model = glm::scale(model, glm::vec3(1.5, 1.5, 1.5));
    view = glm::translate(view, glm::vec3(0.f, 0.2f, -1.1f));
    floor_transform = proj * view * model;
    
    // Transformations for particles
    view = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(0.67, 0.67, 0.67));
    view = glm::translate(view, glm::vec3(0.f, 0.f, -1.1f));
    particles_transform = proj * view * model;

    GLint transformLoc = glGetUniformLocation(shaderProg, "transform");
    GLint colorLoc = glGetUniformLocation(shaderProg, "vertColor");

    // rendering loop
    while (!glfwWindowShouldClose(window))
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Draw floor
        glBindVertexArray(floor_VAO);
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(floor_transform));
        glUniform3f(colorLoc, 0.5f, 0.f, 0.f); // color red
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // Draw particles
        glBindVertexArray(particles_VAO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(particles_pos), particles_pos, GL_STREAM_DRAW); // Update particle positions in VBO
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(particles_transform));
        glUniform3f(colorLoc, 0.f, 0.f, 0.5f); // color blue
        glPointSize(5);
        glDrawArrays(GL_POINTS, 0, 15);

        // Apply forces
        applyGrav(1.f / 10000.f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void applyGrav(GLfloat dt)
{
    for (int i = 1; i < 45; i += 3)
    {
        particles_vel[i] -= 9.82 * dt;
        particles_pos[i] += particles_vel[i] * dt;
    }
}
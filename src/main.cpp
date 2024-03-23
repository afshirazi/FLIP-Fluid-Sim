#include <glad/glad.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "util.h"
#include "scene.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void applyGrav(GLfloat dt);
void checkFloorCollision();

Scene setupFluidScene();

Scene scene;

/* REMOVE LATER

GLfloat particles_pos[] = {
        -0.5f, 0.2f,  0.5f,
        -0.25f, 0.2f,  0.5f,
        -0.2f, 0.2f,  0.5f,
        -0.15f, 0.2f,  0.5f,
        -0.f, 0.2f,  0.5f,
        -0.5f, 0.2f,  -0.1f,
        -0.25f, 0.2f,  -0.1f,
        -0.2f, 0.2f,  -0.1f,
        -0.15f, 0.2f,  -0.1f,
        -0.f, 0.2f,  -0.1f,
        -0.5f, 0.2f,  -0.4f,
        -0.25f, 0.2f,  -0.4f,
        -0.2f, 0.2f,  -0.4f,
        -0.15f, 0.2f,  -0.4f,
        -0.f, 0.2f,  -0.4f,
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
*/


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

    scene = setupFluidScene();

    // VAO for particles
    GLuint particles_VAO;
    glGenVertexArrays(1, &particles_VAO);
    glBindVertexArray(particles_VAO);

    GLuint particles_VBO;
    glGenBuffers(1, &particles_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, particles_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * scene.num_p, scene.particles_pos, GL_STREAM_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    GLuint fShaderProg = createAndLinkFloorShaderProg();
    GLuint pShaderProg = createAndLinkParticleShaderProg();

    glm::mat4 proj = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 floor_transform;
    glm::mat4 particles_transform;

    // Transformations for floor
    proj = glm::perspective(glm::radians(55.0f), 8.f / 6.f, 0.1f, 10.0f);
    model = glm::rotate(model, glm::radians(40.f), glm::vec3(1.f, 0.f, 0.f));
    model = glm::rotate(model, glm::radians(-120.0f), glm::vec3(0.f, 1.f, 0.f));
    model = glm::scale(model, glm::vec3(1.5, 1.5, 1.5));
    view = glm::translate(view, glm::vec3(0.f, 0.1f, -1.1f));
    floor_transform = proj * view * model;
    
    // Transformations for particles
    model = glm::scale(model, glm::vec3(0.67, 0.67, 0.67));
    particles_transform = proj * view * model;

    GLint fTransformLoc = glGetUniformLocation(fShaderProg, "transform");
    GLint fColorLoc = glGetUniformLocation(fShaderProg, "vertColor");
    GLint pTransformLoc = glGetUniformLocation(pShaderProg, "transform");
    GLint pColorLoc = glGetUniformLocation(pShaderProg, "vertColor");

    // rendering loop
    while (!glfwWindowShouldClose(window))
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Use particle shaders
        glLinkProgram(fShaderProg);
        glUseProgram(fShaderProg);

        // Draw floor
        glBindVertexArray(floor_VAO);
        glUniformMatrix4fv(fTransformLoc, 1, GL_FALSE, glm::value_ptr(floor_transform));
        glUniform3f(fColorLoc, 0.29f, 0.29f, 0.29f); // color red
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // Use particle shaders
        glLinkProgram(pShaderProg);
        glUseProgram(pShaderProg);

        // Draw particles
        glBindVertexArray(particles_VAO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * scene.num_p, scene.particles_pos, GL_STREAM_DRAW); // Update particle positions in VBO
        glUniformMatrix4fv(pTransformLoc, 1, GL_FALSE, glm::value_ptr(particles_transform));
        glUniform3f(pColorLoc, 0.f, 0.f, 0.5f); // color blue
        glPointSize(5);
        glDrawArrays(GL_POINTS, 0, scene.num_p);

        // Apply forces
        applyGrav(1.f / 2000.f);
        checkFloorCollision();

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
    for (GLuint i = 1; i < scene.num_p * 3; i += 3)
    {
        scene.particles_vel[i] -= 9.82f * dt;
        scene.particles_pos[i] += scene.particles_vel[i ] * dt;
    }
}

void checkFloorCollision()
{
    for (GLuint i = 1; i < scene.num_p * 3; i += 3)
    {
        if (scene.particles_pos[i] <= 0.f)
        {
            scene.particles_pos[i] = 0.f;
            scene.particles_vel[i] = 0.f;
        }
    }
}

Scene setupFluidScene()
{
    const GLuint num_p_x = 32;
    const GLuint num_p_y = 32;
    const GLuint num_p_z = 32;
    const GLuint num_c_x = 20;
    const GLuint num_c_y = 20;
    const GLuint num_c_z = 20;

    const GLuint num_particles = num_p_x * num_p_y * num_p_z;
    const GLuint num_cells = num_c_x * num_c_y * num_c_z;

    Scene scene(num_particles, num_cells);

    int particle = 0;
    for (int i = 0; i < num_p_x; i++)
        for (int j = 0; j < num_p_y; j++)
            for (int k = 0; k < num_p_z; k++)
            {
                scene.particles_pos[particle++] = i / 32.f * 0.5f - 0.5f;
                scene.particles_pos[particle++] = j / 32.f * 0.2f + 0.1f;
                scene.particles_pos[particle++] = k / 32.f * 0.9f - 0.45f;
            }

    for (int i = 0; i < num_c_x; i++)
        for (int j = 0; j < num_c_y; j++)
            for (int k = 0; k < num_c_z; k++)
            {
                CellType curr_c_type = FLUID;
                if (i == 0 || i == num_c_x - 1 || j == 0 || k == 0 || k == num_c_z - 1)
                    curr_c_type = SOLID;

                scene.cell_type[i * num_c_y * num_c_z + j * num_c_z + k] = curr_c_type;
            }

    return scene;
}

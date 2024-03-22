#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <glad/glad.h>

enum CellType {
    AIR, FLUID, SOLID
};

struct Scene {
    GLfloat (*particles_pos)[3];
    GLfloat (*particles_vel)[3];
    CellType* cell_type;

    Scene(GLuint num_particles, GLuint num_cells)
    {
        particles_pos = new GLfloat[num_particles][3];
        particles_vel = new GLfloat[num_particles][3];
        cell_type[num_cells];
    }
};


struct Scene setupScene()
{
    const GLuint num_p_x = 32;
    const GLuint num_p_y = 32;
    const GLuint num_p_z = 32;
    const GLuint num_c_x = 20;
    const GLuint num_c_y = 20;
    const GLuint num_c_z = 20;

    const GLuint num_particles = num_p_x * num_p_y * num_p_z;
    const GLuint num_cells = num_c_x * num_c_y * num_c_z;

    struct Scene scene(num_particles, num_cells);

    int particle = 0;
    for (int i = 0; i < num_p_x; i++)
        for (int j = 0; j < num_p_y; j++)
            for (int k = 0; k < num_p_z; k++)
            {
                scene.particles_pos[particle][0] = i / 32.f * 0.5f - 0.5f;
                scene.particles_pos[particle][1] = j / 32.f * 0.2f + 0.05f;
                scene.particles_pos[particle][2] = k / 32.f * 0.9f - 0.45f;
                particle++;
            }

    for (int i = 0; i < num_c_x; i++)
        for (int j = 0; j < num_c_y; j++)
            for (int k = 0; k < num_c_z; k++)
            {
                CellType curr_c_type = FLUID;
                if (i == 0 || i == num_c_x - 1 || j == 0 || k == 0 || k == num_c_z - 1)
                    curr_c_type = SOLID;

                scene.cell_type[i * num_c_x + j * num_c_y + k] = curr_c_type;
            }
}


std::string inputVertShader()
{
    std::ifstream istr("v.glsl");
    std::string vert;
    std::stringstream vShaderStream;
    vShaderStream << istr.rdbuf();
    vert = vShaderStream.str();

    istr.close();
    return vert;
}

std::string inputFragFloorShader()
{
    std::ifstream istr("f_floor.glsl");
    std::string frag;
    std::stringstream fShaderStream;
    fShaderStream << istr.rdbuf();
    frag = fShaderStream.str();

    istr.close();
    return frag;
}

std::string inputFragPartShader()
{
    std::ifstream istr("f_particle.glsl");
    std::string frag;
    std::stringstream fShaderStream;
    fShaderStream << istr.rdbuf();
    frag = fShaderStream.str();

    istr.close();
    return frag;
}

GLuint getCompiledVShader()
{
    std::string vString = inputVertShader();
    const char* vertexShaderSource = vString.c_str();
    GLuint vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    return vertexShader;
}

GLuint getCompiledFFloorShader()
{
    std::string fString = inputFragFloorShader();
    const char* fragmentShaderSource = fString.c_str();
    GLuint fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    return fragmentShader;
}

GLuint getCompiledFPartShader()
{
    std::string fString = inputFragPartShader();
    const char* fragmentShaderSource = fString.c_str();
    GLuint fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    return fragmentShader;
}

GLuint createAndUseFloorShaderProg()
{
    GLuint vertexShader = getCompiledVShader();
    GLuint fragmentShader = getCompiledFFloorShader();

    GLuint shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

GLuint createAndUsePartShaderProg()
{
    GLuint vertexShader = getCompiledVShader();
    GLuint fragmentShader = getCompiledFPartShader();

    GLuint shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <glad/glad.h>


std::string inputVertPShader()
{
    std::ifstream istr("v.glsl");
    std::string vert;
    std::stringstream vShaderStream;
    vShaderStream << istr.rdbuf();
    vert = vShaderStream.str();

    istr.close();
    return vert;
}

std::string inputVertFShader()
{
    std::ifstream istr("v_floor.glsl");
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

GLuint getCompiledVPShader()
{
    std::string vString = inputVertPShader();
    const char* vertexShaderSource = vString.c_str();
    GLuint vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    return vertexShader;
}

GLuint getCompiledVFShader()
{
    std::string vString = inputVertFShader();
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

GLuint createAndLinkFloorShaderProg()
{
    GLuint vertexShader = getCompiledVFShader();
    GLuint fragmentShader = getCompiledFFloorShader();

    GLuint shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

GLuint createAndLinkParticleShaderProg()
{
    GLuint vertexShader = getCompiledVPShader();
    GLuint fragmentShader = getCompiledFPartShader();

    GLuint shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}
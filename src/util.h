#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <glad/glad.h>


std::string inputShader(std::string& path)
{
    std::ifstream istr(path);
    std::string shader;
    std::stringstream shaderStream;
    shaderStream << istr.rdbuf();
    shader = shaderStream.str();

    istr.close();
    return shader;
}

GLuint getCompiledVertShader(std::string& path)
{
    std::string vString = inputShader(path);
    const char* vertexShaderSource = vString.c_str();
    GLuint vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    return vertexShader;
}

GLuint getCompiledFragShader(std::string& path)
{
    std::string fString = inputShader(path);
    const char* fragmentShaderSource = fString.c_str();
    GLuint fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    return fragmentShader;
}

GLuint createAndLinkFloorShaderProg()
{
    std::string vert("v_pos.glsl");
    std::string frag("f_floor.glsl");
    GLuint vertexShader = getCompiledVertShader(vert);
    GLuint fragmentShader = getCompiledFragShader(frag);

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
    std::string vert("v_pos.glsl");
    std::string frag("f_particle.glsl");
    GLuint vertexShader = getCompiledVertShader(vert);
    GLuint fragmentShader = getCompiledFragShader(frag);

    GLuint shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

GLuint createAndLinkSurfaceShaderProg()
{
    std::string vert("v_pos_norm.glsl");
    std::string frag("f_surface.glsl");
    GLuint vertexShader = getCompiledVertShader(vert);
    GLuint fragmentShader = getCompiledFragShader(frag);

    GLuint shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}
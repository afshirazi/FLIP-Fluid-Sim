#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

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

std::string inputFragShader()
{
    std::ifstream istr("f.glsl");
    std::string frag;
    std::stringstream fShaderStream;
    fShaderStream << istr.rdbuf();
    frag = fShaderStream.str();

    istr.close();
    return frag;
}

unsigned int getCompiledVShader()
{
    std::string vString = inputVertShader();
    const char* vertexShaderSource = vString.c_str();
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    return vertexShader;
}

unsigned int getCompiledFShader()
{
    std::string fString = inputFragShader();
    const char* fragmentShaderSource = fString.c_str();
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    return fragmentShader;
}

unsigned int createAndUseShaderProgram()
{
    unsigned int vertexShader = getCompiledVShader();
    unsigned int fragmentShader = getCompiledFShader();

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}
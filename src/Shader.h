#pragma once

#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> // Incluye las funciones de OpenGL
#include <glm/glm.hpp> // Incluye GLM para las matrices
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    // ID del programa de shaders
    unsigned int ID;

    // Constructor: lee y construye los shaders
    Shader(const char* vertexPath, const char* fragmentPath);

    // Activa el shader
    void use();

    // Funciones de utilidad para uniformes
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    unsigned int getShaderProgramID() { return ID; }
    void setMat4(const std::string& name, const glm::mat4& mat) const;
};

#endif

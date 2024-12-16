#pragma once

#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> // Incluye las funciones de OpenGL
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
};

#endif

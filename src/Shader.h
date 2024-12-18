#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


// General purpose shader object. Compiles from file, generates
// compile/link-time error messages and hosts several utility 
// functions for easy management.
class Shader
{
public:
	// state
	unsigned int ID;
	// constructor
	Shader() { }
	// sets the current shader as active
	Shader& Use();
	// compiles the shader from given source code
	void    Compile(const char* vertexSource, const char* fragmentSource, const char* geometrySource = nullptr); // note: geometry source code is optional 
	// utility functions
	void    SetFloat(const char* name, float value, bool useShader = false);
	void    SetInteger(const char* name, int value, bool useShader = false);
	void    SetVector2f(const char* name, float x, float y, bool useShader = false);
	void    SetVector2f(const char* name, const glm::vec2& value, bool useShader = false);
	void    SetVector3f(const char* name, float x, float y, float z, bool useShader = false);
	void    SetVector3f(const char* name, const glm::vec3& value, bool useShader = false);
	void    SetVector4f(const char* name, float x, float y, float z, float w, bool useShader = false);
	void    SetVector4f(const char* name, const glm::vec4& value, bool useShader = false);
	void    SetMatrix4(const char* name, const glm::mat4& matrix, bool useShader = false);
private:
	// checks if compilation or linking failed and if so, print the error logs
	void    checkCompileErrors(unsigned int object, std::string type);
};

#endif
//#pragma once
//
//#ifndef SHADER_H
//#define SHADER_H
//
//#include <glad/glad.h> // Incluye las funciones de OpenGL
//#include <glm/glm.hpp> // Incluye GLM para las matrices
//#include <string>
//#include <fstream>
//#include <sstream>
//#include <iostream>
//
//class Shader
//{
//public:
//    // ID del programa de shaders
//    unsigned int ID;
//
//    // Constructor: lee y construye los shaders
//    Shader(const char* vertexPath, const char* fragmentPath);
//
//    // Activa el shader
//    void use();
//
//    // Funciones de utilidad para uniformes
//    void setBool(const std::string& name, bool value) const;
//    void setInt(const std::string& name, int value) const;
//    void setFloat(const std::string& name, float value) const;
//    unsigned int getShaderProgramID() { return ID; }
//    void setMat4(const std::string& name, const glm::mat4& mat) const;
//};
//
//#endif

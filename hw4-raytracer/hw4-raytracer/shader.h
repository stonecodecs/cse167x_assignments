#pragma once
#include <glad/glad.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

// Shader program that takes in vertex and fragment shaders
class Shader {
private:
	GLuint createShader(const std::string& shader_filepath, GLenum shader_type);
	GLuint createShaderProgram(GLuint vertex_shader, GLuint fragment_shader);
	bool in_use;

public:
	GLuint id;
	Shader(const char* vertex_filepath, const char* fragment_filepath);
	void use();
	void addUniform(std::string name);
	void addUniforms(std::vector<std::string> names);
	void remove();
};
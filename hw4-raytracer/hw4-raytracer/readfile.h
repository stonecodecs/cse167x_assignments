#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <deque>
#include <stack>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "transform.h" 
#include "window.h"

void rightmultiply(const glm::mat4& M, std::stack<glm::mat4>& transfstack);
bool readvals(std::stringstream& s, const int numvals, GLfloat* values);
Window* readfile(const char* filename); // scene to read into

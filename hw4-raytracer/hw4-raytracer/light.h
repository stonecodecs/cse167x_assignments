#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

enum LightType {
	DIRECTIONAL, 
	POINT
};

class Light {
public:
	LightType type;
	glm::vec3 posdir; // position if POINT, direction if DIRECTIONAL
	glm::vec3 rgb;
	
	Light(LightType t, glm::vec3 pos, glm::vec3 color) {
		type = t;
		posdir = pos;
		rgb = color;
	};
};
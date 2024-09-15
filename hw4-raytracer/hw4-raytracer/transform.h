// Transform header file to define the interface. 
// The class is all static for simplicity
// You need to implement left, up and lookAt
// Rotate is a helper function

// Include the helper glm library, including matrix transform extensions
#pragma once
#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// glm provides vector, matrix classes like glsl
// Typedefs to make code more readable 
const float pi = 3.14159265f; // For portability across platforms


class Transform
{
public:
	Transform();
	virtual ~Transform();
	static void left(float degrees, glm::vec3& eye, glm::vec3& up);
	static void up(float degrees, glm::vec3& eye, glm::vec3& up);
	static glm::mat4 lookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up);
	static glm::mat4 perspective(float fovy, float aspect, float zNear, float zFar);
	static glm::mat3 axis_rotation(const float degrees, const glm::vec3& axis);
	static glm::mat4 scale(const float& sx, const float& sy, const float& sz);
	static glm::mat4 translate(const float& tx, const float& ty, const float& tz);
	static glm::vec3 upvector(const glm::vec3& up, const glm::vec3& zvec);
};
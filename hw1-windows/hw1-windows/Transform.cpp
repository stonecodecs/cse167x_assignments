// Transform.cpp: implementation of the Transform class.

#include "Transform.h"

//Please implement the following functions:

// Helper rotation function.  
mat3 Transform::rotate(const float degrees, const vec3& axis) {
  // YOUR CODE FOR HW1 HERE
	mat3 identity = mat3(1.f);
	float rads = degrees * pi / 180;
	
	// need to transpose, since OpenGL uses column-major order
	mat3 skew = transpose(mat3(
		0.0f, -axis.z, axis.y, 
		axis.z, 0.0f, -axis.x, 
		-axis.y, axis.x, 0.0f
	));

	mat3 outer_product = transpose(mat3(
		axis.x * axis.x, axis.x * axis.y, axis.x * axis.z,
		axis.y * axis.x, axis.y * axis.y, axis.y * axis.z,
		axis.z * axis.x, axis.z * axis.y, axis.z * axis.z
	));

	mat3 rotMatrix = identity * cos(rads);
	rotMatrix += (1.0f - cos(rads)) * outer_product;
	rotMatrix += sin(rads) * skew;

  // You will change this return call
	return rotMatrix;
}

// Transforms the camera left around the "crystal ball" interface
void Transform::left(float degrees, vec3& eye, vec3& up) {
  // YOUR CODE FOR HW1 HERE
	eye = rotate(degrees, up) * eye;
}

// Transforms the camera up around the "crystal ball" interface
void Transform::up(float degrees, vec3& eye, vec3& up) {
  // YOUR CODE FOR HW1 HERE 
	// new coordinate system
	vec3 u = glm::normalize(glm::cross(up, eye));
	eye = rotate(-degrees, u) * eye;
	up = glm::normalize(glm::cross(eye, u)); // v -- always orthgonal to crystal ball
}

// Your implementation of the glm::lookAt matrix
mat4 Transform::lookAt(vec3 eye, vec3 up) {
  // YOUR CODE FOR HW1 HERE
	// create uvw coordinates, then auxiliary 4x4
	const vec3 center = vec3(0.f, 0.f, 0.f);
	vec3 w = glm::normalize(eye - center);
	vec3 u = glm::normalize(glm::cross(up, w));
	vec3 v = glm::cross(w, u);
	mat4 translation = mat4(1.0f);
	for (int i = 0; i < 3; i++) {
		translation[i][3] = -eye[i];
	}
	translation = transpose(translation);

	mat4 rotation = mat4(
		u.x, u.y, u.z, 0.f,
		v.x, v.y, v.z, 0.f,
		w.x, w.y, w.z, 0.f,
		0.f, 0.f, 0.f, 1.f
	);
	rotation = transpose(rotation);
  // You will change this return call
  return rotation * translation;
}

Transform::Transform()
{

}

Transform::~Transform()
{

}
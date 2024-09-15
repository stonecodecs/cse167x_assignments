#pragma once

#include <GLFW/glfw3.h>	
#include <glm/glm.hpp>	
#include <vector>

#include "object.h" // get Ray and Intersection
#include "transform.h"

#define GENERATE_GETTER_SETTER(type, name, proj) \
    type get_##name() const { return name; } \
    void set_##name(type value) { name = value; if (!proj) { update_view(); } else { update_projection(); }}
// proj(0) means affects the view matrix, otherwise the projection matrix

enum ProjectionType {
	PERSPECTIVE,
	ORTHOGRAPHIC
};

class Camera {
private:
	glm::mat4 view;
	glm::mat4 projection;
	glm::vec3 pos; // "eye"
	glm::vec3 up;
	glm::vec3 center; // "center" around object
	int width; // for now, locked into window size
	int height;
	float fov; // fovy
	float z_near;
	float z_far;

	void update_view();
	void update_projection();

public:
	ProjectionType projection_type;

	Camera(
		glm::vec3 pos,
		glm::vec3 up,
		glm::vec3 center,
		int w,
		int h,
		float fov = 90.0f, // in degrees for the Y component
		float z_n = 10.0f,
		float z_f = 100.0f,
		ProjectionType projection_type = PERSPECTIVE // for now, only give options for perspective and orthography
	);

	glm::mat4 vp(); // view * projection
	glm::mat4 view_matrix();

	float get_aspect_ratio();
	void rotate_left(float degrees);
	void rotate_up(float degrees);
	Ray ray_for_pixel(int i, int j);

	GENERATE_GETTER_SETTER(glm::vec3, pos, 0);
	GENERATE_GETTER_SETTER(glm::vec3, up, 0);
	GENERATE_GETTER_SETTER(glm::vec3, center, 0);
	GENERATE_GETTER_SETTER(int, width, 1);
	GENERATE_GETTER_SETTER(int, height, 1);
	GENERATE_GETTER_SETTER(float, fov, 1);
	GENERATE_GETTER_SETTER(float, z_near, 1);
	GENERATE_GETTER_SETTER(float, z_far, 1);

};
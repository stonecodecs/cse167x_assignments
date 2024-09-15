#include <iostream>

#include "camera.h"
#include "transform.h"

// EDIT LATER:
// Ray tracer implementation doesn't require perspective projection matrices
// nor view matrix

Camera::Camera(
	glm::vec3 pos,
	glm::vec3 up,
	glm::vec3 center,
	int w,
	int h,
	float fov,
	float z_n,
	float z_f,
	ProjectionType projection_type // not just char for extension purposes
) {
	this->pos = pos;
	this->up = up;
	this->center = center;
	this->width = w;
	this->height = h;
	this->fov = fov;
	this->z_near = z_n;
	this->z_far = z_f;
	this->projection_type = projection_type;
	
	if (projection_type == PERSPECTIVE){ // perspective
		this->projection = Transform::perspective(fov, w / h, z_n, z_f);
	}
	else if (projection_type == ORTHOGRAPHIC) { // ortho
		// TODO: CHANGE LATER TO ORTHOGRAPHY (right now work on others, default perspective)
		this->projection = Transform::perspective(fov, w / h, z_n, z_f);
	}
	else {
		std::cout << "Did not receive valid projection type string. Defaulting to perspective.";
		this->projection = Transform::perspective(fov, w / h, z_n, z_f);
	}

	this->view = Transform::lookAt(this->pos, this->center, this->up);
}


void Camera::update_view() {
	// update view matrix after changing parameters
	view = Transform::lookAt(pos, center, up);
}

void Camera::update_projection() {
	// update view matrix after changing parameters
	if (projection_type == PERSPECTIVE) {
		projection = Transform::perspective(fov, width / height, z_near, z_far);
	} else if (projection_type == ORTHOGRAPHIC) {
		projection = Transform::perspective(fov, width / height, z_near, z_far);
	}
	else {
		std::cout << "Tried to update projection matrix, but failed.";
	}
	// check type in calling function
}

float Camera::get_aspect_ratio() {
	return static_cast<float>(width) / static_cast<float>(height);
}

//glm::vec3 Camera::get_pos() {
//	return pos;
//}
//
//glm::vec3 Camera::get_up() {
//	return up;
//}
//
//glm::vec3 Camera::get_center() {
//	return center;
//}
//
//void Camera::set_pos(glm::vec3 newpos) {
//	pos = newpos;
//	update_view();
//}
//
//void Camera::set_up(glm::vec3 newup) {
//	up = newup;
//	update_view();
//}
//
//void Camera::set_center(glm::vec3 newcenter) {
//	center = newcenter;
//	update_view();
//}

glm::mat4 Camera::view_matrix() {
	return view; // for lights that don't use projection
}

glm::mat4 Camera::vp() {
	return view * projection;
}

void Camera::rotate_left(float degrees) {
	Transform::left(degrees, pos, up);
}

void Camera::rotate_up(float degrees) {
	Transform::up(degrees, pos, up);
}

Ray Camera::ray_for_pixel(int i, int j) {
	// generates a ray in the direction of the camera's (i,j)th pixel 
	glm::vec3 origin = this->pos;  // origin is camera location
	glm::vec3 dir; // to be found

	// construct new coordinate frame (same notation as in lecture)
	glm::vec3 w = glm::normalize(pos - center);
	glm::vec3 u = glm::normalize(glm::cross(up, w));
	glm::vec3 v = glm::cross(w, u); // already normalized

	float fovy = glm::radians(fov);

	float alpha = glm::tan(fovy / 2.0f) * ((j / (width / 2.0f)) - 1) * get_aspect_ratio(); // for fovx
	float beta  = glm::tan(fovy / 2.0f) * (1 - (i / (height / 2.0f)));

	dir = glm::normalize((alpha * u + beta * v - w));
	return Ray(origin, dir);
}
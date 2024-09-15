#include <iostream>

#include "scene.h"

///* SCENE *///

Scene::Scene(int sens) {
	cam_sensitivity = sens;
	max_depth = 5; // default 3
}

Scene::Scene(Camera* cam, int sens) {
	// every scene needs a camera for display
	register_camera(cam, true);
	cam_sensitivity = sens;
	max_depth = 5; // default
}

Scene::~Scene() {
	for (int i = 0; i < objects.size(); i++) {
		delete objects[i];
	}
	for (int i = 0; i < cameras.size(); i++) {
		delete cameras[i];
	}
}

Camera* Scene::get_main_camera() {
	if (cameras.size() == 0) {
		return nullptr;
	}
	return cameras[main_cam];
}

Camera* Scene::get_camera(int cam_idx) {
	if (cam_idx >= cameras.size()) {
		return nullptr;
	}
	return cameras[cam_idx];
}

bool Scene::register_camera(Camera* cam, bool make_current) {
	if (cam == nullptr) {
		return false;
	}
	int size_before = cameras.size(); // acts as new camera ID
	cameras.push_back(cam);
	if (make_current || size_before == 0) { // if make main, or no camera in scene yet
		set_current_camera(size_before); // set as main
		main_cam = size_before;
	}
	return true;
}

bool Scene::set_current_camera(int idx) {
	if (idx < cameras.size()) {
		main_cam = idx;
		return true;
	}
	return false; // and don't change
}

// adds object to "node"
void Scene::register_object(Object* obj) {
	if (obj != nullptr) {
		objects.push_back(obj);
	}
}

void Scene::register_light(Light* light) {
	if (light != nullptr) {
		lights.push_back(light);
	}
}

void Scene::transform_sensitivity_up() {
	if (cam_sensitivity >= 1) {
		set_sensitivity(cam_sensitivity + 1);
	}
}
void Scene::transform_sensitivity_down() {
	if (cam_sensitivity > 1) { // cannot go lower than 1
		set_sensitivity(cam_sensitivity - 1);
	}
}

void Scene::set_sensitivity(int sens_to) {
	cam_sensitivity = sens_to;
}

int Scene::get_sensitivity() {
	return cam_sensitivity;
}

void Scene::set_maxdepth(int depth) {
	if (depth > 0) {
		max_depth = depth;
	}
	else {
		std::cout << "Depth not changed. Argument needs to be a positive integer.\n";
	}
}

void Scene::set_transform_type(TransformType t) {
	transop = t;
}

TransformType Scene::get_transform_type() {
	return transop;
}

// using 2D image here, but flatten in main.cpp
RGBImage Scene::raytrace() {
	// Compute frame, load textures
	Camera* cam = get_main_camera();
	RGBImage texture(cam->get_height(), std::vector<glm::vec3>(cam->get_width()));

	for (int i = 0; i < cam->get_height(); i++) {
		for (int j = 0; j < cam->get_width(); j++) {
			// for all pixels
			Ray ray = cam->ray_for_pixel(i, j); // generate ray
			Intersection hit = closest_intersection(ray);  // get closest hit (if any)
			if (hit.hit_obj != nullptr) {
				texture[i][j] = color_at(hit); // hit: color with object properties
			} else {
				texture[i][j] = glm::vec3(0.0f); // black; no hit
			}
		}
	}

	// for each pixel, compute intersections, then compute colors

	return texture;
}

Intersection Scene::closest_intersection(Ray& ray) {
	return bvh->locate(ray);
}

glm::vec3 Scene::color_at(Intersection& inter) {
	// assumes hit is NOT NULL already
	glm::vec3 ambient = inter.hit_obj->get_ambient();
	glm::vec3 diffuse = inter.hit_obj->get_diffuse();
	glm::vec3 specular = inter.hit_obj->get_specular();
	glm::vec3 emission = inter.hit_obj->get_emission();
	float shininess = inter.hit_obj->get_shininess();

	// Compute the halfway vector between the light direction and the view direction

	Camera* cam = get_main_camera();
	glm::vec3 final_color = ambient + emission;
	// std::cout << "FINAL_LIGHT: " << final_color.x << ", " << final_color.y << ", " << final_color.z << "\n";

	// compute intersection color using all lights 
	for (const Light* light : lights) {
		glm::vec3 light_dir(0.0f);

		if (light->type == POINT) {
			light_dir = glm::normalize(light->posdir - inter.hit); // normal must be reverrsed
		}
		else if (light->type == DIRECTIONAL) {
			light_dir = glm::normalize(light->posdir);
		}

		glm::vec3 view_dir = glm::normalize(cam->get_pos() - inter.hit);
		glm::vec3 halfvec = glm::normalize(light_dir + view_dir);

		Ray shadow_ray(inter.hit + (light_dir * 1e-6f), light_dir); // hit + [small step] for numerical stability
		Intersection shadow_inter = closest_intersection(shadow_ray);
		
		if (shadow_inter.hit_obj == nullptr) { // not shadow
			glm::vec3 light_attribution = compute_color(
				light_dir,
				light->rgb,
				inter.normal,
				halfvec,
				diffuse,
				specular,
				shininess
			);

			final_color += light_attribution;
		}
	}

	return final_color * 255.0f;
}

glm::vec3 Scene::compute_color(
	glm::vec3 dir,
	glm::vec3 lightcolor,
	glm::vec3 normal,
	glm::vec3 halfvec,
	glm::vec3 _diffuse,
	glm::vec3 _specular,
	float _shininess)
{
	float n_dot_L = glm::dot(normal, dir);
	// std::cout << "n*L=" << n_dot_L << "\n";
	glm::vec3 lambert = _diffuse * lightcolor * std::max(n_dot_L, 0.0f);

	// add specular highlight
	float n_dot_H = glm::dot(normal, halfvec);
	// std::cout << "n*H=" << n_dot_H << "\n";
	glm::vec3 phong = _specular * lightcolor * std::pow(std::max(n_dot_H, 0.0f), _shininess);

	glm::vec3 ret = lambert + phong;
	return ret;
}

// best to call this when all objects are read 
void Scene::construct_bvh() {
	// create full bvh based on the objects that we currently have
	if (objects.size() < 1) {
		bvh = nullptr;
		return; // nothing to process
	}
	
	bvh = new BVH<Object*>(objects);
}
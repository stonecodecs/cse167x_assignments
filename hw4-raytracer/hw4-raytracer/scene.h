#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>	
#include <glm/glm.hpp>	
#include <vector>
#include <stack>
#include "object.h"
#include "light.h"
#include "camera.h"
#include "enums.h"
#include "bvh.h"

typedef void (*DisplayFunc)();
typedef std::vector<std::vector<glm::vec3>> RGBImage; // the image/frame to be displayed

enum TransformType {
	ROTATE,
	TRANSLATE,
	SCALE
};

class Scene {
private:
	BVH<Object*>* bvh = NULL;
	std::vector<Object*> objects; // collect all objects, but then use BVH after processing
	std::vector<Light*>   lights;
	std::vector<Camera*> cameras;
	int main_cam; // index of the current camera to view (for potential extension, but will only use one camera)
	int cam_sensitivity; // rather than have this intrinsic to the camera, do for all
	int max_depth;
	TransformType transop = ROTATE;
	glm::vec3 compute_color(
		glm::vec3 lightdir,
		glm::vec3 lightcolor,
		glm::vec3 normal,
		glm::vec3 halfvec,
		glm::vec3 diffuse,
		glm::vec3 specular,
		float shininess
	);

public:
	Scene(int sens = 5);
	Scene(Camera* cam, int sens = 5);
	~Scene();

	Camera* get_main_camera();
	Camera* get_camera(int cam_idx); // possible extension; not implemented
	bool register_camera(Camera* cam, bool make_current = true);
	void register_object(Object* obj);
	void register_light(Light* light);
	bool set_current_camera(int idx);
	void transform_sensitivity_up();
	void transform_sensitivity_down();
	void set_sensitivity(int sens_to);
	int get_sensitivity();
	void set_maxdepth(int d);
	void set_transform_type(TransformType t);
	TransformType get_transform_type();
	RGBImage raytrace();
	Intersection closest_intersection(Ray& ray);
	glm::vec3 color_at(Intersection& hit);
	void construct_bvh();
	void print_bvh() {
		bvh->display();
	};
	int how_many_objects() {
		return static_cast<int>(objects.size());
	};
};
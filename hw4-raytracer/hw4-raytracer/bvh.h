#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <algorithm>
#include <iostream>
#include "enums.h"
#include "ray.h"

template <typename T>
class BVH;

template <typename T>
class BVHNode;

struct BoundingBox { // axis-aligned bounding box
	glm::vec3 c1; // corner1 
	glm::vec3 c2; // corner2

	BoundingBox() : c1(glm::vec3(0.0)), c2(glm::vec3(0.0)) {};
	BoundingBox(const glm::vec3 corner1, const glm::vec3 corner2) : c1(corner1), c2(corner2) {};
	glm::vec3 centroid() const {
		return (c1 + c2) / 2.0f;
	}

	float surface_area() const { // of the bounding box, rather than the actual mesh
		glm::vec3 s = c2 - c1; // sides: (w, h, l)
		return 2.0f * (s.x * s.y + s.x * s.z + s.y * s.z);
	}
};

// Bounding Volume Hiearchies, using SAH to split
template <typename T>
class BVH {
private: 
	BVHNode<T>* root;
	BVHNode<T>* split_nodes(BVHNode<T>* r, std::vector<BVHNode<T>*> leaf_nodes); // splits root

	// Methods needed for SAH:
	float SAH_cost(std::vector<float> cumulative_sa, int prims_on_left); // helper function for constructor
	int* min_SAH_params(std::vector<BVHNode<T>*> temp); // returns (best split axis, split position)
	void _display(BVHNode<T>* r);
	Intersection _locate(BVHNode<T>* node, Ray& r);
	// helper functions
	glm::vec3 vec3_get_extremes(const glm::vec3& a, const glm::vec3& b, bool max) {
		return (max) ?
			glm::vec3(
				std::max(a.x, b.x),
				std::max(a.y, b.y),
				std::max(a.z, b.z)
			) : glm::vec3(
				std::min(a.x, b.x),
				std::min(a.y, b.y),
				std::min(a.z, b.z)
			);
	};
	bool centroid_sort(BoundingBox const& b1, BoundingBox const& b2, int axis) {
		glm::vec3 center1 = (b1.c1 + b1.c2) / 2.0f;
		glm::vec3 center2 = (b2.c1 + b2.c2) / 2.0f;

		return center1[axis] < center2[axis];
	};
	void _cleanup(BVHNode<T>* r);
	
public:
	BVH<T>(std::vector<T> objects); // get objects from Scene
	~BVH();
	Intersection locate(Ray& ray); // traversal func
	void cleanup();
	void display(); // for debugging
};

// Bounding Volume Hierarchies acceleration structure
template <typename T>
class BVHNode {
public:
	BoundingBox box;  // Axis-Aligned Bounding Box (or any other bounding volume)
	BVHNode<T>* left;
	BVHNode<T>* right;
	// std::vector<BVHNode<T>*> children; // leaf node if children == 0 and obj is not NULL
	T obj;	   // Only used in leaf nodes (otherwise, NULL)
	int held_objects; // how many objects are held below this

	// min/max coordinates to bounding boxes; if 'corners' is true, then xyz are corner1 and corner2 instead
	BVHNode<T>(int total_objs);
	BVHNode<T>(glm::vec3 min_xyz, glm::vec3 max_xyz, T obj = NULL);
	bool is_leaf_node() const;
	bool check_intersection(Ray& r);
};



////////////////////////////// BVH main functions ////////////////////////////////



template <typename T>
BVH<T>::BVH(std::vector<T> objects) { // objects can be of Object class, or Triangles
	if (objects.size() < 1) { // if no objects, BVH is NULL
		root = nullptr;
		return;
	}

	// for every object, wrap with a BVHNode (leaf-node level)
	std::vector<BVHNode<T>*> temp{};
	for (int i = 0; i < objects.size(); i++) {
		ObjectType objtype = objects[i]->get_type();

		if (objtype == SPHERE || objtype == TRIANGLE) {
			// every bounding box is just the center of the sphere +/- radius
			BVHNode<T>* new_node = new BVHNode<T>(
				objects[i]->get_xyz_extrema(false), // minimum
				objects[i]->get_xyz_extrema(true),  // maximum
				objects[i]
			);

			temp.push_back(new_node);
		}
		else {
			std::cout << "BVH::BVH::Constructor only allows objects of type SPHERE or TRIANGLE.\n";
			for (auto& leaf : temp) { // release new_nodes in this degenerate case
				delete leaf;
			}
			return;
		}
	}

	// with all objects wrapped in Nodes, create the binary tree/BVH starting from root
	root = split_nodes(root, temp); // recursive call that creates the entire BVH
}

template <typename T>
float BVH<T>::SAH_cost(std::vector<float> cumulative_sa, int prims_on_left) {
	// cost function (of surface area)
	// prims_on_left represents (inclusively) the number of primitives on the (to-be) left child
	int total_size = static_cast<int>(cumulative_sa.size());

	if (total_size - 1 < prims_on_left) { return -1.0f; }
	if (total_size < 1) { return 0.0f; }
	if (total_size == 1) { return cumulative_sa[0]; }

	float SA_left = cumulative_sa[prims_on_left];
	float SA_right = cumulative_sa[total_size - 1] - cumulative_sa[prims_on_left];

	return (prims_on_left + 1) * SA_left + (total_size - prims_on_left) * SA_right;
}

template <typename T>
int* BVH<T>::min_SAH_params(std::vector<BVHNode<T>*> temp) {
	int params[2]{ 0, 0 };
	float min_SAH = (float)INT_MAX;
	int best_axis = 0; // 0: X, 1: Y, 2: Z
	int split_pos = 0;

	if (temp.size() <= 1) { // nothing to be done if <1 objects
		return params;
	}

	// for every node, get the smallest SAH score axis
	for (int axis = 0; axis < 3; axis++) { // (x, y, z)
		// sort based on axes
		std::sort(temp.begin(), temp.end(),
			[axis](const BVHNode<T>* a, const BVHNode<T>* b) {
				// take centroids
				glm::vec3 center1 = a->box.centroid();
				glm::vec3 center2 = b->box.centroid();
				return center1[axis] < center2[axis];
			});

		std::vector<float> surface_areas{};

		// run a loop taking the cumulative sum of surface areas needed for SAH
		for (int i = 0; i < temp.size(); i++) {
			float current_SA = temp[i]->box.surface_area(); // current object SA
			if (i == 0) { // if first object checked
				surface_areas.push_back(current_SA);
				continue;
			}
			// add with previous cumulative SA sum
			surface_areas.push_back(current_SA + surface_areas[surface_areas.size() - 1]);
		}

		// compute minimum SAH (cost function)
		// why temp.size() - 1: avoid the (all vs none) case
		for (int i = 0; i < temp.size(); i++) {
			float cost = SAH_cost(surface_areas, i);
			if (cost == -1) {
				continue; // error case; should not happen
			}
			if (cost < min_SAH) {
				// update minimum SAH + best axis (x,y,z)
				min_SAH = cost;
				best_axis = axis;
				split_pos = i;
				std::cout << "[axis: " << best_axis << ", splitidx: " << split_pos << "]new lowest cost : " << cost << '\n';
			}
		}
	}
	params[0] = best_axis;
	params[1] = split_pos;

	return params;
}

template <typename T>
Intersection BVH<T>::locate(Ray& ray) {
	return _locate(this->root, ray);
}

template <typename T>
Intersection BVH<T>::_locate(BVHNode<T>* node, Ray& ray) {
	bool is_intersecting = node->check_intersection(ray);

	if (is_intersecting) {
		if (node->is_leaf_node()) { // if leaf node
			return node->obj->check_hit(ray); // object T needs to have CHECK_HIT function
		}
		else { // if intermediate
			Intersection left_inter  = _locate(node->left, ray);
			Intersection right_inter = _locate(node->right, ray);

			// handle by cases
			if (left_inter.hit_obj == nullptr && right_inter.hit_obj == nullptr) { // no hits
				return NoIntersection;
			}
			else if (left_inter.hit_obj != nullptr && right_inter.hit_obj != nullptr) { // both hit
				return (left_inter.distance <= right_inter.distance) ? left_inter : right_inter;
			}
			else if (left_inter.hit_obj != nullptr) {
				return left_inter;
			}
			else {
				return right_inter;
			}
		}
	}
	return NoIntersection; // ray.origin is dummy data, no intersection
}

template <typename T>
BVHNode<T>* BVH<T>::split_nodes(BVHNode<T>* r, std::vector<BVHNode<T>*> leaf) {
	if (leaf.size() < 1) { // if no BVHNodes, end here
		return NULL;
	}
	if (leaf.size() == 1) { // if leaf node
		r = leaf[0];
		return r;
	}

	// otherwise, create a new intermediate node here
	r = new BVHNode<T>(static_cast<int> (leaf.size())); // this constructor keeps track of held objects

	// compute best SAH for current leaves, then get best splitting parameters (axis, pos)
	int* params = min_SAH_params(leaf);
	int best_axis = params[0];
	int split_pos = params[1];

	// sort by best axis again to make sure split objects are aligned
	std::sort(leaf.begin(), leaf.end(), [best_axis](const BVHNode<T>* a, const BVHNode<T>* b) {
		// take centroids
		glm::vec3 center1 = a->box.centroid();
		glm::vec3 center2 = b->box.centroid();
		return center1[best_axis] < center2[best_axis];
		});

	// once we have the minimum SAH score + parameters, we can actually split
	std::vector<BVHNode<T>*> left_nodes(leaf.begin(), leaf.begin() + split_pos + 1); // left leaf objects
	std::vector<BVHNode<T>*> right_nodes(leaf.begin() + split_pos + 1, leaf.end()); // right leaf objects

	// recursively create new nodes for split objects
	r->left = split_nodes(r->left, left_nodes);
	std::cout << "left done, " << r->left << '\n';
	r->right = split_nodes(r->right, right_nodes);
	std::cout << "right done, " << r->right << '\n';

	// once we've computed left and right subtrees, fill in data for THIS node
		// - calculate parent (this) bounding box

	if (r->left != NULL && r->right != NULL) {
		r->box.c1 = vec3_get_extremes(r->left->box.c1, r->right->box.c1, false); // minimum
		r->box.c2 = vec3_get_extremes(r->left->box.c2, r->right->box.c2, true);  // maximum
	}
	else {
		std::cout << "BVH::ERROR: split_nodes has only ONE child; this should not be possible.\n";
	}

	return r;
}

template <typename T>
BVHNode<T>::BVHNode(int total_objs) {
	left = nullptr;
	right = nullptr;
	obj = nullptr;
	held_objects = total_objs;
}

// for leaf node
template <typename T>
BVHNode<T>::BVHNode(glm::vec3 min_xyz, glm::vec3 max_xyz, T obj) {
	BoundingBox b(min_xyz, max_xyz); // corner1, corner2
	this->box = b;
	this->obj = obj;
	left = nullptr;
	right = nullptr;
	held_objects = 1;
}

template <typename T>
bool BVHNode<T>::is_leaf_node() const {
	if (left == nullptr && right == nullptr) {
		return true;
	}
	return false;
}

template <typename T> 
bool BVHNode<T>::check_intersection(Ray& r) {
	// Initialize tmin and tmax to very large and very small values respectively
	float tmin = 0.0f;
	float tmax = INT_MAX;

	// Iterate over the 3 axes (x, y, z)
	for (int i = 0; i < 3; i++) {
		float invD = 1.0f / r.direction[i];
		float t0 = (this->box.c1[i] - r.origin[i]) * invD;
		float t1 = (this->box.c2[i] - r.origin[i]) * invD;

		// Swap t0 and t1 if the ray is traveling in the negative direction of the axis
		if (invD < 0.0f) std::swap(t0, t1);

		// Update tmin and tmax to account for the new intersections
		tmin = std::max(tmin, t0);
		tmax = std::min(tmax, t1);

		// If at any point tmin exceeds tmax, the ray does not intersect the box
		if (tmax < tmin) {
			return false;
		}
	}

	return true;
}

template <typename T>
void BVH<T>::display() {
	if (this->root == nullptr) {
		std::cout << "BVH is empty.\n";
		return;
	}
	_display(root);
}

template <typename T>
void BVH<T>::_display(BVHNode<T>* r) {
	if (r == nullptr) {
		return;
	}

	glm::vec3 center = r->box.centroid();

	// Print the current node's value
	std::cout << "[count below:" << r->held_objects << "]address: " << r << " || : ";
	std::cout << r->box.surface_area() << "\n"; // check that parent boxes are bigger

	// Traverse left subtree
	_display(r->left);

	// Traverse right subtree
	_display(r->right);
}

template <typename T>
void BVH<T>::cleanup() {
	_cleanup(this->root);
}

template <typename T>
void BVH<T>::_cleanup(BVHNode<T>* r) {
	_display(r->left);
	_display(r->right);

	// delete from bottom up
	delete r;
}

template <typename T>
BVH<T>::~BVH() {
	cleanup();
}
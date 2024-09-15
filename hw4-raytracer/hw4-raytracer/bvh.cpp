#include "bvh.h"
#include "object.h"

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
}

// custom sort function for BVH splitting coordinates (using centroids)
bool centroid_sort(BoundingBox const& b1, BoundingBox const& b2, int axis) {
	glm::vec3 center1 = (b1.c1 + b1.c2) / 2.0f;
	glm::vec3 center2 = (b2.c1 + b2.c2) / 2.0f;

	return center1[axis] < center2[axis];
}

void printVec3(const glm::vec3& vector) {
	std::cout << " vec3(" << vector.x << ", " << vector.y << ", " << vector.z << ")" << '\n';
}

// BVH main functions
template <typename T>
BVH<T>::BVH(std::vector<T> objects) { // objects can be of Object class, or Triangles
	if (objects.size() < 1) { // if no objects, BVH is NULL
		root = NULL;
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
	left = NULL;
	right = NULL;
	obj = NULL;
	held_objects = total_objs;
}

// for leaf node
template <typename T>
BVHNode<T>::BVHNode(glm::vec3 min_xyz, glm::vec3 max_xyz, T obj) {
	BoundingBox b(min_xyz, max_xyz); // corner1, corner2
	this->box = b;
	this->obj = obj;
	left = NULL;
	right = NULL;
	held_objects = 1;
}

template <typename T>
bool BVHNode<T>::is_leaf_node() const {
	if (left == NULL && right == NULL) {
		return true;
	}
	return false;
}

template <typename T>
void BVH<T>::display() {
	if (this->root == NULL) {
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
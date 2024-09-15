#pragma once

#include <glm/glm.hpp>

class Object;

class Intersection {
public:
    float distance = -1.0f;
    glm::vec3 hit;
    glm::vec3 normal; // useful parameter (ignore if hit_obj is NULL)
    Object* hit_obj; // no-hit indicator by default (NULL)

    Intersection(glm::vec3 hit_, glm::vec3 normal_ = glm::vec3(0.0f), Object* obj = nullptr) : hit(hit_), normal(normal_), hit_obj(obj) {};
};

// for convenience
inline Intersection NoIntersection(glm::vec3(0.0f), glm::vec3(0.0f), nullptr);

class Ray {
public:
    glm::vec3 origin;
    glm::vec3 direction;

    Ray(glm::vec3 orig, glm::vec3 dir) : origin(orig), direction(dir) {};
};
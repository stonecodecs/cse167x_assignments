#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include "bvh.h"
#include "ray.h"

class Object;
class Triangle;
class Mesh;

class Object {
protected:
    ObjectType type;
    glm::mat4 transform;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 emission;
    float shininess;
    
public:
    Object(
        ObjectType type,
        glm::mat4 transform,
        glm::vec3 ambient,
        glm::vec3 diffuse,
        glm::vec3 specular,
        glm::vec3 emission,
        float shininess
    ) : type(type), transform(transform), ambient(ambient), diffuse(diffuse),
        specular(specular), emission(emission), shininess(shininess) {};
    ObjectType get_type() { return type; }
    glm::mat4 get_transform() { return transform; }
    glm::vec3 get_ambient() { return ambient; }
    glm::vec3 get_diffuse() { return diffuse; }
    glm::vec3 get_specular() { return specular; }
    glm::vec3 get_emission() { return emission; }
    float get_shininess() { return shininess; }

    virtual Intersection check_hit(Ray& r) = 0; // different intersection algorithms for each object type
    virtual glm::vec3 get_xyz_extrema(bool maximum) = 0; // for bounding boxes
};

class Triangle : public Object {
private:
    // we assume that all Triangles generated are part of a Mesh
    // pointer saves memory with "global-to-triangle" properties though may be bad practice
    Mesh* parent_mesh = nullptr;
    glm::vec3 idx; // vertex component indices

public:
    Triangle(
        glm::vec3 idx_,
        Mesh* parent,
        ObjectType type,
        glm::mat4 transform,
        glm::vec3 ambient,
        glm::vec3 diffuse,
        glm::vec3 specular,
        glm::vec3 emission,
        float shininess
    );

    Intersection check_hit(Ray& r);
    glm::vec3 get_xyz_extrema(bool maximum);
    void assign_parent(Mesh* p);
    glm::vec3 get_vertex(int v); // returns the vertex 'v' (0,1,2)
};

// Mesh is made of triangle components, therefore type==TRIANGLE
class Mesh : public Object {
private:
    BVH<Triangle*>* bvh;
    std::vector<glm::vec3> vertices; // set of all triangle primitive vertex components
    std::vector<Triangle*> triangles; // made of indices of vertices

public:
    Mesh(
        std::vector<glm::vec3> vertices,
        std::vector<Triangle*> triangles,
        glm::mat4 transform,
        glm::vec3 ambient,
        glm::vec3 diffuse,
        glm::vec3 specular,
        glm::vec3 emission,
        float shininess
    );

    ~Mesh() {
        for (auto& tri : triangles) {
            delete tri;
        }
        delete bvh;
    }
    
    Intersection check_hit(Ray& r);
    glm::vec3 get_xyz_extrema(bool maximum);
    std::vector<glm::vec3>* get_vertices() { return &vertices; }
};


class Sphere : public Object {
private:
    float radius;

public:
    Sphere(
        float radius,
        glm::mat4 transform,
        glm::vec3 ambient,
        glm::vec3 diffuse,
        glm::vec3 specular,
        glm::vec3 emission,
        float shininess
    ) : Object(ObjectType::SPHERE, transform, ambient, diffuse, specular, emission, shininess),
        radius(radius) {};

    float get_radius() { return radius; }
    glm::vec3 get_center() { return glm::vec3(transform[3][0], transform[3][1], transform[3][2]); }
    Intersection check_hit(Ray& r);
    glm::vec3 get_xyz_extrema(bool maximum);
};
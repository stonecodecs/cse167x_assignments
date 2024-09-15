#include <cmath>
#include <iostream>
#include "object.h"

//Object::Object(
//    ObjectType type,
//    glm::mat4 transform,
//    glm::vec3 ambient,
//    glm::vec3 diffuse,
//    glm::vec3 specular,
//    glm::vec3 emission,
//    float shininess
//) {
//    this->type = type;
//    this->transform = transform;
//    this->ambient = ambient;
//    this->diffuse = diffuse;
//    this->specular = specular;
//    this->emission = emission;
//    this->shininess = shininess;
//}

Intersection Triangle::check_hit(Ray& r) {
    // vertices of triangle
    // (already transformed)
    glm::vec3 a = get_vertex(0);
    glm::vec3 b = get_vertex(1);
    glm::vec3 c = get_vertex(2);

    // check where ray intersects triangle plane
    glm::vec3 tri_normal = glm::normalize(glm::cross(b - a, c - a));
    float denom = glm::dot(r.direction, tri_normal);
    // std::cout << "\ninside_checkhit_denom" << denom << "\n";
    if (denom == 0.0f) { // check if parallel
        return NoIntersection; // no hit
    }

    float t = glm::dot(a - r.origin, tri_normal) / denom; // finds "scale" 
    
    // std::cout << "\ninside_checkhit" << t << "\n";
    if (t < 0) { // if t negative, then triangle is behind ray origin
        return NoIntersection; // no hit
    }
    // otherwise, record the point of intersection
    Intersection inter(r.origin + t * r.direction);
    
    // use intersection point and check that it's within the triangle
    // using inside-outside test
    glm::vec3 ab = b - a;
    glm::vec3 bc = c - b;
    glm::vec3 ca = a - c;

    glm::vec3 a_int = inter.hit - a; // vector between A and plane intersection point
    glm::vec3 b_int = inter.hit - b;
    glm::vec3 c_int = inter.hit - c;

    // for each edge, the intersection point must be to the LEFT of each edge
    if (glm::dot(tri_normal, glm::cross(ab, a_int)) > -1e-6f &&
        glm::dot(tri_normal, glm::cross(bc, b_int)) > -1e-6f &&
        glm::dot(tri_normal, glm::cross(ca, c_int)) > -1e-6f) {

        // std::cout << "INTERSECTION AT(" << inter.hit.x << ", " << inter.hit.y << ", " << inter.hit.z << "\n";

        inter.distance = t;
        inter.hit_obj = this;
        inter.normal = tri_normal;
        // std::cout << "\nhit " << inter.distance << "\n";
        return inter;
    }

    // otherwise, not in triangle
    return NoIntersection;
}

Intersection Mesh::check_hit(Ray& r) {
    // relies on Triangle::check_hit (from bvh)
    // check bvh to find the triangle primitive intersecting with the ray
    return this->bvh->locate(r);
}

Intersection Sphere::check_hit(Ray& ray) {
    // since sphere, extend to ellipse case using the inverse of transform
    // 0.0f since already transformed using get_center() called below;
    glm::vec3 new_orig = glm::inverse(transform) * glm::vec4(ray.origin, 0.0f);
    glm::vec3 new_dir  = glm::inverse(transform) * glm::vec4(ray.direction, 0.0f);
    Ray r(new_orig, glm::normalize(new_dir)); 
    glm::vec3 world_hit(0.0f); // going back to world coordinates for ellipse
    
    // solve quadratic equation for 't': 
    // (P1 dot P1)t^2 + 2*(P1 dot (P0 - C))t + (P0 - C) dot (P0 - C) - r^2 = 0
    glm::vec3 p0 = r.origin;
    glm::vec3 p1 = r.direction;
    glm::vec3 center = transform * glm::vec4(this->get_center(), 0.0f);


    // in quadratic notation
    float a = glm::dot(p1, p1);
    float b = 2 * glm::dot(p1, (p0 - center));
    float c = glm::dot((p0 - center), (p0 - center)) - (glm::pow(radius, 2));
    
    // look at determinant first
    float det = glm::sqrt(glm::pow(b, 2) - (4.0f * a * c));
    Intersection hitobj(p0);

    if (std::isnan(det) || det < 0.0f) { // no real root case
        // std::cout << "NULLHITdet: " << det << ".\n";

        return NoIntersection; // NULL hit
    }
    else if (det == 0.0f) { // only one intersection (tangent case)
        // tangent + positive/negative case
        float root1 = (-b + det) / (2.0f * a);
        float root2 = (-b - det) / (2.0f * a);
        // std::cout << "ONEHITdet: " << det << ".\n";

        if (root1 == root2) { // unique root
            world_hit = transform * glm::vec4(p0 + (p1 * root1), 1.0f);
            hitobj.hit = world_hit;
            hitobj.distance = root1;
        }
        else {
            world_hit = transform * glm::vec4((std::isnan(root1)) ? p0 + (p1 * root2) : p0 + (p1 * root1), 1.0f);
            hitobj.hit = world_hit;
            hitobj.distance = (std::isnan(root1)) ? root2 : root1;
        }
    }
    else { // two intersections (det > 0)
        float root1 = (-b + det) / (2.0f * a);
        float root2 = (-b - det) / (2.0f * a);
        // std::cout << "TWOHITdet: " << det << ".\n";

        world_hit = transform * glm::vec4(p0 + (p1 * std::min(root1, root2)), 1.0f);
        hitobj.hit = world_hit; // take the one nearest to us
        hitobj.distance = std::min(root2, root1);
    }

    hitobj.hit_obj = this; // assign object hit 
    glm::vec3 normal = glm::normalize(hitobj.hit - center);
    hitobj.normal = glm::normalize(glm::vec3(glm::transpose(glm::inverse(transform)) * glm::vec4(normal, 0.0f)));

    //std::cout<< "worldhit: " << world_hit.x << ", " << world_hit.y << ", " << world_hit.z << "\n";

    return hitobj;
}


glm::vec3 Mesh::get_xyz_extrema(bool maximum) {
    // go through every vertex, find maximum/minimum
    glm::vec3 obj_xyz(maximum ? INT_MIN : INT_MAX); // init
    for (int i = 0; i < vertices.size(); i++) {
        if (maximum) {
            obj_xyz = glm::max(glm::vec3(transform * glm::vec4(vertices[i], 1.0f)), obj_xyz);
        }
        else {
            obj_xyz = glm::min(glm::vec3(transform * glm::vec4(vertices[i], 1.0f)), obj_xyz);
        }
    }
    std::cout << "MESH::getexetrema objxyz: (" << obj_xyz.x << ", " << obj_xyz.y << ", " << obj_xyz.z << ")\n.";

    return obj_xyz;
}

// recheck implementation
glm::vec3 Sphere::get_xyz_extrema(bool maximum) {
    // for SPHERE, just return the radius distance from center
    glm::vec4 center = glm::vec4(glm::vec3(transform[3]), 0.0f); // not yet world coords
    glm::vec3 obj_xyz = glm::vec3(transform * center); // get world coordinate xyz position

    float scale_x = transform[0][0];
    float scale_y = transform[1][1];
    float scale_z = transform[2][2];

    glm::vec3 scales(scale_x, scale_y, scale_z);
    return (maximum) ? obj_xyz + (radius * scales) : obj_xyz - (radius * scales);
}

glm::vec3 Triangle::get_xyz_extrema(bool maximum) {
    glm::vec3 tri_xyz(maximum ? INT_MIN : INT_MAX); // init
    std::vector<glm::vec3>* vertices = parent_mesh->get_vertices();

    glm::vec3 a = transform * glm::vec4((*vertices).at(static_cast<int>(idx.x)), 1.0f);
    glm::vec3 b = transform * glm::vec4((*vertices).at(static_cast<int>(idx.y)), 1.0f);
    glm::vec3 c = transform * glm::vec4((*vertices).at(static_cast<int>(idx.z)), 1.0f);

    if (maximum) {
        tri_xyz = glm::max(a, tri_xyz);
        tri_xyz = glm::max(b, tri_xyz);
        tri_xyz = glm::max(c, tri_xyz);
    }
    else {
        tri_xyz = glm::min(a, tri_xyz);
        tri_xyz = glm::min(b, tri_xyz);
        tri_xyz = glm::min(c, tri_xyz);
    }

    std::cout << "MESH::getexetrema trixyz: (" << tri_xyz.x << ", " << tri_xyz.y << ", " << tri_xyz.z << ")\n.";

    return tri_xyz;
}

// AMBIENT, DIFFUSE, ETC. PARAMETERS ARE NOT ACTUALLY UNIQUE TO OBJECT
// TRIANGLES CAN HAVE THEIR OWN.
Triangle::Triangle(
    glm::vec3 idx_, 
    Mesh* parent,
    ObjectType type,
    glm::mat4 transform,
    glm::vec3 ambient,
    glm::vec3 diffuse,
    glm::vec3 specular,
    glm::vec3 emission,
    float shininess
) : Object(
        TRIANGLE,
        transform,
        ambient,
        diffuse,
        specular,
        emission,
        shininess
    )
{
    this->idx = idx_;
    this->parent_mesh = parent;
}

void Triangle::assign_parent(Mesh* p) {
    parent_mesh = p;
}

glm::vec3 Triangle::get_vertex(int v) {
    // returns transformed vertices
    if (v > 3 || v < 0) {
        throw "get_vertex must be in range 0-2, inclusive.";
    }
    int vertex_idx = idx[v];
    glm::vec4 ret = transform * glm::vec4(parent_mesh->get_vertices()->at(vertex_idx), 1);
    return glm::vec3(ret);
}

Mesh::Mesh(
    std::vector<glm::vec3> vertices,
    std::vector<Triangle*> triangles,
    glm::mat4 transform,
    glm::vec3 ambient,
    glm::vec3 diffuse,
    glm::vec3 specular,
    glm::vec3 emission,
    float shininess
) : Object(ObjectType::TRIANGLE, transform, ambient, diffuse, specular, emission, shininess), vertices(vertices) {
    for (int i = 0; i < triangles.size(); i++) {
        triangles[i]->assign_parent(this);
        this->triangles.push_back(triangles[i]);
    }
    bvh = new BVH<Triangle*>(this->triangles);
    std::cout << "\n\nMAKING TRIANGLE/MESH bvh:\n";
    bvh->display();
};
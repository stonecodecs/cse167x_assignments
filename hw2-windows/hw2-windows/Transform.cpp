// Transform.cpp: implementation of the Transform class.

// Note: when you construct a matrix using mat4() or mat3(), it will be COLUMN-MAJOR
// Keep this in mind in readfile.cpp and display.cpp
// See FAQ for more details or if you're having problems.

#include "Transform.h"

// Helper rotation function.  Please implement this.  

mat3 Transform::rotate(const float degrees, const vec3& axis) 
{
    mat3 ret;
    // YOUR CODE FOR HW2 HERE
    // Please implement this.  Likely the same as in HW 1.  
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

    ret = mat3(1.0f) * cos(rads);
    ret += (1.0f - cos(rads)) * outer_product;
    ret += sin(rads) * skew;

    return ret;
}

void Transform::left(float degrees, vec3& eye, vec3& up) 
{
    // YOUR CODE FOR HW2 HERE
    // Likely the same as in HW 1.  
    printf("Coordinates: %.2f, %.2f, %.2f; distance: %.2f\n", eye.x, eye.y, eye.z, sqrt(pow(eye.x, 2) + pow(eye.y, 2) + pow(eye.z, 2)));
    eye = rotate(degrees, up) * eye;
}

void Transform::up(float degrees, vec3& eye, vec3& up) 
{
    // YOUR CODE FOR HW2 HERE 
    // Likely the same as in HW 1.  
    vec3 u = glm::normalize(glm::cross(up, eye));
    eye = rotate(-degrees, u) * eye;
    up = glm::normalize(glm::cross(eye, u));
    printf("Coordinates: %.2f, %.2f, %.2f; distance: %.2f\n", eye.x, eye.y, eye.z, sqrt(pow(eye.x, 2) + pow(eye.y, 2) + pow(eye.z, 2)));
}

mat4 Transform::lookAt(const vec3 &eye, const vec3 &center, const vec3 &up) 
{
    mat4 ret;
    // YOUR CODE FOR HW2 HERE
    // Likely the same as in HW 1.  
    vec3 w = glm::normalize(eye - center);
    vec3 u = glm::normalize(glm::cross(up, w));
    vec3 v = glm::cross(w, u);
    mat4 translation = mat4(1.0f);
    for (int i = 0; i < 3; i++) {
        translation[i][3] = -eye[i];
    }
    translation = glm::transpose(translation);

    mat4 rotation = glm::transpose(mat4(
        u.x, u.y, u.z, 0.f,
        v.x, v.y, v.z, 0.f,
        w.x, w.y, w.z, 0.f,
        0.f, 0.f, 0.f, 1.f
    ));
    ret = rotation * translation;
   
    return ret;
}

mat4 Transform::perspective(float fovy, float aspect, float zNear, float zFar)
{
    mat4 ret;
    // YOUR CODE FOR HW2 HERE
    // New, to implement the perspective transform as well.
    float theta = glm::radians(fovy / 2);
    float d = 1.0f / glm::tan(theta);
    float f = zFar;
    float n = zNear;
    ret = glm::transpose(mat4(
        d/aspect,     0.0f,         0.0f,             0.0f,
            0.0f,        d,         0.0f,             0.0f,
            0.0f,     0.0f, -(f+n)/(f-n),   (-2*f*n)/(f-n),
            0.0f,     0.0f,        -1.0f,             0.0f
    ));
    return ret;
}

mat4 Transform::scale(const float &sx, const float &sy, const float &sz) 
{
    mat4 ret;
    // YOUR CODE FOR HW2 HERE
    // Implement scaling 
    ret = glm::transpose(mat4(
          sx, 0.0f, 0.0f, 0.0f,
        0.0f,   sy, 0.0f, 0.0f,
        0.0f, 0.0f,   sz, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    ));
    return ret;
}

mat4 Transform::translate(const float &tx, const float &ty, const float &tz) 
{
    mat4 ret;
    // YOUR CODE FOR HW2 HERE
    // Implement translation 
    ret = glm::transpose(mat4(
        1.0f, 0.0f, 0.0f,  tx,
        0.0f, 1.0f, 0.0f,  ty,
        0.0f, 0.0f, 1.0f,  tz,
        0.0f, 0.0f, 0.0f, 1.0f
    ));
    
    return ret;
}

// To normalize the up direction and construct a coordinate frame.  
// As discussed in the lecture.  May be relevant to create a properly 
// orthogonal and normalized up. 
// This function is provided as a helper, in case you want to use it. 
// Using this function (in readfile.cpp or display.cpp) is optional.  

vec3 Transform::upvector(const vec3 &up, const vec3 & zvec) 
{
    vec3 x = glm::cross(up,zvec); 
    vec3 y = glm::cross(zvec,x); 
    vec3 ret = glm::normalize(y); 
    return ret; 
}


Transform::Transform()
{

}

Transform::~Transform()
{

}

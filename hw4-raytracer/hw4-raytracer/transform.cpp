#include "transform.h"

glm::mat3 Transform::axis_rotation(const float degrees, const glm::vec3& axis)
{
    glm::mat3 ret;
    float rads = degrees * pi / 180;

    // need to transpose, since OpenGL uses column-major order
    glm::mat3 skew = transpose(glm::mat3(
        0.0f, -axis.z, axis.y,
        axis.z, 0.0f, -axis.x,
        -axis.y, axis.x, 0.0f
    ));

    glm::mat3 outer_product = transpose(glm::mat3(
        axis.x * axis.x, axis.x * axis.y, axis.x * axis.z,
        axis.y * axis.x, axis.y * axis.y, axis.y * axis.z,
        axis.z * axis.x, axis.z * axis.y, axis.z * axis.z
    ));

    ret = glm::mat3(1.0f) * cos(rads);
    ret += (1.0f - cos(rads)) * outer_product;
    ret += sin(rads) * skew;

    return ret;
}

void Transform::left(float degrees, glm::vec3& eye, glm::vec3& up)
{
    printf("Coordinates: %.2f, %.2f, %.2f; distance: %.2f\n", eye.x, eye.y, eye.z, sqrt(pow(eye.x, 2) + pow(eye.y, 2) + pow(eye.z, 2)));
    eye = axis_rotation(degrees, up) * eye;
}

void Transform::up(float degrees, glm::vec3& eye, glm::vec3& up)
{
    glm::vec3 u = glm::normalize(glm::cross(up, eye));
    eye = axis_rotation(-degrees, u) * eye;
    up = glm::normalize(glm::cross(eye, u));
    printf("Coordinates: %.2f, %.2f, %.2f; distance: %.2f\n", eye.x, eye.y, eye.z, sqrt(pow(eye.x, 2) + pow(eye.y, 2) + pow(eye.z, 2)));
}

glm::mat4 Transform::lookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up)
{
    glm::mat4 ret;

    glm::vec3 w = glm::normalize(eye - center);
    glm::vec3 u = glm::normalize(glm::cross(up, w));
    glm::vec3 v = glm::cross(w, u);
    glm::mat4 translation = glm::mat4(1.0f);
    for (int i = 0; i < 3; i++) {
        translation[i][3] = -eye[i];
    }
    translation = glm::transpose(translation);

    glm::mat4 rotation = glm::transpose(glm::mat4(
        u.x, u.y, u.z, 0.f,
        v.x, v.y, v.z, 0.f,
        w.x, w.y, w.z, 0.f,
        0.f, 0.f, 0.f, 1.f
    ));
    ret = rotation * translation;

    return ret;
}

glm::mat4 Transform::perspective(float fovy, float aspect, float zNear, float zFar)
{
    glm::mat4 ret;

    float theta = glm::radians(fovy / 2.0f);
    float d = 1.0f / glm::tan(theta);
    float f = zFar;
    float n = zNear;
    ret = glm::transpose(glm::mat4(
        d / aspect, 0.0f, 0.0f, 0.0f,
        0.0f, d, 0.0f, 0.0f,
        0.0f, 0.0f, -(f + n) / (f - n), (-2 * f * n) / (f - n),
        0.0f, 0.0f, -1.0f, 0.0f
    ));
    return ret;
}

glm::mat4 Transform::scale(const float& sx, const float& sy, const float& sz)
{
    glm::mat4 ret;

    ret = glm::transpose(glm::mat4(
        sx, 0.0f, 0.0f, 0.0f,
        0.0f, sy, 0.0f, 0.0f,
        0.0f, 0.0f, sz, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    ));
    return ret;
}

glm::mat4 Transform::translate(const float& tx, const float& ty, const float& tz)
{
    glm::mat4 ret;

    ret = glm::transpose(glm::mat4(
        1.0f, 0.0f, 0.0f, tx,
        0.0f, 1.0f, 0.0f, ty,
        0.0f, 0.0f, 1.0f, tz,
        0.0f, 0.0f, 0.0f, 1.0f
    ));

    return ret;
}

// To normalize the up direction and construct a coordinate frame.  
// As discussed in the lecture.  May be relevant to create a properly 
// orthogonal and normalized up. 
// This function is provided as a helper, in case you want to use it. 
// Using this function (in readfile.cpp or display.cpp) is optional.  

glm::vec3 Transform::upvector(const glm::vec3& up, const glm::vec3& zvec)
{
    glm::vec3 x = glm::cross(up, zvec);
    glm::vec3 y = glm::cross(zvec, x);
    glm::vec3 ret = glm::normalize(y);
    return ret;
}


Transform::Transform()
{

}

Transform::~Transform()
{

}
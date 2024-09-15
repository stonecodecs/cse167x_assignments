/*****************************************************************************/
/* This is the program skeleton for homework 2 in CSE167 by Ravi Ramamoorthi */
/* Extends HW 1 to deal with shading, more transforms and multiple objects   */
/*****************************************************************************/
// Repurposed to work with the project files.
/*****************************************************************************/
// This file is readfile.cpp.  It includes helper functions for glm::matrix 
// transforglm::mations for a stack (glm::matransform) and to rightmultiply the 
// top of a stack.  These functions are given to aid in setting up the 
// transforglm::mations properly, and to use glm functions in the right way.  
// Their use is optional in your program.  


// The functions readvals and readfile do basic parsing.  You can of course 
// rewrite the parser as you wish, but we think this basic form might be 
// useful to you.  It is a very simple parser.

// Please fill in parts that say YOUR CODE FOR HW 2 HERE. 
// Read the other parts to get a context of what is going on. 

/*****************************************************************************/
#include "readfile.h"
#include "object.h"

void rightmultiply(const glm::mat4& M, std::stack<glm::mat4>& transfstack)
{
    glm::mat4& T = transfstack.top();
    T = T * M;
}

// Function to read the input data values
// Use is optional, but should be very helpful in parsing.  
bool readvals(std::stringstream& s, const int numvals, GLfloat* values)
{
    for (int i = 0; i < numvals; i++) {
        s >> values[i];
        if (s.fail()) {
            std::cout << "Failed reading value " << i << " will skip\n";
            return false;
        }
    }
    return true;
}

Window* readfile(const char* filename)
{
    std::string str, cmd;
    std::ifstream in;
    in.open(filename);
    if (in.is_open()) {
        Window* window = new Window(2, 2, "raytracer"); // initialize with dummy values first
        Scene* scene = new Scene();
        window->attach_scene(scene);

        std::stack <glm::mat4> transfstack; // these are MODEL -> WORLD coordinates
        transfstack.push(glm::mat4(1.0f)); // push identity if root

        unsigned int maxverts = 0;
        unsigned int current_vert = 0; // when this value hits maxverts, store in scene, and reset
        std::vector<glm::vec3> vertices; // for new mesh
        std::vector<Triangle*> triangles; // for new mesh

        // object property states:
        // (FILE FORMAT MATTERS: very fragile)
        glm::vec3 ambient(0.0f);
        glm::vec3 diffuse(0.0f);
        glm::vec3 specular(0.0f);
        glm::vec3 emission(0.0f);
        float shininess = 0.0f;

        getline(in, str);
        while (in) {
            std::cout << "commandstring: " << str << '\n';
            if ((str.find_first_not_of(" \t\r\n") != std::string::npos) && (str[0] != '#')) {
                // Ruled out comment and blank lines 

                std::stringstream s(str);
                s >> cmd;
                int i;
                GLfloat values[10]; // Position and color for light, colors for others
                // Up to 10 params for cameras.  
                bool validinput; // Validity of input 

                // Process the light, add it to database.
                // Lighting Command

                if (cmd == "maxverts") { // store maximum amount of vertices (maybe not needed)
                    validinput = readvals(s, 1, values);
                    if (validinput) {
                        maxverts = values[0];
                    }
                }

                else if (cmd == "vertex") {
                    // vertices persist through entire readfile
                    validinput = readvals(s, 3, values);
                    if (current_vert == maxverts) {
                        // reset for other object (defined with new vertices)
                        vertices.clear();
                        current_vert = 0;
                    }
                    if (validinput && current_vert < maxverts) {
                        vertices.push_back(glm::vec3(values[0], values[1], values[2])); // x,y,z
                        current_vert++;
                    }
                    else {
                        std::cout << "not valid input for 'vertex', or maxverts has been reached.\n";
                    }
                }

                else if (cmd == "tri") { // triangle idx based on vertices
                    // unlike vertices, these are cleared after every popTransform call
                    validinput = readvals(s, 3, values);
                    if (validinput) {
                        // not checking, but values[i] should be a valid vertex index <maxverts
                        Triangle* new_tri = new Triangle(
                            glm::vec3(values[0], values[1], values[2]),
                            nullptr, // parent
                            TRIANGLE,
                            transfstack.top(),
                            ambient,
                            diffuse,
                            specular,
                            emission,
                            shininess
                        );
                        triangles.push_back(new_tri);
                    }
                }

                else if (cmd == "point") { // point light
                    validinput = readvals(s, 6, values); // Position/color for lts.
                    if (validinput) {
                        Light* point_light = new Light(
                            POINT,
                            glm::vec3(values[0], values[1], values[2]), // xyz
                            glm::vec3(values[3], values[4], values[5])  // rgb
                        );

                        scene->register_light(point_light);
                    }
                }

                else if (cmd == "directional") { // directional light
                    validinput = readvals(s, 6, values); // Position/color for lts.
                    if (validinput) {
                        Light* dir_light = new Light(
                            DIRECTIONAL,
                            glm::vec3(values[0], values[1], values[2]), // dxdydz
                            glm::vec3(values[3], values[4], values[5])  // rgb
                        );

                        scene->register_light(dir_light);
                    }
                }

                // glm::material Commands 
                // Ambient, diffuse, specular, shininess properties for each object.
                // Filling this in is pretty straightforward, so I've left it in 
                // the skeleton, also as a hint of how to do the more complex ones.
                // Note that no transforms/stacks are applied to the colors. 

                else if (cmd == "ambient") {
                    validinput = readvals(s, 3, values); // colors 
                    if (validinput) {
                        for (i = 0; i < 3; i++) {
                            ambient[i] = values[i];
                        }
                    }
                }
                else if (cmd == "diffuse") {
                    validinput = readvals(s, 3, values);
                    if (validinput) {
                        for (i = 0; i < 3; i++) {
                            diffuse[i] = values[i];
                        }
                    }
                }
                else if (cmd == "specular") {
                    validinput = readvals(s, 3, values);
                    if (validinput) {
                        for (i = 0; i < 3; i++) {
                            specular[i] = values[i];
                        }
                    }
                }
                else if (cmd == "emission") {
                    validinput = readvals(s, 3, values);
                    if (validinput) {
                        for (i = 0; i < 3; i++) {
                            emission[i] = values[i];
                        }
                    }
                }
                else if (cmd == "shininess") {
                    validinput = readvals(s, 1, values);
                    if (validinput) {
                        shininess = values[0];

                    }
                }
                else if (cmd == "size") { // window size
                    validinput = readvals(s, 2, values);
                    if (validinput) {
                        window->set_size((int)values[0], (int)values[1]);
                        std::cout << "[from readfile] windowsize set to " << values[0] << ", " << values[1];
                    }
                }
                else if (cmd == "camera") {
                    validinput = readvals(s, 10, values); // 10 values eye cen up fov
                    if (validinput) {
                        glm::vec3 eyeinit(0.0f);
                        glm::vec3 center(0.0f);
                        glm::vec3 upinit(0.0f);
                        float fovy;

                        for (int i = 0; i < 3; i++) {
                            eyeinit[i] = values[i];
                            center[i] = values[i + 3];
                            upinit[i] = values[i + 6];
                        }

                        upinit = Transform::upvector(upinit, center - eyeinit);
                        fovy = values[9]; // float in degrees
                        Camera* new_cam = new Camera(
                            eyeinit,
                            upinit,
                            center,
                            window->get_res().w,
                            window->get_res().h,
                            fovy
                        );

                        window->scene->register_camera(new_cam, true);
                        std::cout << "camera_created, posx: " << new_cam->get_pos()[0];
                    }
                }

                // sphere is a separate object from a "Mesh" of triangles/vertices
                else if (cmd == "sphere") {
                    validinput = readvals(s, 4, values); // (x,y,z) + radius
                    if (validinput) {
                        glm::mat4 sphere_transform = Transform::translate(values[0], values[1], values[2]) * transfstack.top();
                        Sphere* obj = new Sphere(
                            values[3],
                            sphere_transform, // stack AND translate
                            ambient,
                            diffuse,
                            specular,
                            emission,
                            shininess
                        );
                        scene->register_object(obj);
                    }
                }

                else if (cmd == "translate") {
                    validinput = readvals(s, 3, values);
                    if (validinput) {
                        glm::mat4 translateM = Transform::translate(
                            values[0], values[1], values[2]);
                        rightmultiply(translateM, transfstack);
                    }
                }
                else if (cmd == "scale") {
                    validinput = readvals(s, 3, values);
                    if (validinput) {
                        glm::mat4 scaleM = Transform::scale(
                            values[0], values[1], values[2]);
                        rightmultiply(scaleM, transfstack);
                    }
                }
                else if (cmd == "rotate") {
                    validinput = readvals(s, 4, values);
                    if (validinput) {
                        glm::vec3 norm_axis = glm::normalize(glm::vec3(values[0], values[1], values[2]));
                        glm::mat3 rot3 = Transform::axis_rotation(values[3], norm_axis);
                        rightmultiply(glm::mat4(rot3), transfstack);
                    }
                }

                else if (cmd == "maxdepth") { // the max depth of recursive raytracing calls
                    validinput = readvals(s, 1, values);
                    if (validinput) {
                        scene->set_maxdepth(values[0]);
                    }
                }

                else if (cmd == "pushTransform") {
                    transfstack.push(transfstack.top());
                }
                else if (cmd == "popTransform") {
                    // whenever we pop, we need to create a new object
                    if (transfstack.size() <= 1) {
                        std::cerr << "Stack has no elements.  Cannot Pop\n";
                    }
                    else {
                        // create Mesh if there is one
                        if (vertices.size() > 0 && triangles.size() > 0) {
                            Mesh* obj = new Mesh(
                                vertices,
                                triangles,
                                transfstack.top(),
                                ambient,
                                diffuse,
                                specular,
                                emission,
                                shininess
                            );
                            scene->register_object(obj);
                        }
                        triangles.clear();
                        transfstack.pop();
                    }
                }

                else {
                    std::cerr << "Unknown Command: " << cmd << " Skipping \n";
                }
            }
            getline(in, str);
        }

        // create Mesh if there is one (if no popTransform)
        if (vertices.size() > 0 && triangles.size() > 0) {
            Mesh* obj = new Mesh(
                vertices,
                triangles,
                transfstack.top(),
                ambient,
                diffuse,
                specular,
                emission,
                shininess
            );
            scene->register_object(obj);
        }

        return window;
    }
    else {
        std::cerr << "Unable to Open Input Data File " << filename << "\n";
        throw 2;
    }
}
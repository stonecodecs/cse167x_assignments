#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <FreeImage.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <deque>
#include <stack>

#include "transform.h"
#include "scene.h"
#include "shader.h"
#include "window.h"
#include "readfile.h"

#define MAINPROGRAM


void resizeWindowCallback(GLFWwindow* window, int width, int height);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void initialize(int argc, char* argv[]);
void printHelp();
void saveScreenshot(std::string fname, Window* window);

Window* window1; // can be extended to multiple later
Scene* scene1; // attach to the window
Camera* cam1; // attach to Scene

// Default starting parameters: (otherwise, get from readfile if included)
    // for shader:
const std::string VERTEX_SHADER_PATH = "./shaders/vertex_demo.glsl";
const std::string FRAGMENT_SHADER_PATH = "./shaders/fragment_demo.glsl";

    // for window:
int init_width = 800;
int init_height = 600;

    // for camera:
glm::vec3 eye_init(0.0, 0.0, 5.0);
glm::vec3 up_init(0.0, 1.0, 0.0);
glm::vec3 center_init(0.0, 0.0, 0.0);
float fovy_init = 90.0;
float z_near_init = 10.0;
float z_far_init = 100.0;
int sensitivity_init(5);
    // scene scale/translate
float sx(1.0), sy(1.0);
float tx(0.0), ty(0.0); 

// NOTE: these are flipped around the y-axis so that the raytraced display is upright
float quad_verts[] = {
    // positions   // texture coords
    -1.0f,  1.0f,  0.0f, 0.0f,   // Top-left
    -1.0f, -1.0f,  0.0f, 1.0f,   // Bottom-left
     1.0f, -1.0f,  1.0f, 1.0f,   // Bottom-right
     1.0f,  1.0f,  1.0f, 0.0f    // Top-right
};

unsigned int quad_idx[] = {
     0, 1, 2,
     2, 3, 0
};


/* 
    Adjusts viewport dimensions to (width, height) when window is resized.
*/
void resizeWindowCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);

    // may not be needed
    Camera* scene_cam = scene1->get_main_camera();
    if (scene_cam != NULL) {
        scene_cam->set_height(height);
        scene_cam->set_width(width);
    }
}

void keyCallback(GLFWwindow* window, int key, int scanline, int action, int mods) {
    int state = glfwGetKey(window, key); // use state instead of action for held buttons
    TransformType transop = window1->scene->get_transform_type();
    Camera* main_cam = window1->scene->get_main_camera();
    int amount = window1->scene->get_sensitivity();
    Resolution res = window1->get_res(); // as a check

    if (state == GLFW_PRESS) {
        //std::cout << "key pressed: " << key << '\n';
        //std::cout << "mod : " << mods << '\n';

        switch (key) {
            case GLFW_KEY_EQUAL:  // '+' key
                if (mods == 1) { // shift + '=' is '+'
                    // increase sensitivity for the camera
                    scene1->transform_sensitivity_up(); 
                    amount = window1->scene->get_sensitivity();
                    std::cout << "[sens up!] amount set to " << amount << "\n";
                }
                break;
            case GLFW_KEY_MINUS:  // '-' key
                scene1->transform_sensitivity_down();
                amount = window1->scene->get_sensitivity();
                std::cout << "[sens down!] amount set to " << amount << "\n";
                break;
            case GLFW_KEY_H:
                printHelp();
                std::cout << "current_window size: (" << res.w << "," << res.h << ")\n";
                break;
            case GLFW_KEY_ESCAPE:  // Escape key
                window1->close();
                break;
            case GLFW_KEY_R:  // reset eye and up vectors, scale and translations
                // reset camera parameters:
                if (main_cam != NULL) { // if already exists
                    main_cam->set_pos(eye_init);
                    main_cam->set_up(up_init);
                    main_cam->set_center(center_init);
                    scene1->set_sensitivity(sensitivity_init);
                }
                else { // otherwise create new camera; should never reach, though
                    Camera* new_cam = new Camera(
                        eye_init,
                        up_init,
                        center_init,
                        init_width,
                        init_height,
                        fovy_init,
                        z_near_init,
                        z_far_init,
                        ProjectionType::PERSPECTIVE
                    );
                    scene1->register_camera(new_cam);
                }

                // reset scale/translation ratios for modelview
                sx = 1.0, sy = 1.0;
                tx = 0.0, ty = 0.0;
                break;
            case GLFW_KEY_V:
                // use WINDOW here because it directly processes the callback function
                window1->scene->set_transform_type(ROTATE);
                std::cout << "Operation is set to View\n";
                break;
            case GLFW_KEY_T:
                window1->scene->set_transform_type(TRANSLATE);
                std::cout << "Operation is set to Translate\n";
                break;
            case GLFW_KEY_S:
                window1->scene->set_transform_type(SCALE);
                std::cout << "Operation is set to Scale\n";
                break;
            case GLFW_KEY_LEFT: //left
                if (transop == ROTATE && main_cam != NULL) {
                    main_cam->rotate_left((float)amount);
                }
                else if (transop == SCALE) sx -= (float)amount * 0.01;
                else if (transop == TRANSLATE) tx -= (float)amount * 0.01;
                break;
            case GLFW_KEY_UP: //up
                if (transop == ROTATE && main_cam != NULL) {
                    main_cam->rotate_up((float)amount);
                }
                else if (transop == SCALE) sy += (float)amount * 0.01;
                else if (transop == TRANSLATE) ty += (float)amount * 0.01;
                break;
            case GLFW_KEY_RIGHT: //right
                if (transop == ROTATE && main_cam != NULL) {
                    main_cam->rotate_left((float)-amount);
                }
                else if (transop == SCALE) sx += (float)amount * 0.01;
                else if (transop == TRANSLATE) tx += (float)amount * 0.01;
                break;
            case GLFW_KEY_DOWN: //down
                if (transop == ROTATE && main_cam != NULL) {
                    main_cam->rotate_up((float)-amount);
                }
                else if (transop == SCALE) sy -= (float)amount * 0.01;
                else if (transop == TRANSLATE) ty -= (float)amount * 0.01;
                break;
            case GLFW_KEY_0:
                if (window1->scene->set_current_camera(0)) {
                    std::cout << "set to camera0\n";
                } else { std::cout << "no camera registered at this ID\n"; }
                break;
            case GLFW_KEY_1:
                if (window1->scene->set_current_camera(1)) {
                    std::cout << "set to camera1\n";
                } else { std::cout << "no camera registered at this ID\n"; }
                break;
            case GLFW_KEY_2:
                if (window1->scene->set_current_camera(2)) {
                    std::cout << "set to camera2\n";
                } else { std::cout << "no camera registered at this ID\n"; }
                break;
            case GLFW_KEY_3:
                if (window1->scene->set_current_camera(3)) {
                    std::cout << "set to camera3\n";
                } else { std::cout << "no camera registered at this ID\n"; }
                break;
            case GLFW_KEY_4:
                if (window1->scene->set_current_camera(4)) {
                    std::cout << "set to camera4\n";
                } else { std::cout << "no camera registered at this ID\n"; }
                break;
            case GLFW_KEY_5:
                if (window1->scene->set_current_camera(5)) {
                    std::cout << "set to camera5\n";
                } else { std::cout << "no camera registered at this ID\n"; }
                break;

        }
    }
}

void initQuad(unsigned int &VAO, unsigned int& VBO, unsigned int& EBO) {
    // Generate and bind VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Generate and bind VBO, upload vertex data
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_verts), quad_verts, GL_STATIC_DRAW);

    // Generate and bind EBO, upload index data
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_idx), quad_idx, GL_STATIC_DRAW);

    // Set up vertex attribute pointers
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0); // other 2 coordinates for texture
    glEnableVertexAttribArray(0);

    // Unbind VAO
    glBindVertexArray(0);
}

// add Image &cam_data when done with testing to parameters as texture
void renderQuad(std::vector<unsigned char> texture, unsigned int &VAO) {
    // takes in the Image created from the ray tracer in Scene
    glBindVertexArray(VAO);

    // load texture
    unsigned int tex0;
    glGenTextures(1, &tex0);
    glBindTexture(GL_TEXTURE_2D, tex0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, cam1->get_width(), cam1->get_height(), 0, GL_RGB, GL_UNSIGNED_BYTE, texture.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0); // unbind
}

std::vector<unsigned char> flattenTexture(const RGBImage& texture) {
    std::vector<unsigned char> flattened;
    for (const auto& row : texture) {
        for (const auto& pixel : row) {
            flattened.push_back(static_cast<unsigned char>(pixel.r));
            flattened.push_back(static_cast<unsigned char>(pixel.g));
            flattened.push_back(static_cast<unsigned char>(pixel.b));
        }
    }
    return flattened;
}


/*
    Things to initialize before running the program.
*/
void initialize(int argc, char* argv[]) {
    // global window hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // option to configure, int that sets value
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4); // version 4 here
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // uncomment for MacOS:
    // glfwWindowHint(GLEW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    if (argc < 2) {
        std::cout << "Filename not included, initiaizing with default settings.";
        // initial window, scene, and camera
        window1 = new Window(init_width, init_height, "raytracer-demo");
        // readfile here; if doesn't exist, do below
        cam1 = new Camera(
            eye_init,
            up_init,
            center_init,
            init_width,
            init_height,
            fovy_init,
            z_near_init,
            z_far_init,
            ProjectionType::PERSPECTIVE
        );
        scene1 = new Scene(cam1); // auto-registers as main camera
        window1->attach_scene(scene1);
    }
    else {
        std::cout << "Data read from " << argv[1] << ".\n";
        const char* filepath = argv[1];
        window1 = readfile(filepath); // scene already attached
        scene1 = window1->scene;
        cam1 = scene1->get_main_camera();
        std::cout << scene1->how_many_objects() << "<- objects registered\n";
    }

    window1->make_context_current();
    scene1->construct_bvh(); // construct BVH
    scene1->print_bvh();
    FreeImage_Initialise(); // FreeImage
}

void deinitialize() {
    glfwDestroyWindow(window1->window); // frees memory
    delete window1; // automatically deletes cameras and objects in scene

    FreeImage_DeInitialise();
    glfwTerminate();
}

/*
void transformvec(const GLfloat input[4], GLfloat output[4])
{
    glm::vec4 inputvec(input[0], input[1], input[2], input[3]);
    glm::vec4 outputvec = modelview * inputvec;

    output[0] = outputvec[0];
    output[1] = outputvec[1];
    output[2] = outputvec[2];
    output[3] = outputvec[3];
}

void display()
{
    glClearColor(0, 0, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up the camera view

    // modelviewPos from 
    glUniformMatrix4fv(modelviewPos, 1, GL_FALSE, &modelview[0][0]);

    // Lights are transformed by current modelview matrix. 
    // The shader can't do this globally. 
    // So we need to do so manually.  
    if (numused) {
        glUniform1i(enablelighting, true); // these are uniform variables in shader that we init
        // by first setting location using glGetUniformLocation (main.cpp)
        // then doing this to set variable "enableighting" to 'true'

        // YOUR CODE FOR HW 2 HERE.  
        // You need to pass the light positions and colors to the shader. 
        // glUniform4fv() and similar functions will be useful. See FAQ for help with these functions.
        // The lightransf[] array in variables.h and transformvec() might also be useful here.
        // Remember that light positions must be transformed by modelview.  
        glUniform1i(numusedcol, numused);

        for (int i = 0; i < numused; i++) {
            transformvec(&lightposn[i * 4], &lightransf[i * 4]);
        }

        glUniform4fv(lightpos, numLights, lightransf);
        glUniform4fv(lightcol, numLights, lightcolor);

    }
    else {
        glUniform1i(enablelighting, false);
    }

    // Transformations for objects, involving translation and scaling 
    glm::mat4 sc(1.0), tr(1.0), transf(1.0);
    sc = Transform::scale(sx, sy, 1.0);
    tr = Transform::translate(tx, ty, 0.0);

    // YOUR CODE FOR HW 2 HERE.  
    // You need to use scale, translate and modelview to 
    // set up the net transformation matrix for the objects.  
    // Account for GLM issues, matrix order (!!), etc.  

    transf = modelview * tr * sc;

    // The object draw functions will need to further modify the top of the stack,

    // so assign whatever transformation matrix you intend to work with to modelview

    // rather than use a uniform variable for that.
    modelview = transf;

    for (int i = 0; i < numobjects; i++) {
        object* obj = &(objects[i]); // Grabs an object struct.

        // YOUR CODE FOR HW 2 HERE. 
        // Set up the object transformations 
        // And pass in the appropriate material properties
        // Again glUniform() related functions will be useful
        modelview = transf * obj->transform;

        glUniform4fv(ambientcol, 1, obj->ambient);
        glUniform4fv(diffusecol, 1, obj->diffuse);
        glUniform4fv(specularcol, 1, obj->specular);
        glUniform4fv(emissioncol, 1, obj->emission);
        glUniform1f(shininesscol, obj->shininess);

        // Actually draw the object
        // We provide the actual drawing functions for you.  
        // Remember that obj->type is notation for accessing struct fields
        if (obj->type == cube) {
            solidCube(obj->size);
        }
        else if (obj->type == sphere) {
            const int tessel = 20;
            solidSphere(obj->size, tessel, tessel);
        }
        else if (obj->type == teapot) {
            solidTeapot(obj->size);
        }
        //std::cout << "obj" << i << std::endl;
        //printmat4(glm::transpose(modelview));
    }
}
*/


void saveScreenshot(std::string fname, Window* window) {
    Camera* main_cam = window->scene->get_main_camera();
    if (main_cam == NULL) {
        std::cout << "Could not save screenshot: Scene has no camera.";
        return;
    }
    int pix = main_cam->get_width() * main_cam->get_height();
    BYTE* pixels = new BYTE[3 * pix];
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, main_cam->get_width(), main_cam->get_height(), GL_BGR, GL_UNSIGNED_BYTE, pixels);

    FIBITMAP* img = FreeImage_ConvertFromRawBits(
        pixels, 
        main_cam->get_width(),
        main_cam->get_height(),
        main_cam->get_width() * 3, 24, 0xFF0000, 0x00FF00, 0x0000FF, false);

    std::cout << "Saving screenshot: " << fname << "\n";

    FreeImage_Save(FIF_PNG, img, fname.c_str(), 0);
    delete[] pixels;
}


void printHelp() {
    std::cout << "\npress 'h' to print this message again.\n"
        << "press '+' or '-' to change the amount of rotation that\noccurs with each arrow press.\n"
        << "press 'r' to reset the transformations.\n"
        << "press 'v' 't' 's' to rotate (view) [default], translate, scale.\n"
        << "press ESC to quit.\n";
}


int main(int argc, char* argv[])
{

    if (!glfwInit()) {
        std::cerr << "failed to initialize GLFW" << '\n';
        return -1;
    }

    // init Window, Scene, Camera, FreeInit
    initialize(argc, argv);

    // load address of OS-specific OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << '\n';
        return -1;
    }

    unsigned int VAO{ 0 }, VBO{ 0 }, EBO{ 0 };
    initQuad(VAO, VBO, EBO);

    // create vertex and fragment shaders
    Shader shader = Shader(VERTEX_SHADER_PATH.c_str(), FRAGMENT_SHADER_PATH.c_str());
    shader.use();

    /*
     * At this point, we've sent data 'vertices' to the GPU
     * and instructed the GPU on how to process it.
     *
     * Now, must tell OpenGL how to 'interpret' the vertex data in memory
     * and how it should connect veretx daat to the vertex shader's attributes.
     */

     /*
      * (
      *    0: shader "layout (location = 0)",
      *    SIZE of EACH vertex attribute is '3' floats: (x,y,z); (r,g,b)
      *    TYPE
      *    NORMALIZE DATA? (GL_T/F)
      *    STRIDE: space for data for a SINGLE vertex (position + color here)
      *    OFFSET: position where data begins in buffer (usually 0) [between attributes of vertex data]
      * )
      * NOTE: VBO it takes attribute data from is the one currently bound with glBindBuffer
     */


    // would have to repeat process glBind->bufferData->attribptr->enable->useProgram->drawing function
    // each time we'd want to draw an object
    // solution: VAO (vertex array objects) switches between bindings of different vertex data easily

    window1->set_key_callback(keyCallback);
    window1->set_framebuffersize_callback(resizeWindowCallback);

    // callback function called to make sure glViewport matches window resolution parameters after readfile
    resizeWindowCallback(window1->window, cam1->get_width(), cam1->get_height());
    printHelp();

    // render loop
    while (window1->is_active()) { // if not instructed to close
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        shader.use();

        // render the quad with the texture from the scene's camera
        renderQuad(flattenTexture(scene1->raytrace()), VAO);

        window1->poll_events();
        window1->swap_buffers();
    }

    // deallocation of resources
    shader.remove(); // calls glDeleteProgram
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    deinitialize();
    return 0;
}
#include <iostream>
#include "window.h"

Window::Window(
	int width,
	int height,
	const char* name,
	Scene* scene,
	GLFWmonitor* monitor,
	GLFWwindow* share){

	this->name = name;
	this->window = glfwCreateWindow(width, height, name, monitor, share);
	this->scene = scene;
	if (window == NULL) {
		std::cout << "Failed to create GLFW window for " << name << ". \n";
		glfwTerminate();
	}
	glfwMakeContextCurrent(this->window);
}

Window::~Window() {
	// window is deleted in deinitialize (main.cpp)
	delete scene;
}

bool Window::is_active() {
	bool close = glfwWindowShouldClose(this->window);
	return !close;
}

void Window::close() {
	glfwSetWindowShouldClose(this->window, GL_TRUE);
}

void Window::set_size(int w, int h) {
	glfwSetWindowSize(this->window, w, h);
	// affect main_cam as well
	Camera* main_cam = scene->get_main_camera();
	if (main_cam != NULL) {
		main_cam->set_width(w);
		main_cam->set_height(h);
	}
}

Resolution Window::get_res() {
	int w, h;
	glfwGetFramebufferSize(this->window, &w, &h); // for pixels, unlike glfwGetWindowSize
	Resolution res(w, h);
	return res;
}

GLFWkeyfun Window::set_key_callback(GLFWkeyfun callback_func) {
	return glfwSetKeyCallback(window, callback_func);
}

GLFWframebuffersizefun Window::set_framebuffersize_callback(GLFWframebuffersizefun callback_func) {
	return glfwSetFramebufferSizeCallback(window, callback_func);
}

void Window::swap_buffers() {
	glfwSwapBuffers(this->window);
}

void Window::poll_events() {
	glfwPollEvents();
}

void Window::make_context_current() {
	glfwMakeContextCurrent(this->window);
}

//void Window::set_display_func(DisplayFunc f) {
//	display_func = f;
//}

void Window::attach_scene(Scene* s) {
	scene = s;
}
#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "scene.h"

struct Resolution {
	int w, h;
	Resolution(int w_, int h_) : w(w_), h(h_) {}
};

// mostly a wrapper class for GLFWwindow and other GLFW aspects
class Window {
public:
	GLFWwindow* window; // connect window
	Scene* scene; // with a scene
	const char* name;

	Window(
		int width,
		int height,
		const char* name,
		Scene* scene = NULL,
		GLFWmonitor* monitor = NULL,
		GLFWwindow* share = NULL);
	~Window();

	bool is_active();
	void close();
	void set_size(int w, int h);
	Resolution get_res();
	GLFWkeyfun set_key_callback(GLFWkeyfun callback_func);
	GLFWframebuffersizefun set_framebuffersize_callback(GLFWframebuffersizefun callback_func);
	void swap_buffers();
	void poll_events(); // for this window context
	void make_context_current();
	// void set_display_func(DisplayFunc f); // moved responsibility to Scene
	void attach_scene(Scene* scene);
};
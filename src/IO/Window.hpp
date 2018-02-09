/* GLFW Handler
* Maintains GLFW window, mouse, and keyboard*/
#pragma once
#ifndef _GLFW_HANDLER_HPP_
#define _GLFW_HANDLER_HPP_

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Mouse.hpp"
#include "Keyboard.hpp"

#include <string>

#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 960

class Window {
public:
    /* Window size */
    static int width;
    static int height;

    /* Reference to GLFW window, mouse, keyboard */
    GLFWwindow *window;

    /* Init */
    int init(std::string);

    /* Set window title */
    void setTitle(const char *);

    /* Update */
    void update();

    /* Return if window should close */
    int shouldClose();

    /* Return running time */
    double getTime();

    /* Shut down */
    void shutDown();
private:
    /* Callback functions */
    static void errorCallback(int, const char *);
    static void keyCallback(GLFWwindow *, int, int, int, int);
    static void mouseButtonCallback(GLFWwindow *, int, int, int);
};

#endif
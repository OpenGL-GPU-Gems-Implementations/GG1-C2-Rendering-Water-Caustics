/**
 * @file kernel.h
 * @author Eron Ristich (eron@ristich.com)
 * @brief Handles application window and construction as well as OpenGL context
 * @version 0.1
 * @date 2022-05-29
 * 
 * @copyright Copyright (c) 2022
 */

#ifndef KERNEL_H
#define KERNEL_H

#include <iostream>
#include <string>
using std::string;

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <GL/glut.h>

#include "../objects/helper.h"
#include "../objects/camera.h"
#include "../objects/skybox.h"
#include "../objects/water.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Kernel {
    public:
        Kernel();
        ~Kernel();

        bool initSDL();
        bool initGL();
        bool initIMG();

        void start(string title, int resx, int resy);

        void render();
        void update(float dt);
        void handleEvents();

    private:
        bool isRunning;
        int rx, ry;

        SDL_Window* window;
        SDL_Renderer* renderer;
        SDL_GLContext glContext;

        bool wDown, aDown, sDown, dDown, spDown, shDown, ctDown, enDown;
        int relX, relY;
        int lastZoom, curZoom;

        // ---- Objects in kernel ----

        // Rocks (for caustics)
        unsigned int normalTex, refractionTex;

        // Camera
        Camera*  camera;

        // Skybox
        Skybox*  skybox;

        // Water
        Water*   water;
        Shader*  water_shader;

        // Test backpack model
        Shader*  backpack_shader;
        Model*   backpack_model;

        // Test rock model
        Shader*  rocks_shader;
        Model*   rocks_model;

};

#endif
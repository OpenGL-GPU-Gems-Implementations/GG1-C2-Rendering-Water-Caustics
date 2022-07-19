/**
 * @file kernel.h
 * @author Eron Ristich (eron@ristich.com)
 * @brief Handles application window and construction as well as OpenGL context
 * @version 0.1
 * @date 2022-05-29
 * 
 * @copyright Copyright (c) 2022
 */

// in order to simulate caustics, what this program does is
/*  1. pass water surface heightmap as texture to program (via ssbo's or other textures) [perhaps an rgba texture? - NORMAL.X, NORMAL.Y, NORMAL.Z, HEIGHT]
    2. cast a ray straight up to the heightmap and refract it accordingly
    3. take the difference in angle of that ray to the direction of the sun (at <0, 1, 0>) and compute an exponential fade function on said angle to determine the intensity of the resultant water caustic
    4. reap rewards
*/

#include <chrono>

#include "kernel.h"
#include "../objects/water.h"
#include "../objects/camera.h"

/**
 * @brief Construct a new Kernel::Kernel object
 */
Kernel::Kernel() {
    std::cout << "Reached line " << __LINE__ << " in " << __FILE__ << std::endl;
    rx = 0;
    ry = 0;
    isRunning = false;
}

/**
 * @brief Destroy the Kernel::Kernel object
 */
Kernel::~Kernel() {
    SDL_DestroyRenderer(renderer);
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);

    renderer = NULL;
    window = NULL;
}

/**
 * @brief Initializes SDL functions
 * 
 * @return bool representing the success of the operation
 */
bool Kernel::initSDL() {
    if (SDL_Init(SDL_INIT_NOPARACHUTE) && SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_Log("Unable to initialize SDL: %s\n", SDL_GetError());
        return false;
    }
    
    //Specify OpenGL Version (4.3)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_Log("SDL Initialized");
    return true;
}

/**
 * @brief Initializes gl functions and VSync (assumes window has already been defined)
 * 
 * @return bool representing the success of the operation
 */
bool Kernel::initGL() {
    // Specify shading technique
    glShadeModel(GL_SMOOTH);

    // Interpolation quality
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // Initial color definition
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Check GLEW initialization
    glewExperimental = GL_TRUE;
    GLenum error = glewInit();
    if(error != GLEW_OK) {
        SDL_Log("Could not initialize GLEW: %s\n", glewGetErrorString(error));
        return false;
    } else {
        SDL_Log("GLEW initialized successfully");
        glViewport(0, 0, (GLsizei)SDL_GetWindowSurface(window)->w, (GLsizei)SDL_GetWindowSurface(window)->h);
    }

    //Use VSync (limit to refresh rate of monitor)
    if(SDL_GL_SetSwapInterval(1) < 0) {
        SDL_Log("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
        // does not return false, VSync is not essential to program
    } else {
        SDL_Log("VSync initialized");
    }
    return true;
}

/**
 * @brief Initializes SDL_Image image loading
 * 
 * @return bool representing the success of the operation
 */
bool Kernel::initIMG() {
    int imgFlags = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF;
    if(!(IMG_Init(imgFlags) & imgFlags)) {
        SDL_Log("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }

    SDL_Log("SDL_image initialized");
    return true;
}

/**
 * @brief Initializes main application
 * 
 * @param resx 
 * @param resy 
 */
void Kernel::start(string title, int resx, int resy) {
    rx = resx; ry = resy;

    // Initialize SDL
    if (!initSDL())
        return;

    // Create and verify window
    window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        rx,
        ry,
        SDL_WINDOW_OPENGL
    );

    if(window == NULL) {
        SDL_Log("Could not create window: %s\n", SDL_GetError());
        return;
    }
    else
        SDL_Log("Window successfully generated");
    
    // Create and verify renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    if(renderer == NULL) {
        SDL_Log("Could not create renderer: %s\n", SDL_GetError());
        return;
    } else
        SDL_Log("Renderer successfully generated");
    
    // Initialize GL Context
    glContext = SDL_GL_CreateContext(window);
    if (!initGL())
        return;

    // Initialize SDL_image
    if (!initIMG())
        return;
    
    // Setup objects
    //camera = new Camera(glm::vec3(0, 0, 3));
    camera = new Camera(glm::vec3(-12.5, -6.5, -55), glm::vec3(0, 1, 0), -270, 0);

    string skyboxTitle = "yokohama/";
    string fileExtension = ".jpg";
    vector<std::string> faces {
        string("resources/skyboxes/") + skyboxTitle + string("negx") + fileExtension,
        string("resources/skyboxes/") + skyboxTitle + string("posx") + fileExtension,
        string("resources/skyboxes/") + skyboxTitle + string("negy") + fileExtension,
        string("resources/skyboxes/") + skyboxTitle + string("posy") + fileExtension,
        string("resources/skyboxes/") + skyboxTitle + string("negz") + fileExtension,
        string("resources/skyboxes/") + skyboxTitle + string("posz") + fileExtension
    };
    skybox = new Skybox("shaders/skybox.vs", "shaders/skybox.fs", faces);

    //backpack_shader = new Shader("shaders/backpack.vs", "shaders/backpack.fs");
    //backpack_model  = new Model("resources/backpack/backpack.obj");

    rocks_shader    = new Shader("shaders/rocks.vs", "shaders/rocks.fs");
    rocks_model     = new Model("resources/rocks/rocks.obj");

    int pX, pZ, pW, pL, pdimX, pdimZ;
    pX = 0; pZ = 0;
    pW = 50; pL = 50;
    pdimX = 500; pdimZ = 500;
    water = new Water(pX, pZ, pW, pL, pdimX, pdimZ, 0.1f, 20, true, true, false);
    water_shader = new Shader("shaders/water.vs", "shaders/water.fs");

    // Generate water height/normal map texture <NORMAL.X, NORMAL.Y, NORMAL.Z> (height is irrelevant because all vectors point straight up anyhow)
    //Uint32 rmask, bmask, gmask, amask;
    //rmask = 0xff000000 >> 8;
    //gmask = 0x00ff0000 >> 8;
    //bmask = 0x0000ff00 >> 8;
    //amask = 0x000000ff >> 8;

    unsigned char* data = new unsigned char[pdimX * pdimZ * 3];
    for (int i = 0; i < pdimX; i ++) {
        for (int j = 0; j < pdimZ; j ++) {
            float x = pX - pW / 2 + (float)i * pW / pdimX;
            float z = pZ - pL / 2 + (float)j * pL / pdimZ;

            glm::vec3 normal = water->N(x, z, 0);
            data[i * pdimZ*3 + j*3 + 0] = (unsigned char)((normal.x / 2 + 0.5) * 256);
            data[i * pdimZ*3 + j*3 + 1] = (unsigned char)((normal.y / 2 + 0.5) * 256);
            data[i * pdimZ*3 + j*3 + 2] = (unsigned char)((normal.z / 2 + 0.5) * 256);
        }
    }

    // Generate additive refraction result texture (basically a shiny dot in the center of the same resolution as the previously generated texture)
    // we can use the same masks!
    unsigned char* refract = new unsigned char[pdimX * pdimZ * 3];
    int centerX, centerY;
    centerX = pdimX / 2; centerY = pdimZ / 2;
    for (int i = 0; i < pdimX; i ++) {
        for (int j = 0; j < pdimZ; j ++) {
            float distX = (float)abs(centerX - i) / pdimX * 2;
            float distY = (float)abs(centerY - j) / pdimZ * 2;

            float distance = sqrt((float)(distX*distX + distY*distY));
            int val = (1 - distance > 0) ? (int)((1.0f - 2*distance)*256) : 0; // linear relation as a function of distance

            refract[i * pdimZ*3 + j*3 + 0] = (unsigned char)val;
            refract[i * pdimZ*3 + j*3 + 1] = (unsigned char)val;
            refract[i * pdimZ*3 + j*3 + 2] = (unsigned char)val;
        }
    }

    std::cout << (int)refract[0];

    // Generate textures
    glGenTextures(1, &normalTex);

    glGenTextures(1, &refractionTex);

    glBindTexture(GL_TEXTURE_2D, normalTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pdimX, pdimZ, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    glBindTexture(GL_TEXTURE_2D, refractionTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pdimX, pdimZ, 0, GL_RGB, GL_UNSIGNED_BYTE, refract);

    // Start loop
    isRunning = true;
    glEnable(GL_DEPTH_TEST);

    // Start time of loop
    //auto initT = std::chrono::steady_clock::now();
    auto lastT = std::chrono::steady_clock::now();
    int frame = 0;
    int curFPS = 0;
    float sumFPS = 0.001;

    // Relative mouse mode (hide mouse)
    SDL_SetRelativeMouseMode(SDL_TRUE);

    // Uncomment for wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (isRunning) {
        // iterate frame count
        frame ++;

        // determine time between frames
        auto curT = std::chrono::steady_clock::now();
        std::chrono::duration<float> diff = curT - lastT;
        lastT = std::chrono::steady_clock::now();
        float dt = diff.count();
        sumFPS += dt;

        // update window title
        // only update FPS every 30 frames
        if (frame % 30 == 1) {
            curFPS = (int)(30/sumFPS);
            sumFPS = 0;
        }
        string atitle = title + string(" - FPS: ") + std::to_string(curFPS) + string(" - Frame: ") + std::to_string(frame);
        SDL_SetWindowTitle(window, atitle.c_str());

        // handle events
        handleEvents();

        // update camera
        int end = NONE;
        if (wDown)
            end |= FORWARD;
        if (aDown)
            end |= LEFT;
        if (sDown)
            end |= BACKWARD;
        if (dDown)
            end |= RIGHT;
        if (spDown)
            end |= UP;
        if (shDown)
            end |= DOWN;
        camera->updateKeyboard(end, dt);
        camera->updateMouse(relX, -relY);

        // update renderer
        if (frame % 2 == 1) {
            //update(dt);
        }
        render();
    }

    delete data;
    delete refract;
}

/**
 * @brief Renders objects as defined by update cycle
 */
void Kernel::render() {
    // clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // sets backpack shaders as active
    //backpack_shader->use();

    // compute matrices
    glm::mat4 projection = glm::perspective(glm::radians(camera->zoom), (float)rx / (float)ry, 0.1f, 100.0f);
    glm::mat4 view = camera->getViewMatrix();

    // loads shaders
    //backpack_shader->setMat4("projection", projection);
    //backpack_shader->setMat4("view", view);
    //backpack_shader->setVec3("cameraPos", camera->position);

    // render model
    glm::mat4 model = glm::mat4(1.0f);
    //model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
    //model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
    //backpack_shader->setMat4("model", model);
    //backpack_model->draw(backpack_shader);

    // same thing but for rocks
    rocks_shader->use();
    rocks_shader->setMat4("projection", projection);
    rocks_shader->setMat4("view", view);
    rocks_shader->setVec3("cameraPos", camera->position);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-12.5f, -20.0f, -12.5f));
    model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
    model = glm::rotate(model, 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    rocks_shader->setMat4("model", model);
    rocks_shader->setInt("normal", 1);
    rocks_shader->setInt("refractions", 2);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, normalTex);
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, refractionTex);
    rocks_model->draw(rocks_shader);

    // render water
    water_shader->use();
    water_shader->setMat4("projection", projection);
    water_shader->setMat4("view", view);
    model = glm::mat4(1.0f);
    water_shader->setMat4("model", model);
    water_shader->setVec3("cameraPos", camera->position);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    water->draw(water_shader, skybox->cubeTexture);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // draw skybox last
    //skybox->draw(camera, rx, ry);

    glFlush();

    SDL_GL_SwapWindow(window);
}

/**
 * @brief Updates all objects in world (positions, meshes, etc.)
 */
void Kernel::update(float dt) {
    water->updateTime(dt);
    water->updateMesh();
}

/**
 * @brief Handles all events that occur in a window between frames
 */
void Kernel::handleEvents() {
    relX = 0; relY = 0;
    SDL_Event m_event;
	while(SDL_PollEvent(&m_event)) {
		switch (m_event.type) {
            case SDL_KEYDOWN:
                switch (m_event.key.keysym.sym) {
                    case SDLK_ESCAPE: // exit window
                        isRunning = false;
                        break;
                    case SDLK_w: // w
                        wDown = true;
                        break;
                    case SDLK_a: // a
                        aDown = true;
                        break;
                    case SDLK_s: // s
                        sDown = true;
                        break;
                    case SDLK_d: // d
                        dDown = true;
                        break;
                    case SDLK_SPACE: // spacebar
                        spDown = true;
                        break;
                    case SDLK_LSHIFT: // left shift
                        shDown = true;
                        break;
                    case SDLK_RETURN: // enter
                        enDown = true;
                        std::cout << "\nCamera position: " << camera->position.x << " " << camera->position.y << " " << camera->position.z;
                        std::cout << "\nCamera orientation: " << camera->yaw << " " << camera->pitch;
                        break;
                }
                break;
            
            case SDL_KEYUP:
                switch (m_event.key.keysym.sym) {
                    case SDLK_w: // w
                        wDown = false;
                        break;
                    case SDLK_a: // a
                        aDown = false;
                        break;
                    case SDLK_s: // s
                        sDown = false;
                        break;
                    case SDLK_d: // d
                        dDown = false;
                        break;
                    case SDLK_SPACE: // spacebar
                        spDown = false;
                        break;
                    case SDLK_LSHIFT: // left shift
                        shDown = false;
                        break;
                    case SDLK_RETURN: // enter
                        enDown = false;
                        break;
                }
                break;

            case SDL_WINDOWEVENT:
                switch (m_event.window.event) {
                    case SDL_WINDOWEVENT_CLOSE: // exit window
                        isRunning = false;
                        break;
                }
                break;
            
            case SDL_MOUSEMOTION:
                relX = m_event.motion.xrel;
                relY = m_event.motion.yrel;
                break;
        }
	}
}
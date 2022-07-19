/**
 * @file water.h
 * @author Eron Ristich (eron@ristich.com)
 * @brief Stores vertex (fragment) information of a simple plane of water. Variations implement various functions detailed in GG1-C1, for directional/circular waves, pointed/rounded crests, etc.
 * @version 0.1
 * @date 2022-05-31
 * 
 * @copyright Copyright (c) 2022
 */

#ifndef WATER_H
#define WATER_H

#include "helper.h"

#include <vector>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MAXFREQ 1.0f
#define MAXSPED 0.005f

//TODO: reimplement Water class using tesselation shaders
// generally calmer water. options for rounded/pointed peaks or directional/circular waves
class Water {
    public:
        vector<float> vertices;
        vector<unsigned int> indices;

        Water(int px, int pz, int pw, int pl, int pdimx, int pdimz, float maxA, int maxI, bool dir, bool rnd, bool anim);

        void setupMesh();
        void updateMesh();
        void updateTime(float dT);

        void draw(Shader* shader, unsigned int cubeTexture);

        // wave equations
        float W(int i, float x, float y, float t);
        float H(float x, float y, float t);

        // partials
        float ddxW(int i, float x, float y, float t);
        float ddxH(float x, float y, float t);
        float ddyW(int i, float x, float y, float t);
        float ddyH(float x, float y, float t);

        // vector space
        glm::vec3 B(float x, float y, float t); // binormal vector
        glm::vec3 T(float x, float y, float t); // tangent vector
        glm::vec3 N(float x, float y, float t); // normal vector

    private:
        float internalTime;
        unsigned int VAO, VBO, EBO;
        
        // px - x position of center of water in world
        // pz - z position of center of water in world
        // pw - width of body of water
        // pl - length of body of water
        // pdimx - number of points in equal distribution in x direction of water
        // pdimz - number of points in equal distribution in z direction of water
        int pX, pZ, pW, pL, pDimX, pDimZ;
        float maxA; int maxI;

        bool directional;
        bool rounded;
        bool animated;

        // wave information
        // wave: W(x, y, t) = Ai sin (Di dot (x, y) * wi + Si * wi * t)
        // surface: H(x, y, t) = sum of all waves i
        vector<float> Ai;       // amplitude of wave
        vector<float> wi;       // frequency of wave
        vector<glm::vec2> Di;   // horizontal direction vector of wave
        vector<float> Si;       // phase-constant = S * 2/L = S * w
};

// rougher seas. variations on intensity
class Gerstner {

};

#endif
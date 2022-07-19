/**
 * @file water.cpp
 * @author Eron Ristich (eron@ristich.com)
 * @brief Stores vertex (fragment) information of a simple plane of water. Variations implement various functions detailed in GG1-C1, for directional/circular waves, pointed/rounded crests, etc.
 * @version 0.1
 * @date 2022-06-11
 * 
 * @copyright Copyright (c) 2022
 */

#include "water.h"

/**
 * @brief Return a random float from 0 to x
 * 
 * @param x Maximum bound of random range
 * @return Random float from 0 to x
 */
float randFloat(float x) {
    return (float)rand() / ((float)RAND_MAX / x);
}

/**
 * @brief Construct a new default Water object
 * 
 * @param px X position of the center of the rectangular body of water
 * @param pz Z position of the center of the rectangular body of water
 * @param pw Width of the rectangular body of water
 * @param pl Length of the rectangular body of water
 * @param pdimx Number of points in mesh in equal distribution along x dimension of the water
 * @param pdimz Number of points in mesh in equal distribution along y dimension of the water
 * @param maxA Maximum amplitude of any individual wave
 * @param maxI Maximum number of waves
 * @param dir Whether or not all waves are directional or circular (directional - true, circular - false)
 * @param rnd Whether or not all waves are rounded or pointed (rounded - true, pointed - false)
 * @param anim Whether or not the water updates with time (default false)
 */
Water::Water(int px, int pz, int pw, int pl, int pdimx, int pdimz, float maxA, int maxI, bool dir, bool rnd, bool anim=false) : pX(px), pZ(pz), pW(pw), pL(pl), pDimX(pdimx), pDimZ(pdimz), maxA(maxA), maxI(maxI), directional(dir), rounded(rnd), animated(anim) {
    // use srand to set arbitrary seed (commented out for testing)
    // srand(time(NULL));
    
    // directional/rounded waves
    if (dir && rnd) {
        // define set of waves
        for (int i = 0; i < maxI; i ++) {
            Ai.push_back(randFloat(maxA));
            wi.push_back(randFloat(MAXFREQ)*0.5+MAXFREQ*0.5);
            Di.push_back(glm::vec2(randFloat(1.0f)*2-1, randFloat(1.0f)*2-1));
            Si.push_back(randFloat(MAXSPED)*0.5+MAXFREQ*0.5);
        }

        // initialize internal time
        internalTime = 0;

        // setup wave mesh
        setupMesh();
    }
}

/**
 * @brief Wave basis equation for some wave i. Follows W(x, y, t) = Ai sin (Di dot (x, y) * wi + Si * wi * t)
 * 
 * @param i ith wave in the set of waves. Assumed to be within bounds
 * @param x x coordinate of the point of evaluation
 * @param y y coordinate of the point of evaluation
 * @param t time elapsed
 * @return float 
 */
float Water::W(int i, float x, float y, float t) {
    float A = Ai.at(i);
    float w = wi.at(i);
    glm::vec2 D = Di.at(i);
    float S = Si.at(i);

    if (directional && rounded) {    
        return A * sin (glm::dot(D, glm::vec2(x, y)) * w + S * w * t);
    }
}

/**
 * @brief Sum of waves equation
 * 
 * @param x x coordinate of the point of evaluation
 * @param y y coordinate of the point of evaluation
 * @param t time elapsed
 * @return float 
 */
float Water::H(float x, float y, float t) {
    // does not change based off of wave type (H is always the sum of all waves Wi)
    float summ = 0;
    for (int i = 0; i < maxI; i ++) {
        summ += W(i, x, y, t);
    }
    return summ;
}

/**
 * @brief Takes the partial in respect to x of some wave i
 * 
 * @param i ith wave in the set of waves. Assumed to be within bounds
 * @param x x coordinate of the point of evaluation
 * @param y y coordinate of the point of evaluation
 * @param t time elapsed
 * @return float 
 */
float Water::ddxW(int i, float x, float y, float t) {
    // wi * Di.x * Ai * cos (Di dot (x, y) * wi + Si * wi * t)
    float A = Ai.at(i);
    float w = wi.at(i);
    glm::vec2 D = Di.at(i);
    float S = Si.at(i);

    if (directional && rounded) {
        return w * D.x * A * cos(glm::dot(D, glm::vec2(x, y)) * w + S * w * t);
    }
}

/**
 * @brief Takes the partial in respect to x of the sum of waves
 * 
 * @param x x coordinate of the point of evaluation
 * @param y y coordinate of the point of evaluation
 * @param t time elapsed
 * @return float 
 */
float Water::ddxH(float x, float y, float t) {
    // does not change based off of wave type (H is always the sum of all waves Wi)
    float summ = 0;
    for (int i = 0; i < maxI; i ++) {
        summ += ddxW(i, x, y, t);
    }
    return summ;
}

/**
 * @brief Takes the partial in respect to y of some wave i
 * 
 * @param i ith wave in the set of waves. Assumed to be within bounds
 * @param x x coordinate of the point of evaluation
 * @param y y coordinate of the point of evaluation
 * @param t time elapsed
 * @return float 
 */
float Water::ddyW(int i, float x, float y, float t) {
    // wi * Di.y * Ai * cos (Di dot (x, y) * wi + Si * wi * t)
    float A = Ai.at(i);
    float w = wi.at(i);
    glm::vec2 D = Di.at(i);
    float S = Si.at(i);

    if (directional && rounded) {
        return w * D.y * A * cos(glm::dot(D, glm::vec2(x, y)) * w + S * w * t);
    }
}

/**
 * @brief Takes the partial in respect to y of the sum of waves
 * 
 * @param x x coordinate of the point of evaluation
 * @param y y coordinate of the point of evaluation
 * @param t time elapsed
 * @return float 
 */
float Water::ddyH(float x, float y, float t) {
    float summ = 0;
    for (int i = 0; i < maxI; i ++) {
        summ += ddyW(i, x, y, t);
    }
    return summ;
}

/**
 * @brief Calculates the binormal vector of the sum of waves at a given point
 * 
 * @param x x coordinate of the point of evaluation
 * @param y y coordinate of the point of evaluation
 * @param t time elapsed
 * @return glm::vec3 
 */
glm::vec3 Water::B(float x, float y, float t) {
    glm::vec3 returned = glm::vec3(1, 0, ddxH(x, y, t));
    return returned;
}

/**
 * @brief Calculates the tangent vector of the sum of waves at a given point
 * 
 * @param x x coordinate of the point of evaluation
 * @param y y coordinate of the point of evaluation
 * @param t time elapsed
 * @return glm::vec3 
 */
glm::vec3 Water::T(float x, float y, float t) {
    glm::vec3 returned = glm::vec3(0, 1, ddyH(x, y, t));
    return returned;
}

/**
 * @brief Calculates the normal vector of the sum of waves at a given point
 * 
 * @param x x coordinate of the point of evaluation
 * @param y y coordinate of the point of evaluation
 * @param t time elapsed
 * @return glm::vec3 
 */
glm::vec3 Water::N(float x, float y, float t) {
    glm::vec3 returned = glm::vec3(0 - ddxH(x, y, t), 0 - ddyH(x, y, t), 1);
    return returned;
}

/**
 * @brief Updates internal clock of water object
 * 
 * @param dT time elapsed since last update
 */
void Water::updateTime(float dT) {
    internalTime += dT;
}

/**
 * @brief Setup the mesh after wave functions have been initialized
 */
void Water::setupMesh() {
    // setup vertices
    for (int i = 0; i < pDimX; i ++) {
        for (int j = 0; j < pDimZ; j ++) {
            // evaluate x and z (or just use i and j)
            float x = pX - pW / 2 + (float)i * pW / pDimX;
            float z = pZ - pL / 2 + (float)j * pL / pDimZ;
            
            // compute H / update vertices
            float y = H(x, z, internalTime);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // compute N / update normals
            glm::vec3 normal = N(x, z, internalTime);
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
        }
    }

    // setup indices
    for(int i = 0; i < pDimZ - 1; i ++) {
        for(int j = 0; j < pDimX; j ++) {
            for(int k = 0; k < 2; k ++) {
                indices.push_back(j + pDimX * (i + k));
            }
        }
    }

    // register/update buffers
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_DYNAMIC_DRAW);
}

/**
 * @brief Updates the mesh given current internal time and wave functions
 */
void Water::updateMesh() {
    // clear vertices/indices
    vertices.clear();
    indices.clear();

    // setup vertices
    for (int i = 0; i < pDimX; i ++) {
        for (int j = 0; j < pDimZ; j ++) {
            // evaluate x and z (or just use i and j)
            float x = pX - pW / 2 + (float)i * pW / pDimX;
            float z = pZ - pL / 2 + (float)j * pL / pDimZ;
            
            // compute H / update vertices
            float y = H(x, z, internalTime);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // compute N / update normals
            glm::vec3 normal = N(x, z, internalTime);
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
        }
    }

    // update buffers
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), &vertices[0]);
}

/**
 * @brief Draws the mesh
 * 
 * @param shader 
 */
void Water::draw(Shader* shader, unsigned int cubeTexture) {
    glBindVertexArray(VAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexture);

    // render the mesh triangle strip by triangle strip - each row at a time
    for(int i = 0; i < pDimZ-1; i ++) {
        glDrawElements(GL_TRIANGLE_STRIP, pDimX, GL_UNSIGNED_INT, 
            (void*)(sizeof(unsigned int) * pDimX * i));
    }
}
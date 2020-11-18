#ifndef _BLOCK_H
#define _BLOCK_H

#include <functional>
#include <map>
#include "model_instance_and_type.h"
#include "../src/modelrendering.h"

void drawBlockGeneric(bool drawXplus, bool drawXminus, bool drawZplus, bool drawZminus){
    glUniform1i(object_id_uniform, CORRIDOR);
    DrawVirtualObject("Floor_1");
    if (drawZminus) DrawVirtualObject("WallY+_4");
    if (drawZplus) DrawVirtualObject("WallY-_5");
    DrawVirtualObject("Ceiling_6");

    glUniform1i(object_id_uniform, CORRIDOR2);
    if (drawXplus) DrawVirtualObject("WallX+_2");
    if (drawXminus) DrawVirtualObject("WallX-_3");
}

std::function<void()> createBlockDrawFunction (bool drawXplus, bool drawXminus, bool drawZplus, bool drawZminus) {
    return [=]() { 
        drawBlockGeneric(drawXplus, drawXminus, drawZplus, drawZminus);
    };
}

std::vector<BoundingBox> createBlockBoundingBoxes(
    bool isXplusOpen,
    bool isXminusOpen, 
    bool isZplusOpen, 
    bool isZminusOpen) {
    std::vector<BoundingBox> boundings = {};

    if(isXplusOpen) boundings.push_back(BoundingBox(glm::vec3(1.0, 0.0, -1.0), glm::vec3(1.0, 2.0, 1.0)));
    if(isXminusOpen) boundings.push_back(BoundingBox(glm::vec3(-1.0, 0.0, -1.0), glm::vec3(-1.0, 2.0, 1.0)));
    if(isZplusOpen) boundings.push_back(BoundingBox(glm::vec3(-1.0, 0.0, 1.0), glm::vec3(1.0, 2.0, 1.0)));
    if(isZminusOpen) boundings.push_back(BoundingBox(glm::vec3(-1.0, 0.0, -1.0), glm::vec3(1.0, 2.0, -1.0)));

    return boundings;
}

ModelType createBlockModelType(
    bool drawXplus,
    bool drawXminus,
    bool drawZplus,
    bool drawZminus) {

    return ModelType(
        glm::vec3(1.0, 1.0, 1.0), //scale
        createBlockDrawFunction(drawXplus, drawXminus, drawZplus, drawZminus),
        createBlockBoundingBoxes(drawXplus, drawXminus, drawZplus, drawZminus)
    );
}

ModelType Blocks[] = {
    createBlockModelType(false, false, false, false),
    createBlockModelType(false, false, false, true),
    createBlockModelType(false, false, true, false),
    createBlockModelType(false, false, true, true),
    createBlockModelType(false, true, false, false),
    createBlockModelType(false, true, false, true),
    createBlockModelType(false, true, true, false),
    createBlockModelType(false, true, true, true),
    createBlockModelType(true, false, false, false),
    createBlockModelType(true, false, false, true),
    createBlockModelType(true, false, true, false),
    createBlockModelType(true, false, true, true),
    createBlockModelType(true, true, false, false),
    createBlockModelType(true, true, false, true),
    createBlockModelType(true, true, true, false),
    createBlockModelType(true, true, true, true),
};

ModelType* getBlock(bool Xplus, bool Xminus, bool Zplus, bool Zminus){
    int id = Xplus * 8 + Xminus * 4 + Zplus * 2 + Zminus * 1;
    return &Blocks[id];
}

#define BLOCK_SIZE 2

ModelInstance createBlock(glm::vec2 position, bool Xplus, bool Xminus, bool Yplus, bool Yminus){
    return ModelInstance(getBlock(Xminus, Xplus, Yplus, Yminus), glm::vec4(-position.x * BLOCK_SIZE, 0.0, position.y * BLOCK_SIZE, 1.0), BLOCK_SIZE / 2.0);
}

void spawnMaze(std::vector<ModelInstance>* instances) {
    std::vector<ModelInstance> mapBlocks = {

        createBlock(glm::vec2(9, 6), true, true, true, false),
        createBlock(glm::vec2(9, 5), true, true, false, false),
        createBlock(glm::vec2(9, 4), true, true, false, false),
        createBlock(glm::vec2(3, 3), false, true, true, false),
        createBlock(glm::vec2(4, 3), false, false, true, true),
        createBlock(glm::vec2(5, 3), true, false, true, false),
        createBlock(glm::vec2(9, 3), true, true, false, false),
        createBlock(glm::vec2(3, 2), true, true, false, false),
        createBlock(glm::vec2(5, 2), true, true, false, false),
        createBlock(glm::vec2(7, 2), false, true, true, false),
        createBlock(glm::vec2(8, 2), false, false, true, false),
        createBlock(glm::vec2(9, 2), false, false, false, false),
        createBlock(glm::vec2(10, 2), false, false, true, false),
        createBlock(glm::vec2(11, 2), true, false, true, false),
        createBlock(glm::vec2(3, 1), true, true, false, false),
        createBlock(glm::vec2(5, 1), true, true, false, false),
        createBlock(glm::vec2(7, 1), false, true, false, false),
        createBlock(glm::vec2(8, 1), false, false, false, false),
        createBlock(glm::vec2(9, 1), false, false, false, false),
        createBlock(glm::vec2(10, 1), false, false, false, false),
        createBlock(glm::vec2(11, 1), false, false, false, false),
        createBlock(glm::vec2(12, 1), false, false, true, true),
        createBlock(glm::vec2(13, 1), false, false, true, true),
        createBlock(glm::vec2(14, 1), true, false, true, true),
        createBlock(glm::vec2(0, -0), false, true, true, true),
        createBlock(glm::vec2(1, -0), false, false, true, true),
        createBlock(glm::vec2(2, -0), false, false, true, true),
        createBlock(glm::vec2(3, -0), false, false, false, false),
        createBlock(glm::vec2(4, -0), false, false, true, true),
        createBlock(glm::vec2(5, -0), false, false, false, false),
        createBlock(glm::vec2(6, -0), false, false, true, true),
        createBlock(glm::vec2(7, -0), false, false, false, false),
        createBlock(glm::vec2(8, -0), false, false, false, false),
        createBlock(glm::vec2(9, -0), false, false, false, false),
        createBlock(glm::vec2(10, -0), false, false, false, false),
        createBlock(glm::vec2(11, -0), true, false, false, false),
        createBlock(glm::vec2(3, -1), true, true, false, false),
        createBlock(glm::vec2(5, -1), true, true, false, false),
        createBlock(glm::vec2(7, -1), false, true, false, false),
        createBlock(glm::vec2(8, -1), false, false, false, false),
        createBlock(glm::vec2(9, -1), false, false, false, false),
        createBlock(glm::vec2(10, -1), false, false, false, false),
        createBlock(glm::vec2(11, -1), false, false, false, false),
        createBlock(glm::vec2(12, -1), false, false, true, true),
        createBlock(glm::vec2(13, -1), false, false, true, true),
        createBlock(glm::vec2(14, -1), true, false, true, true),
        createBlock(glm::vec2(3, -2), true, true, false, false),
        createBlock(glm::vec2(5, -2), true, true, false, false),
        createBlock(glm::vec2(7, -2), false, true, false, true),
        createBlock(glm::vec2(8, -2), false, false, false, true),
        createBlock(glm::vec2(9, -2), false, false, false, false),
        createBlock(glm::vec2(10, -2), false, false, false, true),
        createBlock(glm::vec2(11, -2), true, false, false, true),
        createBlock(glm::vec2(3, -3), false, true, false, true),
        createBlock(glm::vec2(4, -3), false, false, true, true),
        createBlock(glm::vec2(5, -3), true, false, false, true),
        createBlock(glm::vec2(9, -3), true, true, false, false),
        createBlock(glm::vec2(9, -4), true, true, false, false),
        createBlock(glm::vec2(9, -5), true, true, false, false),
        createBlock(glm::vec2(9, -6), true, true, false, true),

    };

    while(mapBlocks.size() > 0){
        instances->push_back(mapBlocks[mapBlocks.size() - 1]);
        mapBlocks.pop_back();
    }
}

#endif // _BLOCK_H
// vim: set spell spelllang=pt_br :

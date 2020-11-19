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

    createBlock(glm::vec2(24, 7), false, true, true, false),
    createBlock(glm::vec2(25, 7), true, false, true, false),
    createBlock(glm::vec2(24, 6), true, true, false, false),
    createBlock(glm::vec2(25, 6), true, true, false, false),
    createBlock(glm::vec2(35, 6), true, true, true, false),
    createBlock(glm::vec2(21, 5), true, true, true, false),
    createBlock(glm::vec2(22, 5), false, true, true, false),
    createBlock(glm::vec2(23, 5), false, false, true, true),
    createBlock(glm::vec2(24, 5), true, false, false, false),
    createBlock(glm::vec2(25, 5), true, true, false, false),
    createBlock(glm::vec2(26, 5), false, true, true, false),
    createBlock(glm::vec2(27, 5), false, false, true, true),
    createBlock(glm::vec2(28, 5), true, false, true, false),
    createBlock(glm::vec2(34, 5), false, true, true, false),
    createBlock(glm::vec2(35, 5), false, false, false, false),
    createBlock(glm::vec2(36, 5), true, false, true, false),
    createBlock(glm::vec2(21, 4), false, true, false, true),
    createBlock(glm::vec2(22, 4), true, false, false, false),
    createBlock(glm::vec2(23, 4), false, true, true, false),
    createBlock(glm::vec2(24, 4), true, false, false, false),
    createBlock(glm::vec2(25, 4), true, true, false, false),
    createBlock(glm::vec2(26, 4), false, true, false, false),
    createBlock(glm::vec2(27, 4), true, false, true, false),
    createBlock(glm::vec2(28, 4), true, true, false, false),
    createBlock(glm::vec2(33, 4), false, true, true, false),
    createBlock(glm::vec2(34, 4), false, false, false, false),
    createBlock(glm::vec2(35, 4), false, false, false, false),
    createBlock(glm::vec2(36, 4), false, false, false, false),
    createBlock(glm::vec2(37, 4), true, false, true, false),
    createBlock(glm::vec2(3, 3), false, true, true, false),
    createBlock(glm::vec2(4, 3), false, false, true, true),
    createBlock(glm::vec2(5, 3), true, false, true, false),
    createBlock(glm::vec2(21, 3), false, true, true, true),
    createBlock(glm::vec2(22, 3), true, false, false, true),
    createBlock(glm::vec2(23, 3), true, true, false, true),
    createBlock(glm::vec2(24, 3), true, true, false, false),
    createBlock(glm::vec2(25, 3), true, true, false, true),
    createBlock(glm::vec2(26, 3), true, true, false, false),
    createBlock(glm::vec2(27, 3), true, true, false, true),
    createBlock(glm::vec2(28, 3), true, true, false, false),
    createBlock(glm::vec2(32, 3), false, true, true, false),
    createBlock(glm::vec2(33, 3), false, false, false, false),
    createBlock(glm::vec2(34, 3), false, false, false, false),
    createBlock(glm::vec2(35, 3), false, false, false, false),
    createBlock(glm::vec2(36, 3), false, false, false, false),
    createBlock(glm::vec2(37, 3), true, false, false, false),
    createBlock(glm::vec2(3, 2), true, true, false, false),
    createBlock(glm::vec2(5, 2), true, true, false, false),
    createBlock(glm::vec2(11, 2), false, true, true, false),
    createBlock(glm::vec2(12, 2), false, false, true, true),
    createBlock(glm::vec2(13, 2), false, false, true, true),
    createBlock(glm::vec2(14, 2), false, false, true, true),
    createBlock(glm::vec2(15, 2), false, false, true, true),
    createBlock(glm::vec2(16, 2), true, false, true, true),
    createBlock(glm::vec2(20, 2), false, true, true, false),
    createBlock(glm::vec2(21, 2), false, false, true, false),
    createBlock(glm::vec2(22, 2), true, false, true, false),
    createBlock(glm::vec2(24, 2), false, true, false, false),
    createBlock(glm::vec2(25, 2), false, false, true, false),
    createBlock(glm::vec2(26, 2), false, false, false, true),
    createBlock(glm::vec2(27, 2), true, false, true, false),
    createBlock(glm::vec2(28, 2), false, true, false, true),
    createBlock(glm::vec2(29, 2), false, false, true, true),
    createBlock(glm::vec2(30, 2), false, false, true, true),
    createBlock(glm::vec2(31, 2), false, false, true, true),
    createBlock(glm::vec2(32, 2), false, false, false, false),
    createBlock(glm::vec2(33, 2), false, false, false, false),
    createBlock(glm::vec2(34, 2), false, false, false, false),
    createBlock(glm::vec2(35, 2), false, false, false, false),
    createBlock(glm::vec2(36, 2), false, false, false, false),
    createBlock(glm::vec2(37, 2), false, false, false, false),
    createBlock(glm::vec2(3, 1), true, true, false, false),
    createBlock(glm::vec2(5, 1), true, true, false, false),
    createBlock(glm::vec2(7, 1), false, true, true, false),
    createBlock(glm::vec2(8, 1), false, false, true, false),
    createBlock(glm::vec2(9, 1), true, false, true, false),
    createBlock(glm::vec2(11, 1), true, true, false, false),
    createBlock(glm::vec2(12, 1), true, true, true, false),
    createBlock(glm::vec2(13, 1), true, true, true, false),
    createBlock(glm::vec2(14, 1), true, true, true, false),
    createBlock(glm::vec2(15, 1), true, true, true, false),
    createBlock(glm::vec2(16, 1), true, true, true, false),
    createBlock(glm::vec2(19, 1), false, true, true, false),
    createBlock(glm::vec2(20, 1), false, false, false, false),
    createBlock(glm::vec2(21, 1), false, false, false, false),
    createBlock(glm::vec2(22, 1), false, false, false, false),
    createBlock(glm::vec2(23, 1), true, false, true, false),
    createBlock(glm::vec2(24, 1), true, true, false, true),
    createBlock(glm::vec2(25, 1), true, true, false, false),
    createBlock(glm::vec2(26, 1), false, true, true, true),
    createBlock(glm::vec2(27, 1), true, false, false, true),
    createBlock(glm::vec2(28, 1), true, true, true, false),
    createBlock(glm::vec2(32, 1), false, true, false, true),
    createBlock(glm::vec2(33, 1), false, false, false, false),
    createBlock(glm::vec2(34, 1), false, false, false, false),
    createBlock(glm::vec2(35, 1), false, false, false, false),
    createBlock(glm::vec2(36, 1), false, false, false, false),
    createBlock(glm::vec2(37, 1), true, false, false, false),
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
    createBlock(glm::vec2(10, -0), false, false, true, true),
    createBlock(glm::vec2(11, -0), false, false, false, true),
    createBlock(glm::vec2(12, -0), false, false, false, true),
    createBlock(glm::vec2(13, -0), false, false, false, true),
    createBlock(glm::vec2(14, -0), false, false, false, true),
    createBlock(glm::vec2(15, -0), false, false, false, true),
    createBlock(glm::vec2(16, -0), false, false, false, true),
    createBlock(glm::vec2(17, -0), false, false, true, true),
    createBlock(glm::vec2(18, -0), false, false, true, true),
    createBlock(glm::vec2(19, -0), false, false, false, false),
    createBlock(glm::vec2(20, -0), false, false, false, false),
    createBlock(glm::vec2(21, -0), false, false, false, false),
    createBlock(glm::vec2(22, -0), false, false, false, false),
    createBlock(glm::vec2(23, -0), false, false, false, false),
    createBlock(glm::vec2(24, -0), false, false, true, true),
    createBlock(glm::vec2(25, -0), false, false, false, false),
    createBlock(glm::vec2(26, -0), false, false, true, true),
    createBlock(glm::vec2(27, -0), false, false, true, true),
    createBlock(glm::vec2(28, -0), false, false, false, false),
    createBlock(glm::vec2(29, -0), true, false, true, true),
    createBlock(glm::vec2(33, -0), false, true, false, true),
    createBlock(glm::vec2(34, -0), false, false, false, false),
    createBlock(glm::vec2(35, -0), false, false, false, false),
    createBlock(glm::vec2(36, -0), false, false, false, false),
    createBlock(glm::vec2(37, -0), true, false, false, true),
    createBlock(glm::vec2(3, -1), true, true, false, false),
    createBlock(glm::vec2(5, -1), true, true, false, false),
    createBlock(glm::vec2(7, -1), false, true, false, true),
    createBlock(glm::vec2(8, -1), false, false, false, true),
    createBlock(glm::vec2(9, -1), true, false, false, true),
    createBlock(glm::vec2(19, -1), false, true, false, true),
    createBlock(glm::vec2(20, -1), false, false, false, false),
    createBlock(glm::vec2(21, -1), false, false, false, false),
    createBlock(glm::vec2(22, -1), false, false, false, false),
    createBlock(glm::vec2(23, -1), true, false, false, true),
    createBlock(glm::vec2(24, -1), false, true, true, true),
    createBlock(glm::vec2(25, -1), true, false, false, true),
    createBlock(glm::vec2(28, -1), true, true, false, true),
    createBlock(glm::vec2(34, -1), false, true, false, true),
    createBlock(glm::vec2(35, -1), false, false, false, false),
    createBlock(glm::vec2(36, -1), true, false, false, true),
    createBlock(glm::vec2(3, -2), true, true, false, false),
    createBlock(glm::vec2(5, -2), true, true, false, false),
    createBlock(glm::vec2(20, -2), false, true, false, true),
    createBlock(glm::vec2(21, -2), false, false, false, true),
    createBlock(glm::vec2(22, -2), true, false, false, true),
    createBlock(glm::vec2(35, -2), true, true, false, true),
    createBlock(glm::vec2(3, -3), false, true, false, true),
    createBlock(glm::vec2(4, -3), false, false, true, true),
    createBlock(glm::vec2(5, -3), true, false, false, true),
    };

    while(mapBlocks.size() > 0){
        instances->push_back(mapBlocks[mapBlocks.size() - 1]);
        mapBlocks.pop_back();
    }
}

#endif // _BLOCK_H
// vim: set spell spelllang=pt_br :

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
    return ModelInstance(getBlock(Xminus, Xplus, Yplus, Yminus), glm::vec4(-position.x * BLOCK_SIZE, 0.0, position.y * BLOCK_SIZE, 1.0));
}

#endif // _BLOCK_H
// vim: set spell spelllang=pt_br :

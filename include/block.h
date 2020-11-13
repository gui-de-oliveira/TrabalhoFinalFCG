#ifndef _BLOCK_H
#define _BLOCK_H

#include <functional>
#include "model_instance_and_type.h"
#include "../src/modelrendering.h"

void drawBlockGeneric(bool drawXplus, bool drawXminus, bool drawZplus, bool drawZminus){
    DrawVirtualObject("Floor_1");
    if (drawZminus) DrawVirtualObject("WallY+_4");
    if (drawZplus) DrawVirtualObject("WallY-_5");
    if (drawXplus) DrawVirtualObject("WallX+_2");
    if (drawXminus) DrawVirtualObject("WallX-_3");
    DrawVirtualObject("Ceiling_6");
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

ModelType Block_ZminusOpen = createBlockModelType(true, true, true, false);
ModelType Block_ZplusOpen = createBlockModelType(true, true, false, true);
ModelType Block_XminusOpen = createBlockModelType(true, false, true, true);
ModelType Block_XbothOpen = createBlockModelType(false, false, true, true);
ModelType Block_XplusAndZOpen = createBlockModelType(false, true, false, false);

#endif // _BLOCK_H
// vim: set spell spelllang=pt_br :

#ifndef _MODEL_H
#define _MODEL_H

#include <cmath>
#include <cstdio>
#include <cstdlib>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

class BoundingBox {
    public:
    glm::vec4 min;
    glm::vec4 max;

    BoundingBox(glm::vec4 center, float size) {
        min = glm::vec4(center.x - size, center.y - size, center.z - size, 1.0);
        max = glm::vec4(center.x + size, center.y + size, center.z + size, 1.0);
    }

    BoundingBox(glm::vec4 _min, glm::vec4 _max) {
        min = _min;
        max = _max;
    }

    BoundingBox(glm::vec3 _min, glm::vec3 _max) {
        min = glm::vec4(_min.x, _min.y, _min.z, 1.0);
        max = glm::vec4(_max.x, _max.y, _max.z, 1.0);
    }

    BoundingBox(BoundingBox bounding, glm::vec4 _position){
        min = bounding.min + glm::vec4(_position.x, _position.y, _position.z, 0.0);
        max = bounding.max + glm::vec4(_position.x, _position.y, _position.z, 0.0);
    }

    BoundingBox(float minx, float miny, float minz, float maxx, float maxy, float maxz){
        min = glm::vec4(minx, miny, minz, 1.0);
        max = glm::vec4(maxx, maxy, maxz, 1.0);
    }
};

class ModelType
{
    public:
    std::function<void()> drawObject;
    glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0);
    std::vector<BoundingBox> boundings = {};

    ModelType(glm::vec3 _scale, void (*_drawObject)()) {
        scale = _scale;
        drawObject = _drawObject;
    }

    ModelType(glm::vec3 _scale, void (*_drawObject)(), std::vector<BoundingBox> _boundings) {
        scale = _scale;
        drawObject = _drawObject;
        boundings = _boundings;
    }

    ModelType(glm::vec3 _scale, std::function<void()> _drawObject, std::vector<BoundingBox> _boundings) {
        scale = _scale;
        drawObject = _drawObject;
        boundings = _boundings;
    }
};

class ModelInstance
{
    public:
    glm::vec4 position;
    glm::vec3 rotation = glm::vec3(0.0, 0.0, 0.0);
    glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0);
    ModelType* object;

    ModelInstance(ModelType* _object, glm::vec4 _position, glm::vec3 _rotation, float _scale) { _setup(_object, _position, _rotation, scale * _scale); }
    ModelInstance(ModelType* _object, glm::vec4 _position, glm::vec3 _rotation) { _setup(_object, _position, _rotation, scale); }
    ModelInstance(ModelType* _object, glm::vec4 _position, float _scale) { _setup(_object, _position, rotation, scale * _scale); }
    ModelInstance(ModelType* _object, glm::vec4 _position) { _setup(_object, _position, rotation, scale); }

    private:
    void _setup(ModelType* _object, glm::vec4 _position, glm::vec3 _rotation, glm::vec3 _scale){
        position = _position;
        rotation = _rotation;
        scale = _scale;
        object = _object;
    }
};

bool doObjectsCollide(BoundingBox* obj1, BoundingBox* obj2) {
    bool isIntersectingX = obj1->min.x <= obj2->max.x && obj2->min.x <= obj1->max.x ;
    bool isIntersectingY = obj1->min.y <= obj2->max.y && obj2->min.y <= obj1->max.y ;
    bool isIntersectingZ = obj1->min.z <= obj2->max.z && obj2->min.z <= obj1->max.z ;

    return isIntersectingX && isIntersectingY && isIntersectingZ;
}

bool doObjectCollidesWithArray(BoundingBox* obj, BoundingBox array[], int arraySize) {
    
    for(int i = 0; i < arraySize; i++){
        if(doObjectsCollide(obj, &array[i])){
            return true;
        }
    }
    return false;
}

bool doObjectCollidesWithInstance(BoundingBox* obj, ModelInstance* instance) {
    std::vector<BoundingBox> boundings = instance->object->boundings;

    for(int i = 0; i < boundings.size(); i++){
        BoundingBox bounding = boundings[i];
        BoundingBox b = BoundingBox(bounding, instance->position);

        if(doObjectsCollide(obj, &b)){
            return true;
        }
    }
    return false;
}

bool doObjectCollidesWithInstancesArray(BoundingBox* obj, std::vector<ModelInstance> instances) {
    for(int i = 0; i < instances.size(); i++){
        if(doObjectCollidesWithInstance(obj, &instances[i])){
            return true;
        }
    }
    return false;
}

#endif // _MODEL_H
// vim: set spell spelllang=pt_br :

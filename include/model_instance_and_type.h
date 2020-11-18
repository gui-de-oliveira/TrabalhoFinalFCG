#ifndef _MODEL_H
#define _MODEL_H

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

#define PI 3.1415
#define HALF_PI PI / 2.0

#define SPHERE 0
#define BUNNY  1
#define PLANE  2
#define LINK  3
#define CORRIDOR 4
#define DRAGON 5
#define REAPER 6
#define LUCINA 7
#define CORRIDOR2 8

#define FRAMES_REAPER 1 //max = 112
#define FRAMES_LUCINA_WALKING 1  //max = 30
#define FRAMES_LUCINA_IDLE 1  //max = 72
#define FRAMES_LUCINA_LEFT_WALKING 1  //max = 30
#define FRAMES_LUCINA_DEATH 1  //max = 50

float vectorLength(glm::vec4 v){
    return sqrt(pow(v.x, 2) + pow(v.y, 2) + pow(v.z, 2));
}
class BoundingBox {
    public:
    glm::vec4 min;
    glm::vec4 max;

    BoundingBox(glm::vec4 center, float size) {
        min = glm::vec4(center.x - size, center.y - size, center.z - size, 1.0);
        max = glm::vec4(center.x + size, center.y + size, center.z + size, 1.0);
    }

    BoundingBox(glm::vec4 v1, glm::vec4 v2) {
        min = glm::vec4(std::min(v1.x, v2.x), std::min(v1.y, v2.y), std::min(v1.z, v2.z), 1.0);
        max = glm::vec4(std::max(v1.x, v2.x), std::max(v1.y, v2.y), std::max(v1.z, v2.z), 1.0);
    }

    BoundingBox(glm::vec3 v1, glm::vec3 v2) {
        min = glm::vec4(std::min(v1.x, v2.x), std::min(v1.y, v2.y), std::min(v1.z, v2.z), 1.0);
        max = glm::vec4(std::max(v1.x, v2.x), std::max(v1.y, v2.y), std::max(v1.z, v2.z), 1.0);
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
    glm::vec3 rotation = glm::vec3(0.0, 0.0, 0.0);
    std::vector<BoundingBox> boundings = {};

    ModelType(glm::vec3 _scale, void (*_drawObject)()) {
        scale = _scale;
        drawObject = _drawObject;
    }

    ModelType(glm::vec3 _scale, glm::vec3 _rotation, void (*_drawObject)()) {
        scale = _scale;
        rotation = _rotation;
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

bool doesRayCollidesWithAnyWall(std::vector<ModelInstance> allInstances, glm::vec4 start, glm::vec4 end)
{
    glm::vec4 rayDirection  = end - start;
    BoundingBox rayBoundingBox = BoundingBox(end, start);

    //Get all instances colliding with the bounding box
    std::vector<ModelInstance> collidingInstances = {};
    while(allInstances.size() > 0)
    {
        ModelInstance instance = allInstances[allInstances.size() - 1];

        if (doObjectCollidesWithInstance(&rayBoundingBox, &instance)) {
            collidingInstances.push_back(instance);
        }

        allInstances.pop_back();
    }
    
    //Search for wall colliding with ray
    while(collidingInstances.size() > 0)
    {
        ModelInstance wall = collidingInstances[collidingInstances.size() - 1];
        collidingInstances.pop_back();

        std::vector<BoundingBox> boundings = wall.object->boundings;
        for(int i = 0; i < boundings.size(); i++) {
            BoundingBox relativeBounding = BoundingBox(boundings[i], wall.position - start);

            //Horizontal line
            if(relativeBounding.min.z == relativeBounding.max.z) 
            {
                float t = relativeBounding.max.z / rayDirection.z;
                float x = rayDirection.x * t;

                //Intersects with the line segment
                if (t >= 0 && relativeBounding.min.x <= x && x <= relativeBounding.max.x) 
                {
                    return true;
                }
            } 

            //Vertical line
            else 
            {
                float t = relativeBounding.max.x / rayDirection.x;
                float z = rayDirection.z * t;

                //Intersects with the line segment
                if (t >= 0 && relativeBounding.min.z <= z && z <= relativeBounding.max.z) 
                {
                    return true;
                }
            }
        }
    }

    return false;
};

void drawEnemy(){
    glUniform1i(object_id_uniform, REAPER);

    float speed = 50.0;
    int frame = (int) (glfwGetTime() * speed) % FRAMES_REAPER;
    string name = ("foe120_model" + to_string(frame));

    DrawVirtualObject(name.c_str());
}

void drawSphere(){
    glUniform1i(object_id_uniform, LINK);
    DrawVirtualObject("Sphere");
}

void drawDragon(){
    glUniform1i(object_id_uniform, DRAGON);

    DrawVirtualObject("Body__ArmMT");
    DrawVirtualObject("Body__BodyMT");
    DrawVirtualObject("HeadUpper__HeadMT");
    DrawVirtualObject("EyeBall__EyeMT");
    DrawVirtualObject("Jaw__HeadMT");
    DrawVirtualObject("Toungue__HeadMT");
    DrawVirtualObject("LowerBody__BodyMT");
    DrawVirtualObject("Maku__HeadMT");
    DrawVirtualObject("Wing__WingMT");
    DrawVirtualObject("WingFrame__WingMT");
}

ModelType Enemy = ModelType(glm::vec3(1.0, 1.0, 1.0), glm::vec3(HALF_PI, 0.0, HALF_PI + PI), drawEnemy);
ModelType Sphere = ModelType(glm::vec3(1.0, 1.0, 1.0), drawSphere);
ModelType Dragon = ModelType(glm::vec3(0.01, 0.01, 0.01), drawDragon);

#endif // _MODEL_H
// vim: set spell spelllang=pt_br :

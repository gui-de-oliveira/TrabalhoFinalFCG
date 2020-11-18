#ifndef _REAPER_H
#define _REAPER_H

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

#include "model_instance_and_type.h"

class Reaper {
    public:
    float totalTime = 0.0;
    ModelInstance* instance;
    std::vector<ModelInstance>* instances;
    
    Reaper(ModelInstance* _instance, std::vector<ModelInstance>* _instances) {
        instance = _instance;
        instances = _instances;
    };

    glm::vec4 getReaperPath(float time) {
        //Transforma valor time em um valor do intervalo entre 0 e maxValue,
        //repetindo ao chega em maxValue
        auto repeating = [](float time, float maxValue){
            float t = floor(time/maxValue);
            return time - t * maxValue;
        };

        float height = 1.0;
        float width = 1.5;

        float dt = repeating(time, 4);
        float ds = repeating(dt, 1);

        auto p = [=](float x, float y){
            return glm::vec2(x * width, y * height);
        };

        glm::vec2 point =
            dt < 1 ? getCubicBezierPoint(p(1, -1), p(0, -1), p(0, 1), p(1, 1), ds)
            : dt < 2 ? getCubicBezierPoint(p(1, 1), p(1, 1), p(2, 1), p(2, 1), ds)
            : dt < 3 ? getCubicBezierPoint(p(2, 1), p(3, 1), p(3, -1), p(2, -1), ds)
                    : getCubicBezierPoint(p(2, -1), p(2, -1), p(1, -1), p(1, -1), ds);

        return glm::vec4(point.x, 0.0, point.y, 1.0);
    };

    glm::vec4 getPositionDisplacement(float delta) {
        float distance = delta * 1.0;

        glm::vec4 initialPos = getReaperPath(totalTime);
        glm::vec4 currentPos = initialPos;
        float totalDistance = 0.0;

        while(totalDistance < distance){
            totalTime += 0.0001;
            glm::vec4 dist = getReaperPath(totalTime);

            glm::vec4 diff = dist - currentPos;
            totalDistance += sqrt(pow(diff.x, 2) + pow(diff.y, 2) + pow(diff.z, 2));
            currentPos = dist;
        }

        glm::vec4 diffor = currentPos - initialPos;
        return diffor;
    };

    void applyEnemyBehaviour(float delta, bool* isGameLost, glm::vec4 playerPosition)
    {
        glm::vec4 displacement = getPositionDisplacement(delta);
        instance->position += displacement;
        glm::vec2 displacement2D = glm::vec2(displacement.x, displacement.z);

        float angle = calculateAngle(displacement2D, glm::vec2(1.0, 0.0));
        instance->rotation.z = -angle;

        glm::vec4 playerDirection = playerPosition - instance->position;
        if(vectorLength(playerDirection) < 50.0) {
            glm::vec2 playerDirection2D = glm::vec2(playerDirection.x, playerDirection.z);
            float playerAngle = fabs(calculateAngle(displacement2D, playerDirection2D)) * (180.0/PI);

            if(playerAngle < 45.0){
                //check for walls on the way
                bool doesRayCollide = doesRayCollidesWithAnyWall(*instances, playerPosition, instance->position);
                (*isGameLost) = !doesRayCollide;
            }
        }
    }
};

#endif // _REAPER_H
// vim: set spell spelllang=pt_br :

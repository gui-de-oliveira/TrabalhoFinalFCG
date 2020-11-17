#include <cmath>
#include <cstdio>
#include <cstdlib>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

glm::vec4 UP_VECTOR = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
class Camera
{
    public:
    glm::vec4 position;
    float phi;
    float theta;

    Camera (float x, float y, float z, float _phi, float _theta);

    glm::vec4 getDirection() {
        return glm::vec4(
            cos(phi) * sin(theta),
            sin(phi),
            cos(phi) * cos(theta),
            0.0f);
    };

    glm::vec4 getRelativeLeft() {
        glm::vec4 relativeLeft = crossproduct(UP_VECTOR, getDirection());
        relativeLeft /= norm(relativeLeft);
        return relativeLeft;
    }

    glm::vec4 getRelativeForward() {
        glm::vec4 relativeForward = crossproduct(getRelativeLeft(), UP_VECTOR);
        relativeForward /= norm(relativeForward);
        return relativeForward;
    }   
};

Camera::Camera (float _x, float _y, float _z, float _phi, float _theta) {
    position = glm::vec4(_x, _y, _z, 1.0f);
    phi = _phi;
    theta = _theta;
}

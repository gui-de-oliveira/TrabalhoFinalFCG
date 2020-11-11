#include <cmath>
#include <cstdio>
#include <cstdlib>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

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
};

Camera::Camera (float _x, float _y, float _z, float _phi, float _theta) {
    position = glm::vec4(_x, _y, _z, 1.0f);
    phi = _phi;
    theta = _theta;
}

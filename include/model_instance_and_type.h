#include <cmath>
#include <cstdio>
#include <cstdlib>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

class ModelType
{
    public:
    void (*drawObject)();
    glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0);

    ModelType(glm::vec3 _scale, void (*_drawObject)()) {
        scale = _scale;
        drawObject = _drawObject;
    }
};

class ModelInstance
{
    public:
    glm::vec4 position;
    glm::vec3 rotation;
    glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0);
    ModelType* object;

    ModelInstance(ModelType* _object, glm::vec4 _position, glm::vec3 _rotation, glm::vec3 _scale) {
        scale = _scale;
        position = _position;
        rotation = _rotation;
        object = _object;
        ModelInstance(_object, _position, _rotation);
    }

    ModelInstance(ModelType* _object, glm::vec4 _position, glm::vec3 _rotation) {
        position = _position;
        rotation = _rotation;
        object = _object;
    }
};



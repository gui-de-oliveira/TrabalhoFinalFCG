#include <cmath>
#include <cstdio>
#include <cstdlib>

#include <map>
#include <stack>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <functional>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

// Headers da biblioteca para carregar modelos obj
#include <stb_image.h>

// Headers locais, definidos na pasta "include/"
#include "utils.h"
#include "matrices.h"
#include "textrendering.h"
#include "modelrendering.h"
#include "camera.h"
#include "model_instance_and_type.h"
#include "block.h"

// Funções abaixo renderizam como texto na janela OpenGL algumas matrizes e
// outras informações do programa. Definidas após main().
void TextRendering_ShowFramesPerSecond(GLFWwindow* window);
void PrintStringTopLeft(GLFWwindow* window, string text);
void PrintStringCenter(GLFWwindow* window, string text);
void PrintStringCenter(GLFWwindow* window, string text, float scale);

void DrawWorld(bool drawPlayer, bool drawCamera);
bool collisionDetect (Camera g_PlayerCamera);

#define BLOCK_SIZE 2

#define SPHERE 0
#define BUNNY  1
#define PLANE  2
#define LINK  3
#define CORRIDOR 4

int g_InstanceSelectedId = 0;
std::vector<ModelInstance> instances = {};
ModelInstance* g_InstanceSelected;

Camera INITIAL_PLAYER_CAMERA(0.0, 1.0, -2.75, -0.24, 18.85);
Camera g_PlayerCamera = INITIAL_PLAYER_CAMERA;
Camera g_FixedCamera(-1.052, 1.917, 2.0, -0.28, 20.39);

bool g_ShouldPlayerRotate = true;

enum ActionMode {
    PLAY,
    EDIT
};

struct ButtonStates {
    bool RightMouse = false;

    bool MovingForward = false;
    bool MovingBackward = false;
    bool MovingLeft = false;
    bool MovingRight = false;
    bool MovingUp = false;
    bool MovingDown = false;

    bool Shift = false;
    bool Ctrl = false;
    
    bool ESC = false;

    bool Z = false;
    bool X = false;
    bool C = false;
    bool V = false;

    bool R = false;

    bool IsCursorDisabled = false;

    ActionMode InputMode = PLAY;
} g_ButtonState;

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
    // ==============
    // Não modifique este loop! Ele é utilizando para correção automatizada dos
    // laboratórios. Deve ser sempre o primeiro comando desta função KeyCallback().
    for (int i = 0; i < 10; ++i)
        if (key == GLFW_KEY_0 + i && action == GLFW_PRESS && mod == GLFW_MOD_SHIFT)
            std::exit(100 + i);
    // ==============

    auto IsButtonPressed = [&](int assignedKey){
        return key == assignedKey && (action == GLFW_PRESS || action == GLFW_REPEAT);
    };

    // Se o usuário pressionar a tecla ESC, mostramos o mouse.
    if (IsButtonPressed(GLFW_KEY_ESCAPE)){
        g_ButtonState.IsCursorDisabled = !g_ButtonState.IsCursorDisabled;

        if(g_ButtonState.IsCursorDisabled) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    if (IsButtonPressed(GLFW_KEY_LEFT))
    {
        g_InstanceSelectedId--;

        if(g_InstanceSelectedId < 0){
            g_InstanceSelectedId = 0;
        }

        g_InstanceSelected = &instances[g_InstanceSelectedId];
    }

    if (IsButtonPressed(GLFW_KEY_RIGHT))
    {
        g_InstanceSelectedId++;

        if(g_InstanceSelectedId > instances.size() - 1){
            g_InstanceSelectedId = 0;
        }

        g_InstanceSelected = &instances[g_InstanceSelectedId];
    }

    auto updateButtonState = [&](bool* button, int assignedKey){
        if(key == assignedKey){
            *button = action == GLFW_PRESS || action == GLFW_REPEAT;
        }
    };

    //Update movement buttons
    updateButtonState(&g_ButtonState.MovingForward, GLFW_KEY_W);
    updateButtonState(&g_ButtonState.MovingBackward, GLFW_KEY_S);
    updateButtonState(&g_ButtonState.MovingLeft, GLFW_KEY_A);
    updateButtonState(&g_ButtonState.MovingRight, GLFW_KEY_D);
    updateButtonState(&g_ButtonState.MovingUp, GLFW_KEY_Q);
    updateButtonState(&g_ButtonState.MovingDown, GLFW_KEY_E);

    updateButtonState(&g_ButtonState.Shift, GLFW_KEY_LEFT_SHIFT);
    updateButtonState(&g_ButtonState.Ctrl, GLFW_KEY_LEFT_CONTROL);

    //Other butons
    updateButtonState(&g_ButtonState.Z, GLFW_KEY_Z);
    updateButtonState(&g_ButtonState.X, GLFW_KEY_X);
    updateButtonState(&g_ButtonState.C, GLFW_KEY_C);
    updateButtonState(&g_ButtonState.V, GLFW_KEY_V);
    updateButtonState(&g_ButtonState.R, GLFW_KEY_R);

    if(IsButtonPressed(GLFW_KEY_P)) g_ButtonState.InputMode = PLAY;
    if(IsButtonPressed(GLFW_KEY_O)) g_ButtonState.InputMode = EDIT;

    // Se o usuário apertar a tecla R, recarregamos os shaders dos arquivos "shader_fragment.glsl" e "shader_vertex.glsl".
    if (IsButtonPressed(GLFW_KEY_R))
    {
        LoadShadersFromFiles();
        fprintf(stdout,"Shaders recarregados!\n");
        fflush(stdout);
    }
}

double g_LastCursorPosX, g_LastCursorPosY;

// Função callback chamada sempre que o usuário aperta algum dos botões do mouse
void MouseButtonCallback(GLFWwindow* window, int key, int action, int mods)
{
    auto updateButtonState = [&](bool* button, int assignedKey){
        if(key == assignedKey){
            *button = action == GLFW_PRESS || action == GLFW_REPEAT;
        }
    };

    updateButtonState(&g_ButtonState.RightMouse, GLFW_MOUSE_BUTTON_RIGHT);
    if(action == GLFW_PRESS) glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
}

void cursorPosCallbackOnGameLost(GLFWwindow* window, double xpos, double ypos)
{
    float dx = xpos - g_LastCursorPosX;
    float dy = ypos - g_LastCursorPosY;

    // Atualizamos parâmetros da câmera com os deslocamentos
    g_PlayerCamera.theta -= 0.01f*dx;
    g_PlayerCamera.phi   -= 0.01f*dy;

    // Em coordenadas esféricas, o ângulo phi deve ficar entre -pi/2 e +pi/2.
    float phimax = 3.141592f/2;
    float phimin = 0.1;

    if (g_PlayerCamera.phi > phimax)
        g_PlayerCamera.phi = phimax;

    if (g_PlayerCamera.phi < phimin)
        g_PlayerCamera.phi = phimin;

    g_LastCursorPosX = xpos;
    g_LastCursorPosY = ypos;
};

void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    if(!g_ButtonState.IsCursorDisabled){
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;

        // Atualizamos parâmetros da câmera com os deslocamentos
        g_PlayerCamera.theta -= 0.01f*dx;
        g_PlayerCamera.phi   -= 0.01f*dy;

        // Em coordenadas esféricas, o ângulo phi deve ficar entre -pi/2 e +pi/2.
        float phimax = 3.141592f/2;
        float phimin = -phimax;

        if (g_PlayerCamera.phi > phimax)
            g_PlayerCamera.phi = phimax;

        if (g_PlayerCamera.phi < phimin)
            g_PlayerCamera.phi = phimin;

    }

    // Atualizamos as variáveis globais para armazenar a posição atual do
    // cursor como sendo a última posição conhecida do cursor.
    g_LastCursorPosX = xpos;
    g_LastCursorPosY = ypos;
}

// Dado o tempo de execução, retorna o offset de posição X do inimigo
float positionXByTime(float time){
    return sin(time);
}

float vectorLength(glm::vec4 v){
    return sqrt(pow(v.x, 2) * pow(v.y, 2) * pow(v.z, 2));
}

// Checa se é possível mover o personagem na direção mostrada e o move
void tryToMove(glm::vec4 direction);

glm::vec2 getCubicBezierPoint(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, float dt){
    auto interpolate = [=](float n1, float n2 , float perc ) {
        float diff = n2 - n1;
        return n1 + ( diff * perc );
    };

    float xa = interpolate(p1.x, p2.x, dt);
    float ya = interpolate(p1.y, p2.y, dt);
    float xb = interpolate(p2.x, p3.x, dt);
    float yb = interpolate(p2.y, p3.y, dt);
    float xc = interpolate(p3.x, p4.x, dt);
    float yc = interpolate(p3.y, p4.y, dt);

    float xm = interpolate(xa, xb, dt);
    float ym = interpolate(ya, yb, dt);
    float xn = interpolate(xb, xc, dt);
    float yn = interpolate(yb, yc, dt);

    float x = interpolate(xm , xn , dt);
    float y = interpolate(ym , yn , dt);

    return glm::vec2(x, y);
};

glm::vec4 getPath(float time) {
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

float totalTime = 0.0;
glm::vec4 getEnemyPosition(float delta) {
    float distance = delta * 1.0;

    glm::vec4 initialPos = getPath(totalTime);
    glm::vec4 currentPos = initialPos;
    float totalDistance = 0.0;

    while(totalDistance < distance){
        totalTime += 0.0001;
        glm::vec4 dist = getPath(totalTime);

        glm::vec4 diff = dist - currentPos;
        totalDistance += sqrt(pow(diff.x, 2) + pow(diff.y, 2) + pow(diff.z, 2));
        currentPos = dist;
    }

    glm::vec4 diffor = currentPos - initialPos;
    return diffor;
};

float calculateAngle(glm::vec2 v1, glm::vec2 v2){
    float dot = v1.x * v2.x + v1.y * v2.y;
    float det = v1.x * v2.y - v1.y * v2.x;

    return atan2(det, dot);
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

enum PlayerState {
    MOVING_FORWARD,
    MOVING_BACKWARD,
    MOVING_LEFT,
    MOVING_RIGHT,
    IDLE
};

PlayerState playerState = IDLE;
float frameCounter = 0;
std::string playerModel = "";

int Game(GLFWwindow* window, float* width, float* height, float* screenRatio )
{
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);

    g_PlayerCamera = INITIAL_PLAYER_CAMERA;
    totalTime = 0.0;

    instances =
    {
        createBlock(glm::vec2(0, -1), true, true, false, true),
        createBlock(glm::vec2(0, 0), true, false, false, false),
        createBlock(glm::vec2(0, 1), true, false, false, false),
        createBlock(glm::vec2(0, 2), true, true, false, false),

        createBlock(glm::vec2(-1, 0), false, false, false, true),
        createBlock(glm::vec2(-1, 1), false, false, true, false),

        createBlock(glm::vec2(-2, 0), false, false, false, true),
        createBlock(glm::vec2(-2, 1), false, false, true, false),

        createBlock(glm::vec2(-3, 0), false, true, false, true),
        createBlock(glm::vec2(-3, 1), false, true, true, false),
    };
    g_InstanceSelected = &instances[0];

    auto pushToInstances = [&](ModelInstance** ptr, ModelInstance modelInstance){
        instances.push_back(modelInstance);
        (*ptr) = &instances[instances.size() - 1];
    };

    ModelInstance *enemyInstance, *dragonInstance, *endGame;
    pushToInstances(&enemyInstance, ModelInstance(&Enemy, glm::vec4(3.0, 0.0, 0.0, 1.0), 0.0056));
    pushToInstances(&dragonInstance, ModelInstance(&Dragon, glm::vec4(0.0, -1.15, 15.0, 1.0), glm::vec3(0.0, PI, 0.0), 13.4));
    pushToInstances(&endGame, ModelInstance(&Sphere, glm::vec4(0.0, 0.5, 4.0, 1.0), glm::vec3(0.0, 0.0, 0.0), 3.0));

    bool isGameWon = false;
    bool isGameLost = false;

    glm::vec4 offset;
    float initialTime = glfwGetTime();
    float currentTime = glfwGetTime() - initialTime;

    // Ficamos em loop, renderizando, até que o usuário feche a janela
    while (!glfwWindowShouldClose(window) && !isGameWon && !isGameLost)
    {
        std::stringstream debugStream;

        float lastTime = currentTime;
        currentTime = glfwGetTime() - initialTime;
        float delta = currentTime - lastTime;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glViewport(0, 0, (*width), (*height));

        //Pintamos tudo de branco e reiniciamos o Z-BUFFER
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program_id);

        // Calcula a distância que deve ser percorrida desde o último frame
        float deltaDistance = positionXByTime(currentTime) - positionXByTime(lastTime);

        glm::vec4 d = (endGame->position - g_PlayerCamera.position);
        float distance = sqrt(d.x * d.x + d.y * d.y + d.z * d.z);

        if(!g_ButtonState.IsCursorDisabled){
            glm::vec4 direction = getEnemyPosition(delta);
            enemyInstance->position += direction;
            glm::vec2 enemyDirectionTopDown = glm::vec2(direction.x, direction.z);

            float angle = calculateAngle(enemyDirectionTopDown, glm::vec2(1.0, 0.0));

            enemyInstance->rotation.z = -angle;

            auto offset = [=](float speed, float amp){ return (sin(currentTime * speed) - sin(lastTime * speed)) * amp; };

            dragonInstance->position.y += deltaDistance;

            if(!isGameWon){
                endGame->scale *= 1 + offset(2.5, 0.3);
            }

            if(!isGameWon && distance < 0.5){
                cout << "You won!!!";
                endGame->scale = glm::vec3(0.0, 0.0, 0.0);
                isGameWon = true;
            }

            glm::vec4 relativeEnemyPosition = g_PlayerCamera.position - enemyInstance->position;
           
            if(vectorLength(relativeEnemyPosition) < 50.0) {

                glm::vec2 relEnemyPositionTopDown = glm::vec2(relativeEnemyPosition.x, relativeEnemyPosition.z);
                glm::vec2 dirction = glm::vec2(direction.x, direction.z);
                float playerAngle = fabs(calculateAngle(dirction, relEnemyPositionTopDown)) * (180.0/PI);

                if(playerAngle < 45.0){
                    //check for walls on the way
                    bool doesRayCollide = doesRayCollidesWithAnyWall(instances, g_PlayerCamera.position, enemyInstance->position);
                    isGameLost = !doesRayCollide;
                }
            }

            // Movimentamos o personagem se alguma tecla estiver pressionada
            float speed = 2.0f * (g_ButtonState.Shift ? 10.0 : 1.0) * (g_ButtonState.Ctrl ? 0.1 : 1.0);

            if(g_ButtonState.MovingForward) tryToMove(speed * delta * g_PlayerCamera.getRelativeForward());
            if(g_ButtonState.MovingBackward) tryToMove(-1.0f * speed * delta * g_PlayerCamera.getRelativeForward());
            if(g_ButtonState.MovingLeft) tryToMove(speed * delta * g_PlayerCamera.getRelativeLeft());
            if(g_ButtonState.MovingRight) tryToMove(-1.0f * speed * delta * g_PlayerCamera.getRelativeLeft());

            if(g_ButtonState.MovingForward || g_ButtonState.MovingLeft) {
                frameCounter += delta;    
                if(playerState == IDLE) frameCounter = 0;

                int frame = (int) (frameCounter * 50.0) % FRAMES_LUCINA_WALKING;
                playerModel = "vsn_mesh_0_body_mesh_mesh_0_body_mesh.001_WALK_" + to_string(frame);
                playerState = MOVING_FORWARD;
            } else if(g_ButtonState.MovingBackward || g_ButtonState.MovingRight) {
                frameCounter -= delta;
                frameCounter += frameCounter < 0 ? FRAMES_LUCINA_WALKING : 0;
                if(playerState == IDLE) frameCounter = 0; 

                int frame = (int) (frameCounter * 50.0) % FRAMES_LUCINA_WALKING;
                playerModel = "vsn_mesh_0_body_mesh_mesh_0_body_mesh.001_WALK_" + to_string(frame);
                playerState = MOVING_BACKWARD;
            } else if(!g_ButtonState.MovingLeft && !g_ButtonState.MovingRight) {
                frameCounter += delta;
                if(playerState != IDLE) frameCounter = 0;  

                int frame = (int) (frameCounter * 50.0) % FRAMES_LUCINA_IDLE;
                playerModel = "vsn_mesh_0_body_mesh_mesh_0_body_mesh.001_IDLE_" + to_string(frame);
                playerState = IDLE;
            }

            if(g_ButtonState.MovingUp) g_PlayerCamera.position += speed * delta * UP_VECTOR;
            if(g_ButtonState.MovingDown) g_PlayerCamera.position -= speed * delta * UP_VECTOR;

            if(g_ButtonState.RightMouse)
            {
                if(g_ButtonState.MovingForward) g_InstanceSelected->position += speed * delta * g_PlayerCamera.getRelativeForward();
                if(g_ButtonState.MovingBackward) g_InstanceSelected->position -= speed * delta * g_PlayerCamera.getRelativeForward();
                if(g_ButtonState.MovingLeft) g_InstanceSelected->position += speed * delta * g_PlayerCamera.getRelativeLeft();
                if(g_ButtonState.MovingRight) g_InstanceSelected->position -= speed * delta * g_PlayerCamera.getRelativeLeft();
                if(g_ButtonState.MovingUp) g_InstanceSelected->position += speed * delta * UP_VECTOR;
                if(g_ButtonState.MovingDown) g_InstanceSelected->position -= speed * delta * UP_VECTOR;
            }

            if(g_ButtonState.Z) g_InstanceSelected->rotation.x += delta * (g_ButtonState.Shift ? -1 : 1);
            if(g_ButtonState.X) g_InstanceSelected->rotation.y += delta * (g_ButtonState.Shift ? -1 : 1);
            if(g_ButtonState.C) g_InstanceSelected->rotation.z += delta * (g_ButtonState.Shift ? -1 : 1);
            if(g_ButtonState.V) g_InstanceSelected->scale *= (g_ButtonState.Shift ? (1 - delta) : (1 + delta));
        }

        glm::mat4 view = Matrix_Camera_View(g_PlayerCamera.position, g_PlayerCamera.getDirection(), UP_VECTOR);

        float nearPlane = -0.1f;  // Posição do "near plane"
        float farPlane  = -500.0f; // Posição do "far plane"
        float fieldOfView = 3.141592 / 3.0f;

        glm::mat4 projection = Matrix_Perspective(fieldOfView, (*screenRatio), nearPlane, farPlane);

        glUniformMatrix4fv(view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));

        DrawWorld(false, true);

        glClear(GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, (*width) * 0.25, (*height) * 0.25);

        view = Matrix_Camera_View(g_FixedCamera.position, g_FixedCamera.getDirection(), UP_VECTOR);
        glUniformMatrix4fv(view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));

        DrawWorld(true, false);

        glViewport(0, 0, (*width), (*height));

        TextRendering_ShowFramesPerSecond(window);

        if (g_ButtonState.InputMode == PLAY) {
            debugStream << "PlayerCamera Stats"<< endl;
            debugStream << "Phi: " << g_PlayerCamera.phi << " Theta:" << g_PlayerCamera.theta << endl;
            debugStream << "X: " << g_PlayerCamera.position.x << " Y:" << g_PlayerCamera.position.y << " Z:" << g_PlayerCamera.position.z << endl;
        } else if (g_ButtonState.InputMode == EDIT) {
            debugStream << "Instance Stats"<< endl;
            debugStream << "ObjectId: " << g_InstanceSelectedId << endl;
            debugStream << "Position X: " << g_InstanceSelected->position.x << " Y:" << g_InstanceSelected->position.y << " Z:" << g_InstanceSelected->position.z << endl;
            debugStream << "Rotation X: " << g_InstanceSelected->rotation.x << " Y:" << g_InstanceSelected->rotation.y << " Z:" << g_InstanceSelected->rotation.z << endl;
            debugStream << "Scale X: " << g_InstanceSelected->scale.x << " Y:" << g_InstanceSelected->scale.y << " Z:" << g_InstanceSelected->scale.z << endl;
        }

        PrintStringTopLeft(window, debugStream.str());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    if(glfwWindowShouldClose(window)) {
        glfwTerminate();
        return 0;
    }

    glViewport(0, 0, (*width), (*height));

    g_ShouldPlayerRotate = false;
    glfwSetCursorPosCallback(window, cursorPosCallbackOnGameLost);

    if(isGameLost) while(!glfwWindowShouldClose(window) && !g_ButtonState.R)
    {
        //Pintamos tudo de branco e reiniciamos o Z-BUFFER
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program_id);

        float r = 2.0;
        float y = g_PlayerCamera.position.y + r * sin(g_PlayerCamera.phi); 
        float z = g_PlayerCamera.position.z + r * cos(g_PlayerCamera.phi) * cos(g_PlayerCamera.theta);
        float x = g_PlayerCamera.position.x + r * cos(g_PlayerCamera.phi) * sin(g_PlayerCamera.theta);

        glm::vec4 camera_position_c  = glm::vec4(x,y,z,1.0f); // Ponto "c", centro da câmera
        glm::vec4 camera_lookat_l    = glm::vec4(g_PlayerCamera.position.x, g_PlayerCamera.position.y, g_PlayerCamera.position.z, 1.0f);
        glm::vec4 camera_view_vector = camera_lookat_l - camera_position_c; // Vetor "view", sentido para onde a câmera está virada

        // Computamos a matriz "View" utilizando os parâmetros da câmera para
        // definir o sistema de coordenadas da câmera.  Veja slides 2-14, 184-190 e 236-242 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
        glm::mat4 view = Matrix_Camera_View(camera_position_c, camera_view_vector, UP_VECTOR);

        float nearPlane = -0.1f;  // Posição do "near plane"
        float farPlane  = -500.0f; // Posição do "far plane"
        float fieldOfView = 3.141592 / 3.0f;

        glm::mat4 projection = Matrix_Perspective(fieldOfView, (*screenRatio), nearPlane, farPlane);

        glUniformMatrix4fv(view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));

        DrawWorld(true, true);

        PrintStringCenter(window, "YOU LOST!\n Press R to play again", 2.0f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    if(isGameWon) while(!glfwWindowShouldClose(window) && !g_ButtonState.R)
    {
        //Pintamos tudo de branco e reiniciamos o Z-BUFFER
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program_id);

        glm::mat4 view = Matrix_Camera_View(g_PlayerCamera.position, g_PlayerCamera.getDirection(), UP_VECTOR);

        float nearPlane = -0.1f;  // Posição do "near plane"
        float farPlane  = -500.0f; // Posição do "far plane"
        float fieldOfView = 3.141592 / 3.0f;

        glm::mat4 projection = Matrix_Perspective(fieldOfView, (*screenRatio), nearPlane, farPlane);

        glUniformMatrix4fv(view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));
        PrintStringCenter(window, "YOU WON THE GAME!\n Press R to play again", 2.0f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}

void DrawWorld(bool drawPlayer, bool drawCamera)
{
    glm::mat4 model = Matrix_Identity(); // Transformação identidade de modelagem

    if (drawPlayer) {
        // Desenhamos o player
        model = Matrix_Translate(g_PlayerCamera.position.x, g_PlayerCamera.position.y, g_PlayerCamera.position.z)
            * Matrix_Translate(0.0, -1.0, 0.0)
            * Matrix_Scale(1.5f, 1.5f, 1.5f);
            
        if (g_ShouldPlayerRotate) {
            model *= Matrix_Rotate_Y(g_PlayerCamera.theta);
        }

        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, LUCINA);
        DrawVirtualObject(playerModel.c_str());
    }

    if (drawCamera) {
        model = Matrix_Translate(g_FixedCamera.position.x, g_FixedCamera.position.y, g_FixedCamera.position.z)
            // * Matrix_Translate(g_FixedCamera.position)
            * Matrix_Rotate_Y(g_FixedCamera.theta)
            * Matrix_Rotate_X(g_FixedCamera.phi + 3.141592 * 0.25)
            * Matrix_Scale(0.01, 0.01, 0.01);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, PLANE);

        DrawVirtualObject("camera_reference");
    }

    for(int i = 0; i < instances.size(); i++) {
        ModelInstance instance =  instances[i];

        model = Matrix_Translate(instance.position.x, instance.position.y, instance.position.z)
            * Matrix_Rotate_X(instance.object->rotation.x)
            * Matrix_Rotate_Y(instance.object->rotation.y)
            * Matrix_Rotate_Z(instance.object->rotation.z)
            * Matrix_Rotate_X(instance.rotation.x)
            * Matrix_Rotate_Y(instance.rotation.y)
            * Matrix_Rotate_Z(instance.rotation.z)
            * Matrix_Scale(instance.scale.x, instance.scale.y, instance.scale.z)
            * Matrix_Scale(instance.object->scale.x, instance.object->scale.y, instance.object->scale.z);

        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        instance.object->drawObject();
    }
}

void tryToMove(glm::vec4 direction){
    //Checa se pode fazer o movimento inteiro
    glm::vec4 newPosition = g_PlayerCamera.position + direction;
    BoundingBox newPositionBB = BoundingBox(newPosition, 0.2);
    if(!doObjectCollidesWithInstancesArray(&newPositionBB, instances)){
        g_PlayerCamera.position = newPosition;
        return;
    }

    //Se não dar para fazer o movimento inteiro, checa se dá para fazer o movimento em X
    glm::vec4 newPositionX = g_PlayerCamera.position + glm::vec4(direction.x, 0.0, 0.0, 0.0);
    BoundingBox newPositionBBx = BoundingBox(newPositionX, 0.2);
    if(!doObjectCollidesWithInstancesArray(&newPositionBBx, instances)){
        g_PlayerCamera.position = newPositionX;
        return;
    }

    //Se não dar para fazer o movimento em X, checa se dá para fazer o movimento em Y
    glm::vec4 newPositionZ = g_PlayerCamera.position + glm::vec4(0.0, 0.0, direction.z, 0.0);
    BoundingBox newPositionBBZ = BoundingBox(newPositionZ, 0.2);
    if(!doObjectCollidesWithInstancesArray(&newPositionBBZ, instances)){
        g_PlayerCamera.position = newPositionZ;
        return;
    }
}

void PrintStringTopLeft(GLFWwindow* window, string text)
{
    float pad = TextRendering_LineHeight(window);

    char buffer[200];
    int lineCount = 0;
    std::string::size_type lastTokenPos = 0;
    std::string::size_type nextTokenPos = 0;
    while(nextTokenPos != std::string::npos)
    {
        lineCount += 1;
        lastTokenPos = nextTokenPos;
        nextTokenPos = text.find_first_of("\n", lastTokenPos + 1);

        snprintf(buffer, 200, text.substr(lastTokenPos, nextTokenPos - lastTokenPos).c_str());
        TextRendering_PrintString(window, buffer, -1.0f+pad/10, 1.0 - lineCount * pad, 1.0f);
    }
}

void PrintStringCenter(GLFWwindow* window, string text, float scale)
{
    float pad = TextRendering_LineHeight(window) * scale;
    float charWidth = TextRendering_CharWidth(window) * scale;

    char buffer[200];
    int lineCount = 0;
    std::string::size_type lastTokenPos = 0;
    std::string::size_type nextTokenPos = 0;
    std::vector<std::string> lines = {};
    while(nextTokenPos != std::string::npos)
    {
        lineCount += 1;
        lastTokenPos = nextTokenPos;
        nextTokenPos = text.find_first_of("\n", lastTokenPos + 1);

        lines.push_back(text.substr(lastTokenPos, nextTokenPos - lastTokenPos));
    }

    int totalLines = lines.size();
    int currLine = 0;
    while(lines.size() > 0)
    {
        std::string text = lines[lines.size() - 1];
        snprintf(buffer, 200, text.c_str());
        TextRendering_PrintString(window, buffer, text.size() * 0.5 * charWidth * -1, currLine * pad - totalLines * 0.5 * pad , scale);

        currLine += 1;
        lines.pop_back();
    }
}

void PrintStringCenter(GLFWwindow* window, string text) {
    PrintStringCenter(window, text, 1.0f);
}


// Escrevemos na tela o número de quadros renderizados por segundo (frames per
// second).
void TextRendering_ShowFramesPerSecond(GLFWwindow* window)
{
    // Variáveis estáticas (static) mantém seus valores entre chamadas
    // subsequentes da função!
    static float old_seconds = (float)glfwGetTime();
    static int   ellapsed_frames = 0;
    static char  buffer[20] = "?? fps";
    static int   numchars = 7;

    ellapsed_frames += 1;

    // Recuperamos o número de segundos que passou desde a execução do programa
    float seconds = (float)glfwGetTime();

    // Número de segundos desde o último cálculo do fps
    float ellapsed_seconds = seconds - old_seconds;

    if ( ellapsed_seconds > 1.0f )
    {
        numchars = snprintf(buffer, 20, "%.2f fps", ellapsed_frames / ellapsed_seconds);

        old_seconds = seconds;
        ellapsed_frames = 0;
    }

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    TextRendering_PrintString(window, buffer, 1.0f-(numchars + 1)*charwidth, 1.0f-lineheight, 1.0f);
}

// set makeprg=cd\ ..\ &&\ make\ run\ >/dev/null
// vim: set spell spelllang=pt_br :


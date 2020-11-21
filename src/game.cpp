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
#include "reaper.h"

// Funções abaixo renderizam como texto na janela OpenGL algumas matrizes e
// outras informações do programa. Definidas após main().
void TextRendering_ShowFramesPerSecond(GLFWwindow* window);
void PrintStringTopLeft(GLFWwindow* window, string text);
void PrintStringCenter(GLFWwindow* window, string text);
void PrintStringCenter(GLFWwindow* window, string text, float scale);

void DrawWorld(bool drawPlayer, bool drawCamera);
bool collisionDetect (Camera g_PlayerCamera);

#define BLOCK_SIZE 2

#define CRYSTAL 0
#define BUNNY  1
#define PLANE  2
#define LINK  3
#define CORRIDOR 4
#define DRONE 9

int g_InstanceSelectedId = 0;
std::vector<ModelInstance> instances = {};
ModelInstance* g_InstanceSelected;

Camera INITIAL_PLAYER_CAMERA(0.0, 1.0, 0.0, -0.28, 11);
Camera g_PlayerCamera = INITIAL_PLAYER_CAMERA;
Camera INITIAL_CAMERA(-0.0, 1.66, 0.0, -0.44, 4.66);
Camera g_FixedCamera = INITIAL_CAMERA;

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

    bool LeftArrow = false;
    bool RightArrow = false;
    bool UpArrow = false;
    bool DownArrow = false;


    bool R = false;

    bool IsCursorEnable = false;

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
        g_ButtonState.IsCursorEnable = !g_ButtonState.IsCursorEnable;

        if(g_ButtonState.IsCursorEnable) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    if (IsButtonPressed(GLFW_KEY_J))
    {
        g_InstanceSelectedId--;

        if(g_InstanceSelectedId < 0){
            g_InstanceSelectedId = instances.size() - 1;
        }

        g_InstanceSelected = &instances[g_InstanceSelectedId];
    }

    if (IsButtonPressed(GLFW_KEY_K))
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

    updateButtonState(&g_ButtonState.LeftArrow, GLFW_KEY_LEFT);
    updateButtonState(&g_ButtonState.RightArrow, GLFW_KEY_RIGHT);
    updateButtonState(&g_ButtonState.UpArrow, GLFW_KEY_UP);
    updateButtonState(&g_ButtonState.DownArrow, GLFW_KEY_DOWN);

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
    if(!g_ButtonState.IsCursorEnable){
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

// Checa se é possível mover o personagem na direção mostrada e o move
void tryToMove(glm::vec4 direction, glm::vec4* position, float size);

enum PlayerState {
    MOVING_FORWARD,
    MOVING_BACKWARD,
    MOVING_SIDEWAYS,
    IDLE,
    DEATH
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
    g_FixedCamera = INITIAL_CAMERA;
    g_ShouldPlayerRotate = true;

    instances = {};
    spawnMaze(&instances);
    g_InstanceSelected = &instances[0];

    auto pushToInstances = [&](ModelInstance** ptr, ModelInstance modelInstance){
        instances.push_back(modelInstance);
        (*ptr) = &instances[instances.size() - 1];
    };

    ModelInstance *dragonInstance, *endGame;
    pushToInstances(&dragonInstance, ModelInstance(&Dragon, glm::vec4(-94.0, -1.15, 3.6, 1.0), glm::vec3(0.0, HALF_PI, 0.0), 13.4));
    pushToInstances(&endGame, ModelInstance(&Crystal, glm::vec4(-75.0, 0.6, 4.0, 1.0), glm::vec3(0.0, 0.0, 0.0), 0.2));

    struct ReaperProps {
        glm::vec4 position;
        glm::vec4 rotation;
        float timeOffset = 0.0;
        MovementPattern MovementPattern = BEZIER_CURVE;
    };

    auto createReaperProps=  [](glm::vec3 position, float timeOffset = 0.0, MovementPattern MovementPattern = BEZIER_CURVE, glm::vec4 rotation = glm::vec4(0.0, 0.0, 0.0, 1.0)){
        ReaperProps props;
        props.position = glm::vec4(position.x, position.y, position.z, 1.0);
        props.timeOffset = timeOffset;
        props.MovementPattern = MovementPattern;
        props.rotation = rotation;

        return props;
    };

    std::vector<ReaperProps> reapersProps = {
        //First part
        createReaperProps(glm::vec3(-8.5, 0.0, -6.0)),
        createReaperProps(glm::vec3(-8.5, 0.0, -6.0), 10.0),

        //Corridor
        createReaperProps(glm::vec3(-23.7, 0.0, 7.5), 3.0, REPEATING_LINE),
        createReaperProps(glm::vec3(-25.7, 0.0, 7.5), 1.0, REPEATING_LINE),
        createReaperProps(glm::vec3(-27.7, 0.0, 7.5), 2.0, REPEATING_LINE),
        createReaperProps(glm::vec3(-29.7, 0.0, 7.5), 1.0, REPEATING_LINE),
        createReaperProps(glm::vec3(-31.7, 0.0, 7.5), 6.0, REPEATING_LINE),
        
        //Maze
        createReaperProps(glm::vec3(-55.87, 0.0, 2.03), 0.0, STOPPED, glm::vec4(0.0, 0.0, 3.655, 1.0)),
        createReaperProps(glm::vec3(-51.93, 0.0, 2.16), 0.0, STOPPED, glm::vec4(0.0, 0.0, -4.31, 1.0)),
        createReaperProps(glm::vec3(-41.92, 0.0, 10.28), 0.0, STOPPED, glm::vec4(0.0, 0.0, -2.30, 1.0)),
        createReaperProps(glm::vec3(-50.14, 0.0, 6.07), 0.0, STOPPED, glm::vec4(0.0, 0.0, 0.82, 1.0)),
    };

    std::vector<Reaper> reapers = {};
    for(int i = 0; i < reapersProps.size(); i++){
        ModelInstance *instance;
        pushToInstances(&instance, ModelInstance(&Enemy, reapersProps[i].position, reapersProps[i].rotation, 0.0056));
        Reaper reaper(instance, &instances, reapersProps[i].timeOffset, reapersProps[i].MovementPattern);
        reapers.push_back(reaper);
    }
    
    bool isGameLost = false;
    bool isGameWon = false;

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

        if(!g_ButtonState.IsCursorEnable){
            auto offset = [=](float speed, float amp){ return (sin(currentTime * speed) - sin(lastTime * speed)) * amp; };

            endGame->scale *= 1 + offset(2.5, 0.3);
            endGame->rotation.y += 0.03;

            if(vectorLength(endGame->position - g_PlayerCamera.position) < 1.0){
                endGame->scale = glm::vec3(0.0, 0.0, 0.0);
                isGameWon = true;
            }

            dragonInstance->position.y += offset(1.0, 1.0);

            for(int i = 0; i < reapers.size(); i++){
                reapers[i].applyEnemyBehaviour(delta, &isGameLost, g_PlayerCamera.position);
            }

            // Movimentamos o personagem se alguma tecla estiver pressionada
            float speed = 2.0f * (g_ButtonState.Shift ? 10.0 : 1.0) * (g_ButtonState.Ctrl ? 0.1 : 1.0);

            if(g_ButtonState.MovingForward) tryToMove(speed * delta * g_PlayerCamera.getRelativeForward(), &g_PlayerCamera.position, 0.2);
            if(g_ButtonState.MovingBackward) tryToMove(-1.0f * speed * delta * g_PlayerCamera.getRelativeForward(), &g_PlayerCamera.position, 0.2);
            if(g_ButtonState.MovingLeft) tryToMove(speed * delta * g_PlayerCamera.getRelativeLeft(), &g_PlayerCamera.position, 0.2);
            if(g_ButtonState.MovingRight) tryToMove(-1.0f * speed * delta * g_PlayerCamera.getRelativeLeft(), &g_PlayerCamera.position, 0.2);

            if(g_ButtonState.MovingForward) {
                frameCounter += delta;    
                if(playerState != MOVING_FORWARD && playerState != MOVING_BACKWARD) frameCounter = 0; 

                int frame = (int) (frameCounter * 50.0) % FRAMES_LUCINA_WALKING;
                playerModel = "vsn_mesh_0_body_mesh_mesh_0_body_mesh.001_WALK_" + to_string(frame);
                playerState = MOVING_FORWARD;
            } 

            else if(g_ButtonState.MovingBackward) {
                frameCounter -= delta;
                frameCounter += frameCounter < 0 ? FRAMES_LUCINA_WALKING : 0;
                if(playerState != MOVING_FORWARD && playerState != MOVING_BACKWARD) frameCounter = 0; 

                int frame = (int) (frameCounter * 50.0) % FRAMES_LUCINA_WALKING;
                playerModel = "vsn_mesh_0_body_mesh_mesh_0_body_mesh.001_WALK_" + to_string(frame);
                playerState = MOVING_BACKWARD;
            }

            else if(g_ButtonState.MovingRight || g_ButtonState.MovingLeft) {
                frameCounter += delta;
                if(playerState != MOVING_SIDEWAYS) frameCounter = 0; 

                int frame = (int) (frameCounter * 50.0) % FRAMES_LUCINA_LEFT_WALKING;
                playerModel = "vsn_mesh_0_body_mesh_mesh_0_body_mesh.001_LEFT_WALK_" + to_string(frame);
                playerState = MOVING_SIDEWAYS;
            }
            
            else {
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

            
            if(g_ButtonState.UpArrow) tryToMove(speed * delta * g_FixedCamera.getRelativeForward(), & g_FixedCamera.position, 0.25);
            if(g_ButtonState.DownArrow) tryToMove(-speed * delta * g_FixedCamera.getRelativeForward(), & g_FixedCamera.position, 0.25);
            if(g_ButtonState.LeftArrow) g_FixedCamera.theta += speed * delta;
            if(g_ButtonState.RightArrow) g_FixedCamera.theta -= speed * delta;

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

        if(g_ButtonState.IsCursorEnable){
            float lineheight = TextRendering_LineHeight(window);
            TextRendering_PrintString(window, "Comandos: ", -0.2f, 0.5f, 2.0f);
            TextRendering_PrintString(window, "WASD -> Mover Player", -0.2f, 0.5f - lineheight*2, 2.0f);
            TextRendering_PrintString(window, "Setas -> Mover Drone ", -0.2f, 0.5f - lineheight*4, 2.0f);
            TextRendering_PrintString(window, "ESC -> Pausar o jogo ", -0.2f, 0.5f - lineheight*6, 2.0f);
        }

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

    if(isGameLost){

    float initialTime = glfwGetTime();
    float currentTime = glfwGetTime() - initialTime;
    frameCounter = 0.0;

    while(!glfwWindowShouldClose(window) && !g_ButtonState.R)
    {
        float lastTime = currentTime;
        currentTime = glfwGetTime() - initialTime;
        float delta = currentTime - lastTime;

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

        frameCounter += delta;    
        int frame = min((int) (frameCounter * 25.0), FRAMES_LUCINA_DEATH - 1);
        playerModel = "vsn_mesh_0_body_mesh_mesh_0_body_mesh.002_DEATH_" + to_string(frame);
        playerState = DEATH;

        DrawWorld(true, true);

        PrintStringCenter(window, "YOU LOST!\n Press R to play again", 2.0f);

        glfwSwapBuffers(window);
        glfwPollEvents();
        }
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
            * Matrix_Translate(0.0, 0.0, 0.0)
            * Matrix_Rotate_Y(g_FixedCamera.theta)
            * Matrix_Rotate_X(g_FixedCamera.phi + 3.141592 * 0.25)
            * Matrix_Scale(0.01, 0.01, 0.01);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, DRONE);

        DrawVirtualObject("drone");
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

void tryToMove(glm::vec4 direction, glm::vec4* position, float size){
    //Checa se pode fazer o movimento inteiro
    glm::vec4 newPosition = *position + direction;
    BoundingBox newPositionBB = BoundingBox(newPosition, size);
    if(!doObjectCollidesWithInstancesArray(&newPositionBB, instances)){
        *position = newPosition;
        return;
    }

    //Se não dar para fazer o movimento inteiro, checa se dá para fazer o movimento em X
    glm::vec4 newPositionX = *position + glm::vec4(direction.x, 0.0, 0.0, 0.0);
    BoundingBox newPositionBBx = BoundingBox(newPositionX, size);
    if(!doObjectCollidesWithInstancesArray(&newPositionBBx, instances)){
        *position = newPositionX;
        return;
    }

    //Se não dar para fazer o movimento em X, checa se dá para fazer o movimento em Y
    glm::vec4 newPositionZ = *position + glm::vec4(0.0, 0.0, direction.z, 0.0);
    BoundingBox newPositionBBZ = BoundingBox(newPositionZ, size);
    if(!doObjectCollidesWithInstancesArray(&newPositionBBZ, instances)){
        *position = newPositionZ;
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


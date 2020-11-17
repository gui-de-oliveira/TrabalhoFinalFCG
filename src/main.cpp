//     Universidade Federal do Rio Grande do Sul
//             Instituto de Informática
//       Departamento de Informática Aplicada
//
//    INF01047 Fundamentos de Computação Gráfica
//               Prof. Eduardo Gastal
//
//                  TRABALHO FINAL
//

// Arquivos "headers" padrões de C podem ser incluídos em um
// programa C++, sendo necessário somente adicionar o caractere
// "c" antes de seu nome, e remover o sufixo ".h". Exemplo:
//    #include <stdio.h> // Em C
//  vira
//    #include <cstdio> // Em C++
//
#include <cmath>
#include <cstdio>
#include <cstdlib>

// Headers abaixo são específicos de C++
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

// Funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void cursorPosCallbackOnGameLost(GLFWwindow* window, double xpos, double ypos);

void DrawWorld(bool drawPlayer, bool drawCamera);
bool collisionDetect (Camera g_PlayerCamera);

#define SPHERE 0
#define BUNNY  1
#define PLANE  2
#define LINK  3
#define CORRIDOR 4

std::vector<string> modelsList = {
    "Link0.obj",
    "camera.obj",
    "king.obj",
    "corridor.obj",
    "corridor2.obj",
    "Block.obj",
    "sphere.obj",
    "dragon.obj",
    "reaper.obj",
};

void drawEnemy(){
    glUniform1i(object_id_uniform, LINK);
    DrawVirtualObject("foe120_model_foe120_model.001_assets_textures_em0C1_t02");
    DrawVirtualObject("foe120_model_foe120_model.001_assets_textures_em0C1_t01");
}

void drawSphere(){
    glUniform1i(object_id_uniform, LINK);
    DrawVirtualObject("Sphere");
}

void drawDragon(){
    glUniform1i(object_id_uniform, LINK);
    DrawVirtualObject("Body__ArmMT_Body__ArmMT_ArmMT");
    DrawVirtualObject("Body__BodyMT_Body__BodyMT_BodyMT");
    DrawVirtualObject("HeadUpper__HeadMT_HeadUpper__HeadMT_HeadMT");
    DrawVirtualObject("EyeBall__EyeMT_EyeBall__EyeMT_EyeMT");
    DrawVirtualObject("Jaw__HeadMT_Jaw__HeadMT_HeadMT.001");
    DrawVirtualObject("Toungue__HeadMT_Toungue__HeadMT_HeadMT.002");
    DrawVirtualObject("LowerBody__BodyMT_LowerBody__BodyMT_BodyMT.001");
    DrawVirtualObject("Maku__HeadMT_Maku__HeadMT_HeadMT.003");
    DrawVirtualObject("Wing__WingMT_Wing__WingMT_WingMT");
    DrawVirtualObject("WingFrame__WingMT_WingFrame__WingMT_WingMT.001");
}

ModelType Enemy = ModelType(glm::vec3(1.0, 1.0, 1.0), drawEnemy);
ModelType Sphere = ModelType(glm::vec3(1.0, 1.0, 1.0), drawSphere);
ModelType Dragon = ModelType(glm::vec3(0.01, 0.01, 0.01), drawDragon);

#define PI 3.1415
#define HALF_PI PI / 2.0
#define BLOCK_SIZE 2

int g_InstanceSelectedId = 0;
std::vector<ModelInstance> instances =
{
    //This item should go first!! [so I know which id is the enemyInstance]
    ModelInstance(&Enemy, glm::vec4(3.0, 0.0, 0.0, 1.0), 0.0056),
    ModelInstance(&Dragon,    glm::vec4(0.0, -1.15, 15.0, 1.0), glm::vec3(0.0, PI, 0.0), 13.4),
    ModelInstance(&Sphere,    glm::vec4(0.0, 0.5, 4.0, 1.0), glm::vec3(0.0, 0.0, 0.0), 3.0),

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

    ModelInstance(&Sphere,    glm::vec4(0.0, 0.0, 0.0, 1.0)),
};
ModelInstance* g_InstanceSelected = &instances[0];

ModelInstance* enemyInstance = &instances[0];
ModelInstance* dragonInstance = &instances[1];
ModelInstance* endGame = &instances[2];

// Abaixo definimos variáveis globais utilizadas em várias funções do código.
// Vetor "up" fixado para apontar para o "céu" (eito Y global)
glm::vec4 UP_VECTOR = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

Camera g_FixedCamera(-1.052, 1.917, 2.0, -0.28, 20.39);
Camera g_PlayerCamera(0.0, 0.55, -2.75, -0.24, 18.85);

glm::vec4 g_CameraRelativeLeft = crossproduct(UP_VECTOR, g_PlayerCamera.getDirection());
glm::vec4 g_CameraRelativeForward =  crossproduct(g_CameraRelativeLeft, UP_VECTOR);

bool g_MovingForward = false;
bool g_MovingBackward = false;
bool g_MovingLeft = false;
bool g_MovingRight = false;
bool g_MovingUp = false;
bool g_MovingDown = false;

bool g_EscapePressed = false;
bool g_IsZPressed = false;
bool g_IsXPressed = false;
bool g_IsCPressed = false;
bool g_IsVPressed = false;

bool g_ModShift = false;
bool g_ModCtrl = false;

bool isCursorDisabled = true;

glm::vec4 g_Position = glm::vec4(-0.06f, 0.0f, 1.90f, 1.0f);
glm::vec3 g_Rotation = glm::vec3(0.0f, 0.0f, 0.0f);

string g_Mode = "PLAYER";

// Razão de proporção da janela (largura/altura). Veja função FramebufferSizeCallback().
float g_ScreenRatio = 1.0f;
float g_Width = 800;
float g_Height= 600;

// "g_LeftMouseButtonPressed = true" se o usuário está com o botão esquerdo do mouse
// pressionado no momento atual. Veja função MouseButtonCallback().
bool g_RightMouseButtonPressed = false; // Análogo para botão direito do mouse
bool g_MiddleMouseButtonPressed = false; // Análogo para botão do meio do mouse

// Variável que controla se o texto informativo será mostrado na tela.
bool g_ShowInfoText = true;

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

bool shouldRotate = true;

int main(int argc, char* argv[])
{
    // Inicializamos a biblioteca GLFW, utilizada para criar uma janela do
    // sistema operacional, onde poderemos renderizar com OpenGL.
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos o callback para impressão de erros da GLFW no terminal
    glfwSetErrorCallback(ErrorCallback);

    // Pedimos para utilizar OpenGL versão 3.3 (ou superior)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Pedimos para utilizar o perfil "core", isto é, utilizaremos somente as
    // funções modernas de OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criamos uma janela do sistema operacional, com 800 colunas e 600 linhas
    // de pixels, e com título "INF01047 ...".
    GLFWwindow* window;
    window = glfwCreateWindow(800, 600, "INF01047 - Seu Cartao - Seu Nome", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos a função de callback que será chamada sempre que o usuário
    // pressionar alguma tecla do teclado ...
    glfwSetKeyCallback(window, KeyCallback);
    // ... ou clicar os botões do mouse ...
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    // ... ou movimentar o cursor do mouse em cima da janela
    glfwSetCursorPosCallback(window, CursorPosCallback);

    // Capturamos o cursor no centro da tela e o dexamos escondido
    glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);

    // Indicamos que as chamadas OpenGL deverão renderizar nesta janela
    glfwMakeContextCurrent(window);

    // Carregamento de todas funções definidas por OpenGL 3.3, utilizando a
    // biblioteca GLAD.
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Definimos a função de callback que será chamada sempre que a janela for
    // redimensionada, por consequência alterando o tamanho do "framebuffer"
    // (região de memória onde são armazenados os pixels da imagem).
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    FramebufferSizeCallback(window, 800, 600); // Forçamos a chamada do callback acima, para definir g_ScreenRatio.

    // Imprimimos no terminal informações sobre a GPU do sistema
    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *glversion   = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    // Carregamos os shaders de vértices e de fragmentos que serão utilizados
    // para renderização. Veja slides 176-196 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
    //
    LoadShadersFromFiles();

    LoadTextureImage("../../data/cameratitlemenu_alb.png");

    // Construímos a representação de objetos geométricos através de malhas de triângulos
    string path = "../../data/";
    for(int i = 0; i < modelsList.size(); i++)
    {
        ObjModel models((path + modelsList[i]).c_str());
        ComputeNormals(&models);
        BuildTrianglesAndAddToVirtualScene(&models);
    }

    if ( argc > 1 )
    {
        ObjModel model(argv[1]);
        BuildTrianglesAndAddToVirtualScene(&model);
    }

    // Inicializamos o código para renderização de texto.
    TextRendering_Init();

    // Habilitamos o Z-buffer. Veja slides 104-116 do documento Aula_09_Projecoes.pdf.
    glEnable(GL_DEPTH_TEST);

    // Habilitamos o Backface Culling. Veja slides 23-34 do documento Aula_13_Clipping_and_Culling.pdf.
    glEnable(GL_CULL_FACE);
    // glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    bool isGameWon = false;
    bool isGameLost = false;

    glm::vec4 offset;
    float currentTime = glfwGetTime();
    float lastTime = currentTime;
    // Ficamos em loop, renderizando, até que o usuário feche a janela
    while (!glfwWindowShouldClose(window) && !isGameWon && !isGameLost)
    {
        float delta = glfwGetTime() - currentTime;
        lastTime = currentTime;
        currentTime = glfwGetTime();

        // Indicamos que queremos renderizar em toda região do framebuffer. A
        // função "glViewport" define o mapeamento das "normalized device
        // coordinates" (NDC) para "pixel coordinates".  Essa é a operação de
        // "Screen Mapping" ou "Viewport Mapping" vista em aula ({+ViewportMapping2+}).
        // glEnable(GL_SCISSOR_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glViewport(0, 0, g_Width, g_Height);

        //Pintamos tudo de branco e reiniciamos o Z-BUFFER
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program_id);

        // Calcula a distância que deve ser percorrida desde o último frame
        float deltaDistance = positionXByTime(currentTime) - positionXByTime(lastTime);

        glm::vec4 d = (endGame->position - g_PlayerCamera.position);
        float distance = sqrt(d.x * d.x + d.y * d.y + d.z * d.z);

        if(!g_EscapePressed){
            glm::vec4 direction = getEnemyPosition(delta);
            enemyInstance->position += direction;
            glm::vec2 enemyDirectionTopDown = glm::vec2(direction.x, direction.z);

            float angle = calculateAngle(enemyDirectionTopDown, glm::vec2(1.0, 0.0));

            enemyInstance->rotation.y = angle + HALF_PI;

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

            //Scan for player

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
            float speed = 2.0f * (g_ModShift ? 10.0 : 1.0) * (g_ModCtrl ? 0.1 : 1.0);

            if(g_MovingForward) tryToMove(speed * delta * g_CameraRelativeForward);
            if(g_MovingBackward) tryToMove(-1.0f * speed * delta * g_CameraRelativeForward);
            if(g_MovingLeft) tryToMove(speed * delta * g_CameraRelativeLeft);
            if(g_MovingRight) tryToMove(-1.0f * speed * delta * g_CameraRelativeLeft);

            if(g_MovingUp) g_PlayerCamera.position += speed * delta * UP_VECTOR;
            if(g_MovingDown) g_PlayerCamera.position -= speed * delta * UP_VECTOR;

            if(g_RightMouseButtonPressed)
            {
                if(g_MovingForward) g_InstanceSelected->position += speed * delta * g_CameraRelativeForward;
                if(g_MovingBackward) g_InstanceSelected->position -= speed * delta * g_CameraRelativeForward;
                if(g_MovingLeft) g_InstanceSelected->position += speed * delta * g_CameraRelativeLeft;
                if(g_MovingRight) g_InstanceSelected->position -= speed * delta * g_CameraRelativeLeft;
                if(g_MovingUp) g_InstanceSelected->position += speed * delta * UP_VECTOR;
                if(g_MovingDown) g_InstanceSelected->position -= speed * delta * UP_VECTOR;
            }

            if(g_IsZPressed) g_InstanceSelected->rotation.x += delta * (g_ModShift ? -1 : 1);
            if(g_IsXPressed) g_InstanceSelected->rotation.y += delta * (g_ModShift ? -1 : 1);
            if(g_IsCPressed) g_InstanceSelected->rotation.z += delta * (g_ModShift ? -1 : 1);
            if(g_IsVPressed) g_InstanceSelected->scale *= (g_ModShift ? (1 - delta) : (1 + delta));
        }

        // Computamos a matriz "View" utilizando os parâmetros da câmera para
        // definir o sistema de coordenadas da câmera.  Veja slides 2-14, 184-190 e 236-242 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
        glm::mat4 view = Matrix_Camera_View(g_PlayerCamera.position, g_PlayerCamera.getDirection(), UP_VECTOR);

        // Note que, no sistema de coordenadas da câmera, os planos near e far
        // estão no sentido negativo! Veja slides 176-204 do documento Aula_09_Projecoes.pdf.
        float nearPlane = -0.1f;  // Posição do "near plane"
        float farPlane  = -500.0f; // Posição do "far plane"
        float fieldOfView = 3.141592 / 3.0f;

        glm::mat4 projection = Matrix_Perspective(fieldOfView, g_ScreenRatio, nearPlane, farPlane);

        // Enviamos as matrizes "view" e "projection" para a placa de vídeo
        // (GPU). Veja o arquivo "shader_vertex.glsl", onde estas são
        // efetivamente aplicadas em todos os pontos.
        glUniformMatrix4fv(view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));

        DrawWorld(false, true);

        glClear(GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, g_Width * 0.25, g_Height * 0.25);

        view = Matrix_Camera_View(g_FixedCamera.position, g_FixedCamera.getDirection(), UP_VECTOR);

        // Enviamos as matrizes "view" e "projection" para a placa de vídeo
        // (GPU). Veja o arquivo "shader_vertex.glsl", onde estas são
        // efetivamente aplicadas em todos os pontos.
        glUniformMatrix4fv(view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));

        DrawWorld(true, false);

        glViewport(0, 0, g_Width, g_Height);

        // Imprimimos na tela informação sobre o número de quadros renderizados
        // por segundo (frames per second).
        TextRendering_ShowFramesPerSecond(window);

        std::stringstream fmt;

        if (g_Mode.compare("PLAYER") == 0) {
            fmt << "PlayerCamera Stats"<< endl;
            fmt << "Phi: " << g_PlayerCamera.phi << " Theta:" << g_PlayerCamera.theta << endl;
            fmt << "X: " << g_PlayerCamera.position.x << " Y:" << g_PlayerCamera.position.y << " Z:" << g_PlayerCamera.position.z << endl;
        } else if (g_Mode.compare("MOVE") == 0) {
            fmt << "Instance Stats"<< endl;
            fmt << "ObjectId: " << g_InstanceSelectedId << endl;
            fmt << "Position X: " << g_InstanceSelected->position.x << " Y:" << g_InstanceSelected->position.y << " Z:" << g_InstanceSelected->position.z << endl;
            fmt << "Rotation X: " << g_InstanceSelected->rotation.x << " Y:" << g_InstanceSelected->rotation.y << " Z:" << g_InstanceSelected->rotation.z << endl;
            fmt << "Scale X: " << g_InstanceSelected->scale.x << " Y:" << g_InstanceSelected->scale.y << " Z:" << g_InstanceSelected->scale.z << endl;
        }

        PrintStringTopLeft(window, fmt.str());

        // O framebuffer onde OpenGL executa as operações de renderização não
        // é o mesmo que está sendo mostrado para o usuário, caso contrário
        // seria possível ver artefatos conhecidos como "screen tearing". A
        // chamada abaixo faz a troca dos buffers, mostrando para o usuário
        // tudo que foi renderizado pelas funções acima.
        // Veja o link: Veja o link: https://en.wikipedia.org/w/index.php?title=Multiple_buffering&oldid=793452829#Double_buffering_in_computer_graphics
        glfwSwapBuffers(window);

        // Verificamos com o sistema operacional se houve alguma interação do
        // usuário (teclado, mouse, ...). Caso positivo, as funções de callback
        // definidas anteriormente usando glfwSet*Callback() serão chamadas
        // pela biblioteca GLFW.
        glfwPollEvents();
    }

    if(glfwWindowShouldClose(window)) {
        glfwTerminate();
        return 0;
    }

    glViewport(0, 0, g_Width, g_Height);

    if(isGameLost) {
    shouldRotate = false;
    glfwSetCursorPosCallback(window, cursorPosCallbackOnGameLost);

    while(!glfwWindowShouldClose(window))
    {
        lastTime = currentTime;
        currentTime = glfwGetTime();
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

        glm::mat4 projection = Matrix_Perspective(fieldOfView, g_ScreenRatio, nearPlane, farPlane);

        glUniformMatrix4fv(view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));

        DrawWorld(true, true);

        PrintStringCenter(window, "YOU LOST!\n Press R to play again", 2.0f);

        glfwSwapBuffers(window);
        glfwPollEvents();
        }
    }

    if(isGameWon) {
    while(!glfwWindowShouldClose(window))
    {
        lastTime = currentTime;
        currentTime = glfwGetTime();
        float delta = currentTime - lastTime;

        //Pintamos tudo de branco e reiniciamos o Z-BUFFER
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program_id);

        glm::mat4 view = Matrix_Camera_View(g_PlayerCamera.position, g_PlayerCamera.getDirection(), UP_VECTOR);

        float nearPlane = -0.1f;  // Posição do "near plane"
        float farPlane  = -500.0f; // Posição do "far plane"
        float fieldOfView = 3.141592 / 3.0f;

        glm::mat4 projection = Matrix_Perspective(fieldOfView, g_ScreenRatio, nearPlane, farPlane);

        glUniformMatrix4fv(view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));
        PrintStringCenter(window, "YOU WON THE GAME!\n Press R to play again", 2.0f);

        glfwSwapBuffers(window);
        glfwPollEvents();
        }
    }

    // Finalizamos o uso dos recursos do sistema operacional
    glfwTerminate();

    // Fim do programa
    return 0;
}

void DrawWorld(bool drawPlayer, bool drawCamera)
{
    glm::mat4 model = Matrix_Identity(); // Transformação identidade de modelagem

    if (drawPlayer) {
        // Desenhamos o player
        model = Matrix_Translate(g_PlayerCamera.position.x, g_PlayerCamera.position.y, g_PlayerCamera.position.z)
            * Matrix_Translate(0.0, -0.5, 0.0)
            * Matrix_Scale(0.5f, 0.5f, 0.5f);
            
        if (shouldRotate) {
            model *= Matrix_Rotate_Y(g_PlayerCamera.theta)
                 * Matrix_Rotate_X(-g_PlayerCamera.phi * 0.5);
        }

        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, LINK);

        DrawVirtualObject("link_model_0");
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
            * Matrix_Rotate_X(instance.rotation.x)
            * Matrix_Rotate_Y(instance.rotation.y)
            * Matrix_Rotate_Z(instance.rotation.z)
            * Matrix_Scale(instance.scale.x, instance.scale.y, instance.scale.z)
            * Matrix_Scale(instance.object->scale.x, instance.object->scale.y, instance.object->scale.z);

        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        instance.object->drawObject();
    }
}

// Definição da função que será chamada sempre que a janela do sistema
// operacional for redimensionada, por consequência alterando o tamanho do
// "framebuffer" (região de memória onde são armazenados os pixels da imagem).
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // Atualizamos também a razão que define a proporção da janela (largura /
    // altura), a qual será utilizada na definição das matrizes de projeção,
    // tal que não ocorra distorções durante o processo de "Screen Mapping"
    // acima, quando NDC é mapeado para coordenadas de pixels. Veja slides 205-215 do documento Aula_09_Projecoes.pdf.
    //
    // O cast para float é necessário pois números inteiros são arredondados ao
    // serem divididos!
    g_ScreenRatio = (float)width / height;

    g_Width = width;
    g_Height = height;
}

// Variáveis globais que armazenam a última posição do cursor do mouse, para
// que possamos calcular quanto que o mouse se movimentou entre dois instantes
// de tempo. Utilizadas no callback CursorPosCallback() abaixo.
double g_LastCursorPosX, g_LastCursorPosY;

// Função callback chamada sempre que o usuário aperta algum dos botões do mouse
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_RightMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_RightMouseButtonPressed = true;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_RightMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_MiddleMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_MiddleMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_MiddleMouseButtonPressed = false;
    }
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

// Função callback chamada sempre que o usuário movimentar o cursor do mouse em
// cima da janela OpenGL.
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    // Abaixo executamos o seguinte: caso o botão esquerdo do mouse esteja
    // pressionado, computamos quanto que o mouse se movimento desde o último
    // instante de tempo, e usamos esta movimentação para atualizar os
    // parâmetros que definem a posição da câmera dentro da cena virtual.
    // Assim, temos que o usuário consegue controlar a câmera.
    if(!g_EscapePressed){
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

        // Atualizamos as novas direções relativas
        g_CameraRelativeLeft = crossproduct(UP_VECTOR, g_PlayerCamera.getDirection());
        g_CameraRelativeLeft /= norm(g_CameraRelativeLeft);

        g_CameraRelativeForward = crossproduct(g_CameraRelativeLeft, UP_VECTOR);
        g_CameraRelativeForward /= norm(g_CameraRelativeForward);
    }

    // Atualizamos as variáveis globais para armazenar a posição atual do
    // cursor como sendo a última posição conhecida do cursor.
    g_LastCursorPosX = xpos;
    g_LastCursorPosY = ypos;

    if (g_RightMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;

        float modifier = g_ModShift ? 0.1f : 1.0f;
        g_Position -= g_CameraRelativeLeft * 0.25f * dx * modifier;
        g_Position -= UP_VECTOR * 0.25f * dy * modifier;

        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }

    if (g_MiddleMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;

        float modifier = g_ModShift ? 0.1f : 1.0f;
        g_Position -= g_CameraRelativeLeft * 0.25f * dx * modifier;
        g_Position -= g_PlayerCamera.getDirection() * 0.25f * dy * modifier;

        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }
}

bool IsActionPressed(int action){
    return action == GLFW_PRESS || action == GLFW_REPEAT;
}

// Definição da função que será chamada sempre que o usuário pressionar alguma
// tecla do teclado. Veja http://www.glfw.org/docs/latest/input_guide.html#input_key
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
    // ==============
    // Não modifique este loop! Ele é utilizando para correção automatizada dos
    // laboratórios. Deve ser sempre o primeiro comando desta função KeyCallback().
    for (int i = 0; i < 10; ++i)
        if (key == GLFW_KEY_0 + i && action == GLFW_PRESS && mod == GLFW_MOD_SHIFT)
            std::exit(100 + i);
    // ==============

    // Se o usuário pressionar a tecla ESC, mostramos o mouse.
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        g_EscapePressed = !g_EscapePressed;
        if(isCursorDisabled) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            isCursorDisabled = false;
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            isCursorDisabled = true;
        }
    }

    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        g_InstanceSelectedId--;

        if(g_InstanceSelectedId < 0){
            g_InstanceSelectedId = 0;
        }

        g_InstanceSelected = &instances[g_InstanceSelectedId];
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        g_InstanceSelectedId++;

        if(g_InstanceSelectedId > instances.size() - 1){
            g_InstanceSelectedId = 0;
        }

        g_InstanceSelected = &instances[g_InstanceSelectedId];
    }

    // Movimentação da câmera
    if (key == GLFW_KEY_W) g_MovingForward = IsActionPressed(action);
    if (key == GLFW_KEY_S) g_MovingBackward = IsActionPressed(action);
    if (key == GLFW_KEY_A) g_MovingLeft = IsActionPressed(action);
    if (key == GLFW_KEY_D) g_MovingRight = IsActionPressed(action);
    if (key == GLFW_KEY_Q) g_MovingUp = IsActionPressed(action);
    if (key == GLFW_KEY_E) g_MovingDown = IsActionPressed(action);

    if (key == GLFW_KEY_LEFT_SHIFT) g_ModShift = IsActionPressed(action);
    if (key == GLFW_KEY_LEFT_CONTROL) g_ModCtrl = IsActionPressed(action);

    if (key == GLFW_KEY_Z) g_IsZPressed = IsActionPressed(action);
    if (key == GLFW_KEY_X) g_IsXPressed = IsActionPressed(action);
    if (key == GLFW_KEY_C) g_IsCPressed = IsActionPressed(action);
    if (key == GLFW_KEY_V) g_IsVPressed = IsActionPressed(action);

    if(key == GLFW_KEY_P && IsActionPressed(action)) g_Mode = "PLAYER";
    if(key == GLFW_KEY_O && IsActionPressed(action)) g_Mode = "MOVE";

    // Se o usuário apertar a tecla H, fazemos um "toggle" do texto informativo mostrado na tela.
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
    {
        g_ShowInfoText = !g_ShowInfoText;
    }

    // Se o usuário apertar a tecla R, recarregamos os shaders dos arquivos "shader_fragment.glsl" e "shader_vertex.glsl".
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        LoadShadersFromFiles();
        fprintf(stdout,"Shaders recarregados!\n");
        fflush(stdout);
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

// Definimos o callback para impressão de erros da GLFW no terminal
void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
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
    if ( !g_ShowInfoText )
        return;

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


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

#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

// Declaração de funções utilizadas para pilha de matrizes de modelagem.
void PushMatrix(glm::mat4 M);
void PopMatrix(glm::mat4& M);

// Funções abaixo renderizam como texto na janela OpenGL algumas matrizes e
// outras informações do programa. Definidas após main().
void TextRendering_ShowFramesPerSecond(GLFWwindow* window);
void PrintStringTopLeft(GLFWwindow* window, string text);

// Funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);

void DrawWorld(bool drawPlayer, bool drawCamera);

#define SPHERE 0
#define BUNNY  1
#define PLANE  2
#define LINK  3
#define CORRIDOR 4

void drawCorridorObject(){
    glUniform1i(object_id_uniform, CORRIDOR);
    DrawVirtualObject("corridor2");
    DrawVirtualObject("securityCamera");
}

void drawHalfCorridorObject(){
    glUniform1i(object_id_uniform, CORRIDOR);
    DrawVirtualObject("box-side-0");
    DrawVirtualObject("box-side-1");
    DrawVirtualObject("box-side-2");
}

void drawWallObject(){
    glUniform1i(object_id_uniform, CORRIDOR);
    DrawVirtualObject("side-0");
    DrawVirtualObject("side-1");
    DrawVirtualObject("side-2");
}

ModelType Corridor = ModelType(glm::vec3(10.0,10.0,10.0), drawCorridorObject);
ModelType HalfCorridor = ModelType(glm::vec3(0.25, 0.25, 0.25), drawHalfCorridorObject);
ModelType Wall = ModelType(glm::vec3(0.25, 0.25, 0.25), drawWallObject);

void drawBlockGeneric(bool drawXplus, bool drawXminus, bool drawZplus, bool drawZminus){
    DrawVirtualObject("Floor_1");
    if (drawZminus) DrawVirtualObject("WallY+_4");
    if (drawZplus) DrawVirtualObject("WallY-_5");
    if (drawXplus) DrawVirtualObject("WallX+_2");
    if (drawXminus) DrawVirtualObject("WallX-_3");
    DrawVirtualObject("Ceiling_6");
}

#define BLOCK_SIZE 2

void drawZminusOpen() { drawBlockGeneric(true, true, true, false); }
void drawZplusOpen() { drawBlockGeneric(true, true, false, true); }
void drawXminusOpen() { drawBlockGeneric(true, false, true, true); }
void drawXbothOpen() { drawBlockGeneric(false, false, true, true); }
void drawZbothAndXplusOpen() { drawBlockGeneric(false, true, false, false); }

ModelType Block_ZminusOpen = ModelType(glm::vec3(1.0, 1.0, 1.0), drawZminusOpen);
ModelType Block_ZplusOpen = ModelType(glm::vec3(1.0, 1.0, 1.0), drawZplusOpen);
ModelType Block_XminusOpen = ModelType(glm::vec3(1.0, 1.0, 1.0), drawXminusOpen);
ModelType Block_XbothOpen = ModelType(glm::vec3(1.0, 1.0, 1.0), drawXbothOpen);
ModelType Block_XplusAndZOpen = ModelType(glm::vec3(1.0, 1.0, 1.0), drawZbothAndXplusOpen);

#define PI 3.1415
#define HALF_PI PI / 2.0

int g_InstanceSelectedId = 0;
ModelInstance instances[] =
{
    //                                            Xpos            Ypos  Zpos
    ModelInstance(&Block_ZplusOpen,     glm::vec4(0.0,            0.0,  0.0,            1.0), glm::vec3(0.0, 0.0, 0.0)),
    ModelInstance(&Block_XplusAndZOpen, glm::vec4(0.0,            0.0,  BLOCK_SIZE,     1.0), glm::vec3(0.0, 0.0, 0.0)),
    ModelInstance(&Block_ZminusOpen,    glm::vec4(0.0,            0.0,  2 * BLOCK_SIZE, 1.0), glm::vec3(0.0, 0.0, 0.0)),
    ModelInstance(&Block_XbothOpen,     glm::vec4(BLOCK_SIZE,     0.0,  BLOCK_SIZE,     1.0), glm::vec3(0.0, 0.0, 0.0)),
    ModelInstance(&Block_XminusOpen,    glm::vec4(2 * BLOCK_SIZE, 0.0,  BLOCK_SIZE,     1.0), glm::vec3(0.0, 0.0, 0.0)),

    // ModelInstance(&Corridor, glm::vec4(-0.571, -2.3, -10.442, 1.0), glm::vec3(0.0, 0.0, 0.0)),
    // ModelInstance(&Corridor, glm::vec4(-0.603, -2.3, -10.265, 1.0), glm::vec3(HALF_PI, 0.0, 0.0)),
    // ModelInstance(&Corridor, glm::vec4(-10.01, -2.3, 3.63, 1.0), glm::vec3(HALF_PI, 0.0, -HALF_PI)),
    // ModelInstance(&HalfCorridor, glm::vec4(3.84216, -2.3, 16.3634, 1.0), glm::vec3(HALF_PI, 0.0, PI)),
    // ModelInstance(&Wall, glm::vec4(-13.267, -2.3, -0.37, 1.0), glm::vec3(HALF_PI, 0.0, 3.0 * HALF_PI)),
    // ModelInstance(&Wall, glm::vec4(16.452, -2.3, 3.122, 1.0), glm::vec3(HALF_PI, 0.0, HALF_PI)),
    // ModelInstance(&Wall, glm::vec4(3.434, -2.3, -13.424, 1.0), glm::vec3(HALF_PI, 0.0, 2.0 * PI)),
};
ModelInstance* g_InstanceSelected = &instances[0];
int maxInstanceId = sizeof(instances)/sizeof(instances[0]) - 1;

// Abaixo definimos variáveis globais utilizadas em várias funções do código.
// Vetor "up" fixado para apontar para o "céu" (eito Y global)
glm::vec4 UP_VECTOR = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

Camera g_FixedCamera(-1.052, 1.917, 2.0, -0.28, 20.39);
Camera g_PlayerCamera(0.0, 0.55, 0.0, -0.24, 18.85);

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
bool g_ModShift = false;
bool g_ModCtrl = false;

bool isCursorDisabled = true;

glm::vec4 g_Position = glm::vec4(-0.06f, 0.0f, 1.90f, 1.0f);
glm::vec3 g_Rotation = glm::vec3(0.0f, 0.0f, 0.0f);

// Pilha que guardará as matrizes de modelagem.
std::stack<glm::mat4>  g_MatrixStack;

string g_Mode = "PLAYER";

// Razão de proporção da janela (largura/altura). Veja função FramebufferSizeCallback().
float g_ScreenRatio = 1.0f;
float g_Width = 800;
float g_Height= 600;

// Ângulos de Euler que controlam a rotação de um dos cubos da cena virtual
float g_AngleX = 0.0f;
float g_AngleY = 0.0f;
float g_AngleZ = 0.0f;

// "g_LeftMouseButtonPressed = true" se o usuário está com o botão esquerdo do mouse
// pressionado no momento atual. Veja função MouseButtonCallback().
bool g_RightMouseButtonPressed = false; // Análogo para botão direito do mouse
bool g_MiddleMouseButtonPressed = false; // Análogo para botão do meio do mouse

// Variáveis que controlam rotação do antebraço
float g_ForearmAngleZ = 0.0f;
float g_ForearmAngleX = 0.0f;

// Variáveis que controlam translação do torso
float g_TorsoPositionX = 0.0f;
float g_TorsoPositionY = 0.0f;

// Variável que controla o tipo de projeção utilizada: perspectiva ou ortográfica.
bool g_UsePerspectiveProjection = true;

// Variável que controla se o texto informativo será mostrado na tela.
bool g_ShowInfoText = true;

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

    // Construímos a representação de objetos geométricos através de malhas de triângulos
    string path = "../../data/";
    string modelsList[10] = {
        "Link0.obj",
        "camera.obj",
        "king.obj",
        "corridor.obj",
        "box-side.obj",
        "floor.obj",
        "side.obj",
        "box.obj",
        "corridor2.obj",
        "Block.obj",
    };
    for(int i = 0; i < 10; i++)
    {
        ObjModel corridorModel((path + modelsList[i]).c_str());
        ComputeNormals(&corridorModel);
        BuildTrianglesAndAddToVirtualScene(&corridorModel);
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
    // glEnable(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
    // glFrontFace(GL_CCW);

    float lastTime = glfwGetTime();
    // Ficamos em loop, renderizando, até que o usuário feche a janela
    while (!glfwWindowShouldClose(window))
    {
        float delta = glfwGetTime() - lastTime;
        lastTime = glfwGetTime();

        // Indicamos que queremos renderizar em toda região do framebuffer. A
        // função "glViewport" define o mapeamento das "normalized device
        // coordinates" (NDC) para "pixel coordinates".  Essa é a operação de
        // "Screen Mapping" ou "Viewport Mapping" vista em aula ({+ViewportMapping2+}).
        // glEnable(GL_SCISSOR_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glViewport(0, 0, g_Width, g_Height);

        //Pintamos tudo de branco e reiniciamos o Z-BUFFER
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program_id);

        // Movimentamos o personagem se alguma tecla estiver pressionada
        float speed = 2.0f * (g_ModShift ? 10.0 : 1.0) * (g_ModCtrl ? 0.1 : 1.0);
        if(g_MovingForward) g_PlayerCamera.position += speed * delta * g_CameraRelativeForward;
        if(g_MovingBackward) g_PlayerCamera.position -= speed * delta * g_CameraRelativeForward;
        if(g_MovingLeft) g_PlayerCamera.position += speed * delta * g_CameraRelativeLeft;
        if(g_MovingRight) g_PlayerCamera.position -= speed * delta * g_CameraRelativeLeft;
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
            * Matrix_Scale(0.5f, 0.5f, 0.5f)
            * Matrix_Rotate_Y(g_PlayerCamera.theta)
            * Matrix_Rotate_X(-g_PlayerCamera.phi * 0.5);
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

    model = Matrix_Translate(-4.82, 0.42, -17.29)
        * Matrix_Scale(0.5f, 0.5f, 0.5f);

    glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
    glUniform1i(object_id_uniform, LINK);
    DrawVirtualObject("demyx");

    for(int i = 0; i < maxInstanceId + 1; i++) {
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

    model = Matrix_Translate(-10, 0.42, -17.29)
        * Matrix_Scale(0.5f, 0.5f, 0.5f)
        * Matrix_Rotate_Z(M_PI_2);
    glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
    glUniform1i(object_id_uniform, CORRIDOR);
    DrawVirtualObject("corridor2");

}

// Função que pega a matriz M e guarda a mesma no topo da pilha
void PushMatrix(glm::mat4 M)
{
    g_MatrixStack.push(M);
}

// Função que remove a matriz atualmente no topo da pilha e armazena a mesma na variável M
void PopMatrix(glm::mat4& M)
{
    if ( g_MatrixStack.empty() )
    {
        M = Matrix_Identity();
    }
    else
    {
        M = g_MatrixStack.top();
        g_MatrixStack.pop();
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

        if(g_InstanceSelectedId > maxInstanceId){
            g_InstanceSelectedId = maxInstanceId;
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


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

// Declaração de funções utilizadas para pilha de matrizes de modelagem.
void PushMatrix(glm::mat4 M);
void PopMatrix(glm::mat4& M);

// Funções abaixo renderizam como texto na janela OpenGL algumas matrizes e
// outras informações do programa. Definidas após main().
void TextRendering_ShowModelViewProjection(GLFWwindow* window, glm::mat4 projection, glm::mat4 view, glm::mat4 model, glm::vec4 p_model);
void TextRendering_PrintCameraStats(GLFWwindow* window);
void TextRendering_ShowMode(GLFWwindow* window, string mode);
void TextRendering_ShowFramesPerSecond(GLFWwindow* window);
void TextRendering_PrintMoveStats(GLFWwindow* window, glm::vec4 position);

// Funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);

void DrawWorld(bool drawPlayer, bool drawCamera);

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

// Abaixo definimos variáveis globais utilizadas em várias funções do código.
// Vetor "up" fixado para apontar para o "céu" (eito Y global)
glm::vec4 UP_VECTOR = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

Camera g_FixedCamera(-4.87, 1.82, -21.42, -0.51, 0.06);
Camera g_PlayerCamera(-4.82, 0.42, -17.29, 0.20, 3.10);

glm::vec4 g_CameraRelativeLeft = crossproduct(UP_VECTOR, g_PlayerCamera.getDirection());
glm::vec4 g_CameraRelativeForward =  crossproduct(g_CameraRelativeLeft, UP_VECTOR);

bool g_MovingForward = false;
bool g_MovingBackward = false;
bool g_MovingLeft = false;
bool g_MovingRight = false;
bool g_MovingUp = false;
bool g_MovingDown = false;

bool g_ModShift = false;

glm::vec4 g_Position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

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
bool g_LeftMouseButtonPressed = false;
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

    // Carregamos duas imagens para serem utilizadas como textura

    LoadTextureImage("../../data/de_00.png");
    LoadTextureImage("../../data/de_00.png");
    LoadTextureImage("../../data/de_00.png");
    LoadTextureImage("../../data/de_01.png");
    LoadTextureImage("../../data/de_hair00.png");
    LoadTextureImage("../../data/de_hair01.png");
    LoadTextureImage("../../data/de_hair02.png");
    LoadTextureImage("../../data/de_hair03.png");
    LoadTextureImage("../../data/de_hair04.png");
    LoadTextureImage("../../data/de_hair05.png");
    LoadTextureImage("../../data/de_hair06.png");
    LoadTextureImage("../../data/de_robe00.png");
    LoadTextureImage("../../data/de_robe00.png");
    LoadTextureImage("../../data/de_robe01.png");
    LoadTextureImage("../../data/robe02.png");

    // Construímos a representação de objetos geométricos através de malhas de triângulos
    ObjModel spheremodel("../../data/sphere.obj");
    ComputeNormals(&spheremodel);
    BuildTrianglesAndAddToVirtualScene(&spheremodel);

    ObjModel bunnymodel("../../data/bunny.obj");
    ComputeNormals(&bunnymodel);
    BuildTrianglesAndAddToVirtualScene(&bunnymodel);

    ObjModel planemodel("../../data/plane.obj");
    ComputeNormals(&planemodel);
    BuildTrianglesAndAddToVirtualScene(&planemodel);

    ObjModel mansionModel("../../data/hallway.obj");
    ComputeNormals(&mansionModel);
    BuildTrianglesAndAddToVirtualScene(&mansionModel);

    ObjModel linkModel("../../data/Link0.obj");
    ComputeNormals(&linkModel);
    BuildTrianglesAndAddToVirtualScene(&linkModel);

    ObjModel cameraModel("../../data/camera.obj");
    ComputeNormals(&cameraModel);
    BuildTrianglesAndAddToVirtualScene(&cameraModel);

    ObjModel kingModel("../../data/king.obj");
    ComputeNormals(&kingModel);
    BuildTrianglesAndAddToVirtualScene(&kingModel);

    ObjModel corridorModel("../../data/corridor.obj");
    ComputeNormals(&corridorModel);
    BuildTrianglesAndAddToVirtualScene(&corridorModel);

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

    // Variáveis auxiliares utilizadas para chamada à função
    // TextRendering_ShowModelViewProjection(), armazenando matrizes 4x4.
    glm::mat4 the_projection;
    glm::mat4 the_model;
    glm::mat4 the_view;

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
        float speed = 2.0f;
        if(g_MovingForward) g_PlayerCamera.position += speed * delta * g_CameraRelativeForward;
        if(g_MovingBackward) g_PlayerCamera.position -= speed * delta * g_CameraRelativeForward;
        if(g_MovingLeft) g_PlayerCamera.position += speed * delta * g_CameraRelativeLeft;
        if(g_MovingRight) g_PlayerCamera.position -= speed * delta * g_CameraRelativeLeft;
        if(g_MovingUp) g_PlayerCamera.position += speed * delta * UP_VECTOR;
        if(g_MovingDown) g_PlayerCamera.position -= speed * delta * UP_VECTOR;

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

        TextRendering_ShowMode(window, g_Mode);
        if(g_Mode.compare("PLAYER") == 0){
            TextRendering_PrintCameraStats(window);
        }

        if(g_Mode.compare("MOVE") == 0){
            TextRendering_PrintMoveStats(window, g_Position);
        }

        // Imprimimos na tela informação sobre o número de quadros renderizados
        // por segundo (frames per second).
        TextRendering_ShowFramesPerSecond(window);

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

#define SPHERE 0
#define BUNNY  1
#define PLANE  2
#define LINK  3
#define CORRIDOR 4

void DrawWorld(bool drawPlayer, bool drawCamera){
    glm::mat4 model = Matrix_Identity(); // Transformação identidade de modelagem

    if (drawPlayer) {
        // Desenhamos o player
        model = Matrix_Translate(g_PlayerCamera.position.x, g_PlayerCamera.position.y, g_PlayerCamera.position.z)
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
            * Matrix_Scale(0.05, 0.05, 0.05);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, PLANE);

        DrawVirtualObject("camera_reference");
    }
    // Desenhamos a fortaleza
    model = Matrix_Translate(0.0f, 0.0f, 0.0f)
        * Matrix_Scale(1.0f, 1.0f, 1.0f);
    glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
    glUniform1i(object_id_uniform, PLANE);

    DrawVirtualObject("part8_texture6");
    DrawVirtualObject("part0_texture11");
    DrawVirtualObject("part12_texture10");
    DrawVirtualObject("part2_texture13");
    DrawVirtualObject("part5_texture3");
    DrawVirtualObject("part6_texture4");
    DrawVirtualObject("part10_texture8");
    DrawVirtualObject("part3_texture14");
    DrawVirtualObject("part11_texture9");
    DrawVirtualObject("part7_texture5");
    DrawVirtualObject("part9_texture7");
    DrawVirtualObject("part2_texture1");
    DrawVirtualObject("part1_texture0");
    DrawVirtualObject("part0_texture0");
    DrawVirtualObject("part0_texture11.001");

    // Desenhamos a fortaleza
    model = Matrix_Translate(g_Position.x, g_Position.y, g_Position.z)
        * Matrix_Translate(-9.72, 0.0, -34.46)
        * Matrix_Scale(1.0f, 1.0f, 1.0f)
        * Matrix_Rotate_Y(3.141592);
    glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));

    DrawVirtualObject("part8_texture6");
    DrawVirtualObject("part0_texture11");
    DrawVirtualObject("part12_texture10");
    DrawVirtualObject("part2_texture13");
    DrawVirtualObject("part5_texture3");
    DrawVirtualObject("part6_texture4");
    DrawVirtualObject("part10_texture8");
    DrawVirtualObject("part3_texture14");
    DrawVirtualObject("part11_texture9");
    DrawVirtualObject("part7_texture5");
    DrawVirtualObject("part9_texture7");
    DrawVirtualObject("part2_texture1");
    DrawVirtualObject("part1_texture0");
    DrawVirtualObject("part0_texture0");
    DrawVirtualObject("part0_texture11.001");

    model = Matrix_Translate(-4.82, 0.42, -17.29)
        * Matrix_Scale(0.5f, 0.5f, 0.5f);

    glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
    glUniform1i(object_id_uniform, LINK);

    DrawVirtualObject("demyx");

    model = Matrix_Translate(0.0f, 0.42, 0.0f)
        * Matrix_Scale(2.5f, 2.5f, 2.5f);

    glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
    glUniform1i(object_id_uniform, CORRIDOR);
    DrawVirtualObject("corridor");


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
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_LeftMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_LeftMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_LeftMouseButtonPressed = true;
    }
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

    if (g_LeftMouseButtonPressed)
    {
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

        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }

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

    // Se o usuário pressionar a tecla ESC, fechamos a janela.
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // Movimentação da câmera
    if (key == GLFW_KEY_W) g_MovingForward = IsActionPressed(action);
    if (key == GLFW_KEY_S) g_MovingBackward = IsActionPressed(action);
    if (key == GLFW_KEY_A) g_MovingLeft = IsActionPressed(action);
    if (key == GLFW_KEY_D) g_MovingRight = IsActionPressed(action);
    if (key == GLFW_KEY_Q) g_MovingUp = IsActionPressed(action);
    if (key == GLFW_KEY_E) g_MovingDown = IsActionPressed(action);

    if (mod == GLFW_MOD_SHIFT) g_ModShift = IsActionPressed(action);

    if(key == GLFW_KEY_P && IsActionPressed(action)) g_Mode = "PLAYER";
    if(key == GLFW_KEY_M && IsActionPressed(action)) g_Mode = "MOVE";

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

// Esta função recebe um vértice com coordenadas de modelo p_model e passa o
// mesmo por todos os sistemas de coordenadas armazenados nas matrizes model,
// view, e projection; e escreve na tela as matrizes e pontos resultantes
// dessas transformações.
void TextRendering_ShowModelViewProjection(
    GLFWwindow* window,
    glm::mat4 projection,
    glm::mat4 view,
    glm::mat4 model,
    glm::vec4 p_model
)
{
    if ( !g_ShowInfoText )
        return;

    glm::vec4 p_world = model*p_model;
    glm::vec4 p_camera = view*p_world;
    glm::vec4 p_clip = projection*p_camera;
    glm::vec4 p_ndc = p_clip / p_clip.w;

    float pad = TextRendering_LineHeight(window);

    TextRendering_PrintString(window, " Model matrix             Model     In World Coords.", -1.0f, 1.0f-pad, 1.0f);
    TextRendering_PrintMatrixVectorProduct(window, model, p_model, -1.0f, 1.0f-2*pad, 1.0f);

    TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f-6*pad, 1.0f);
    TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f-7*pad, 1.0f);
    TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f-8*pad, 1.0f);

    TextRendering_PrintString(window, " View matrix              World     In Camera Coords.", -1.0f, 1.0f-9*pad, 1.0f);
    TextRendering_PrintMatrixVectorProduct(window, view, p_world, -1.0f, 1.0f-10*pad, 1.0f);

    TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f-14*pad, 1.0f);
    TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f-15*pad, 1.0f);
    TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f-16*pad, 1.0f);

    TextRendering_PrintString(window, " Projection matrix        Camera                    In NDC", -1.0f, 1.0f-17*pad, 1.0f);
    TextRendering_PrintMatrixVectorProductDivW(window, projection, p_camera, -1.0f, 1.0f-18*pad, 1.0f);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    glm::vec2 a = glm::vec2(-1, -1);
    glm::vec2 b = glm::vec2(+1, +1);
    glm::vec2 p = glm::vec2( 0,  0);
    glm::vec2 q = glm::vec2(width, height);

    glm::mat4 viewport_mapping = Matrix(
        (q.x - p.x)/(b.x-a.x), 0.0f, 0.0f, (b.x*p.x - a.x*q.x)/(b.x-a.x),
        0.0f, (q.y - p.y)/(b.y-a.y), 0.0f, (b.y*p.y - a.y*q.y)/(b.y-a.y),
        0.0f , 0.0f , 1.0f , 0.0f ,
        0.0f , 0.0f , 0.0f , 1.0f
    );

    TextRendering_PrintString(window, "                                                       |  ", -1.0f, 1.0f-22*pad, 1.0f);
    TextRendering_PrintString(window, "                            .--------------------------'  ", -1.0f, 1.0f-23*pad, 1.0f);
    TextRendering_PrintString(window, "                            V                           ", -1.0f, 1.0f-24*pad, 1.0f);

    TextRendering_PrintString(window, " Viewport matrix           NDC      In Pixel Coords.", -1.0f, 1.0f-25*pad, 1.0f);
    TextRendering_PrintMatrixVectorProductMoreDigits(window, viewport_mapping, p_ndc, -1.0f, 1.0f-26*pad, 1.0f);
}

void TextRendering_PrintCameraStats(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    float pad = TextRendering_LineHeight(window);

    char buffer[80];
    snprintf(buffer, 80, "Camera Direction = Phi: (%.2f), Theta: (%.2f)\n", g_PlayerCamera.phi, g_PlayerCamera.theta);
    TextRendering_PrintString(window, buffer, -1.0f+pad/10, -1.0f+pad, 1.0f);

    snprintf(buffer, 80, "Camera position = X: (%.2f), Y: (%.2f), Z: (%.2f)\n", g_PlayerCamera.position.x, g_PlayerCamera.position.y, g_PlayerCamera.position.z);
    TextRendering_PrintString(window, buffer, -1.0f+pad/10, -1.0f, 1.0f);

}

void TextRendering_PrintMoveStats(GLFWwindow* window, glm::vec4 position)
{
    if ( !g_ShowInfoText )
        return;

    float pad = TextRendering_LineHeight(window);

    char buffer[80];

    snprintf(buffer, 80, "Camera position = X: (%.2f), Y: (%.2f), Z: (%.2f)\n", position.x, position.y, position.z);
    TextRendering_PrintString(window, buffer, -1.0f+pad/10, -1.0f, 1.0f);
}

// Escrevemos na tela qual matriz de projeção está sendo utilizada.
void TextRendering_ShowMode(GLFWwindow* window, string mode)
{
    if ( !g_ShowInfoText )
        return;

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    TextRendering_PrintString(window, mode, 1.0f-strlen(mode.c_str())*charwidth, -1.0f+2*lineheight/10, 1.0f);
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


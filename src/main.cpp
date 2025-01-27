//     Universidade Federal do Rio Grande do Sul
//             Instituto de Informática
//       Departamento de Informática Aplicada
//
//    INF01047 Fundamentos de Computação Gráfica
//               Prof. Eduardo Gastal
//
//                  TRABALHO FINAL
//

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

#include "game.cpp"

struct Viewport {
    float width;
    float height;
    float screenRatio;
} g_Viewport;

void ErrorCallback(int error, const char* description);
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);

int main(int argc, char* argv[])
{
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    glfwSetErrorCallback(ErrorCallback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    window = glfwCreateWindow(800, 600, "INF01047 - Trabalho Final", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    FramebufferSizeCallback(window, 800, 600); // Forçamos a chamada do callback acima, para definir g_ScreenRatio.

    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *glversion   = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);
    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    TextRendering_Init();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);

    LoadShadersFromFiles();
    LoadTextureImage("../../data/cameratitlemenu_alb.png");

    string texturesPath = "../../data/textures/";
    std::vector<string> texturesList = {
    //Dragon textures
    "bossraidarm_alb.png",
    "bossraidbody_alb.png",
    "bossraideye_alb.png",
    "bossraidhead_alb.png",
    "bossraidwing_alb.png",

    //Reaper textures
    "reaper_01.png",
    "reaper_02.png",

    //Lucina texture
    "lucina.png",

    //Dark Brick Wall texture
    "dark_brick_wall.png",
    "dark_brick_wall2.png",

    //Drone texture
    "drone.png",

    //Crystal texture
    "crystal.png",
    };

    for(int i = 0; i < texturesList.size(); i++)
    {
        LoadTextureImage((texturesPath + texturesList[i]).c_str());
    }

    auto loadAnimationModels = [](int totalFrames, string filePreffix, string shapeSuffix){
        for(int i = 0; i < totalFrames; i++)
        {
            std::stringstream number;
            number.fill('0');
            number.width(3);
            number << (i);
            string fileName = filePreffix + number.str() + ".obj";

            ObjModel models(fileName.c_str());
            ComputeNormals(&models);
            BuildTrianglesAndAddToVirtualScene(&models, i, shapeSuffix);
        }
    };

    loadAnimationModels(FRAMES_REAPER, "../../data/reaper/reaper_000", "");
    loadAnimationModels(FRAMES_LUCINA_WALKING, "../../data/lucina_walking/lucina_000", "_WALK_");
    loadAnimationModels(FRAMES_LUCINA_LEFT_WALKING, "../../data/lucina_left_walking/lucina_000", "_LEFT_WALK_");
    loadAnimationModels(FRAMES_LUCINA_IDLE, "../../data/lucina_idle/lucina_000", "_IDLE_");
    loadAnimationModels(FRAMES_LUCINA_DEATH, "../../data/lucina_death/lucina_000", "_DEATH_");

    string path = "../../data/";
    std::vector<string> modelsList = {
    "Link0.obj",
    "camera.obj",
    "king.obj",
    "corridor.obj",
    "corridor2.obj",
    "Block.obj",
    "dragon.obj",
    "reaper.obj",
    "lucina.obj",
    "drone.obj",
    "crystal.obj",
    };

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

    while(true)
    {
        Game(window, &(g_Viewport.width), &(g_Viewport.height), &(g_Viewport.screenRatio));
    }
}

// Definimos o callback para impressão de erros da GLFW no terminal
void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    g_Viewport.screenRatio = (float)width / height;
    g_Viewport.width = width;
    g_Viewport.height = height;
}


// set makeprg=cd\ ..\ &&\ make\ run\ >/dev/null
// vim: set spell spelllang=pt_br :


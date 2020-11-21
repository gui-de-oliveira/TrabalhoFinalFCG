#version 330 core

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpolação da posição global e a normal de cada vértice, definidas em
// "shader_vertex.glsl" e "main.cpp".
in vec4 position_world;
in vec4 normal;

// Posição do vértice atual no sistema de coordenadas local do modelo.
in vec4 position_model;

// Variaveis definidas em "shader_vertex.glsl"
in vec3 colorGourand;
in float lambertGourand;
//out bool useGourand;

float lambert;
bool noTexture = false;

// Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
in vec2 texcoords;

flat in vec2 material_id;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificador que define qual objeto está sendo desenhado no momento
#define CRYSTAL 0
#define BUNNY  1
#define PLANE  2
#define LINK  3
#define CORRIDOR 4
#define DRAGON 5
#define REAPER 6
#define LUCINA 7
#define CORRIDOR2 8
#define DRONE 9

uniform int object_id;

// Parâmetros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;

// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec4 color;

vec3 color3;
vec3 colorPhong;

uniform sampler2D TextureImage_Link;
uniform sampler2D TextureImage_Wall;
uniform sampler2D TextureImage_Wall2;
uniform sampler2D TextureImage_Crystal;

uniform sampler2D DragonTexture_0;
uniform sampler2D DragonTexture_1;
uniform sampler2D DragonTexture_2;
uniform sampler2D DragonTexture_3;
uniform sampler2D DragonTexture_4;

sampler2D selectDragonTexture() {
    switch(int(material_id.x))
    {
        case 0: return DragonTexture_0; //Arm
        case 1:
        case 2: return DragonTexture_1; //Body
        case 3: return DragonTexture_2; //Eye
        case 4:
        case 5:
        case 6:
        case 7: return DragonTexture_3; //Head
        case 8:
        case 9: return DragonTexture_4; //Wings 
    }
}

uniform sampler2D ReaperTexture_0;
uniform sampler2D ReaperTexture_1;

sampler2D selectReaperTexture() {
    switch(int(material_id.x))
    {
        case -1: return DragonTexture_2;
        case 0: return ReaperTexture_0;
        case 1: return ReaperTexture_1;
        default: return ReaperTexture_0;
    }
}

uniform sampler2D LucinaTexture;
uniform sampler2D DroneTexture;

// Constantes
#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

void phongColors (){

    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;
    vec4 n = normalize(normal); //// Normal do fragmento atual
    vec4 l = normalize(vec4(0.0,1.0,0.0,0.0) - position_world); //Sentido da fonte de luz em relação ao ponto atual.
    vec4 v = normalize(camera_position - position_world);  //Sentido da câmera em relação ao ponto atual
    vec4 r = -l + 2*n*dot(n,l); //Sentido da reflex�o especular ideal.
    
    lambert = max(0, dot(n, l));

    // vec3 Kd; // Refletancia difusa
    // vec3 Ks; // Refletancia especular
    // vec3 Ka; // Refletancia ambiente
    // float q; // Expoente especular para o modelo de iluminacao de Phong
    vec3 Kd = vec3(0.6196, 0.0275, 0.8549);
    vec3 Ks = vec3(0.8,0.8,0.8);
    vec3 Ka = Kd/2;
    float q = 32.0;

    vec3 I = vec3(1.0,1.0,1.0); // Espectro da fonte de iluminacao
    vec3 Ia = vec3(0.2,0.2,0.2); // Espectro da luz ambiente
    vec3 lambert_diffuse_term = Kd * I * max(0,dot(n,l));
    vec3 ambient_term = Ka*Ia;
    vec3 phong_specular_term  = Ks * I * pow(max(0,dot(r,v)),q);

    colorPhong = lambert_diffuse_term + ambient_term + phong_specular_term;
}

void main()
{   
    vec4 p = position_world;

    // Coordenadas de textura U e V
    float U = 0.0;
    float V = 0.0;

    vec3 TextureColor;

    if(object_id == CORRIDOR){
        // float minx = bbox_min.x;
        // float maxx = bbox_max.x;

        // float miny = bbox_min.y;
        // float maxy = bbox_max.y;

        // float minz = bbox_min.z;
        // float maxz = bbox_max.z;

        // U = (position_model.x - bbox_min.x) / (bbox_max.x - bbox_min.x);
        // V = (position_model.y - bbox_min.y) / (bbox_max.y - bbox_min.y);

        // vec4 bbox_center = (bbox_min + bbox_max) / 2.0;

        // vec4 p_vec = normalize(position_model - bbox_center);

        // float theta = atan(p_vec.x, p_vec.z);
        // float phi = asin(p_vec.y);

        // U = (theta + M_PI)/(2*M_PI);
        // V = (phi + M_PI_2)/M_PI;
        U = texcoords.x;
        V = texcoords.y;

        TextureColor = texture(TextureImage_Wall, vec2(U,V)).rgb;
    }

    else if (object_id == DRAGON){
        U = texcoords.x;
        V = texcoords.y;
     
        TextureColor = texture(selectDragonTexture(), vec2(U,V)).rgb;
    }

    else if (object_id == REAPER){
        U = texcoords.x;
        V = texcoords.y;
     
        TextureColor = texture(selectReaperTexture(), vec2(U,V)).rgb;
    }

    else if (object_id == LUCINA){
        U = texcoords.x;
        V = texcoords.y;
     
        TextureColor = texture(LucinaTexture, vec2(U,V)).rgb;
    }

    else if (object_id == DRONE){
        U = texcoords.x;
        V = texcoords.y;
     
        TextureColor = texture(DroneTexture, vec2(U,V)).rgb;
    }

    else if(object_id == CORRIDOR2){
        // float minx = bbox_min.x;
        // float maxx = bbox_max.x;

        // float miny = bbox_min.y;
        // float maxy = bbox_max.y;

        // float minz = bbox_min.z;
        // float maxz = bbox_max.z;

        // U = (position_model.x - bbox_min.x) / (bbox_max.x - bbox_min.x);
        // V = (position_model.y - bbox_min.y) / (bbox_max.y - bbox_min.y);

        U = texcoords.x;
        V = texcoords.y;

        TextureColor = texture(TextureImage_Wall2, vec2(U,V)).rgb;
    }
    else if(object_id == CRYSTAL){
        noTexture = true;
        // U = texcoords.x;
        // V = texcoords.y;

        // TextureColor = texture(TextureImage_Crystal, vec2(U,V)).rgb;
    }

    else {
        noTexture = true;
    }

    phongColors();
    color3 = colorPhong;

    if(noTexture){
        color = vec4(color3.xyz, 1.0);
    }
    else{
        color = vec4(TextureColor * (lambert + 0.01), 1.0);
    }

    //color = vec4(vec3(1.0, 1.0, 1.0) * (lambert + 0.01), 1.0);
    color = pow(color, vec4(1.0,1.0,1.0,1.0)/2.2);
}


#version 330 core

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpolação da posição global e a normal de cada vértice, definidas em
// "shader_vertex.glsl" e "main.cpp".
in vec4 position_world;
in vec4 normal;

// Posição do vértice atual no sistema de coordenadas local do modelo.
in vec4 position_model;

// Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
in vec2 texcoords;

flat in vec2 material_id;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificador que define qual objeto está sendo desenhado no momento
#define SPHERE 0
#define BUNNY  1
#define PLANE  2
#define LINK  3
#define CORRIDOR 4

uniform int object_id;

// Parâmetros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;

// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec4 color;
out vec3 color3;

uniform sampler2D TextureImage_Link;


// Constantes
#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

void main()
{
    // Obtemos a posição da câmera utilizando a inversa da matriz que define o
    // sistema de coordenadas da câmera.
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;

    // O fragmento atual é coberto por um ponto que percente à superfície de um
    // dos objetos virtuais da cena. Este ponto, p, possui uma posição no
    // sistema de coordenadas global (World coordinates). Esta posição é obtida
    // através da interpolação, feita pelo rasterizador, da posição de cada
    // vértice.
    vec4 p = position_world;

    // Normal do fragmento atual, interpolada pelo rasterizador a partir das
    // normais de cada vértice.
    vec4 n = normalize(normal);

    // Vetor que define o sentido da fonte de luz em relação ao ponto atual.
    vec4 l = normalize(vec4(1.0,1.0,0.0,0.0));

    // Vetor que define o sentido da câmera em relação ao ponto atual.
    vec4 v = normalize(camera_position - p);

    // Vetor que define o sentido da reflex�o especular ideal.
    vec4 r = -l + 2*n*dot(n,l);

    // Coordenadas de textura U e V
    float U = 0.0;
    float V = 0.0;

    // Parametros que definem as propriedades espectrais da superficie
    vec3 Kd; // Refletancia difusa
    vec3 Ks; // Refletancia especular
    vec3 Ka; // Refletancia ambiente
    float q; // Expoente especular para o modelo de iluminacao de Phong

    if(object_id == LINK){
        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;

        vec4 p_vec = normalize(position_model - bbox_center);

        float theta = atan(p_vec.x, p_vec.z);
        float phi = asin(p_vec.y);

        U = (theta + M_PI)/(2*M_PI);
        V = (phi + M_PI_2)/M_PI;
    }
    // if(object_id == LINK){
    //     float minx = bbox_min.x;
    //     float maxx = bbox_max.x;

    //     float miny = bbox_min.y;
    //     float maxy = bbox_max.y;

    //     float minz = bbox_min.z;
    //     float maxz = bbox_max.z;

    //     U = (position_model.x - bbox_min.x) / (bbox_max.x - bbox_min.x);
    //     V = (position_model.y - bbox_min.y) / (bbox_max.y - bbox_min.y);
    // }
    if(object_id == CORRIDOR){
        Kd = vec3(0.5255, 0.0078, 0.0078);
        Ks = vec3(0.8,0.8,0.8);
        Ka = Kd/2;
        q = 32.0;
    }
    vec3 Kd0 = texture(TextureImage_Link, vec2(U,V)).rgb;
    // else{
    //     Kd = vec3(0.0,0.0,0.0);
    //     Ks = vec3(0.0,0.0,0.0);
    //     Ka = vec3(0.0,0.0,0.0);
    //     q = 1.0;
    // }
    // Por algum motivo todo o corredor fica preto usando isso

    // Espectro da fonte de iluminacao
    vec3 I = vec3(1.0,1.0,1.0);

    // Espectro da luz ambiente
    vec3 Ia = vec3(0.2,0.2,0.2);

    // Termo difuso utilizando a lei dos cossenos de Lambert
    vec3 lambert_diffuse_term = Kd*I*max(0,dot(n,l)); // PREENCHA AQUI o termo difuso de Lambert

    // Termo ambiente
    vec3 ambient_term = Ka*Ia;

    // Termo especular utilizando o modelo de iluminacao de Phong
    vec3 phong_specular_term  = Ks*I*pow(max(0,dot(r,v)),q);

    // Cor final do fragmento calculada com uma combinacao dos termos difuso,
    // especular, e ambiente. Veja slide 129 do documento Aula_17_e_18_Modelos_de_Iluminacao.pdf.
    color3 = lambert_diffuse_term + ambient_term + phong_specular_term;
    
    // Equação de Iluminação
    float lambert = max(0,dot(n,l));
    color3 += Kd0 * (lambert + 0.01);
    color = vec4(color3.x, color3.y, color3.z, 1.0);

    //color = vec4(vec3(1.0, 1.0, 1.0) * (lambert + 0.01), 1.0);
    color = pow(color, vec4(1.0,1.0,1.0,1.0)/2.2);
}


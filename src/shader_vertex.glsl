#version 330 core

// Atributos de v�rtice recebidos como entrada ("in") pelo Vertex Shader.
// Veja a fun��o BuildTrianglesAndAddToVirtualScene() em "main.cpp".
layout (location = 0) in vec4 model_coefficients;
layout (location = 1) in vec4 normal_coefficients;
layout (location = 2) in vec2 texture_coefficients;
layout (location = 3) in vec2 in_material_id;

// Matrizes computadas no c�digo C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Atributos de v�rtice que ser�o gerados como sa�da ("out") pelo Vertex Shader.
// ** Estes ser�o interpolados pelo rasterizador! ** gerando, assim, valores
// para cada fragmento, os quais ser�o recebidos como entrada pelo Fragment
// Shader. Veja o arquivo "shader_fragment.glsl".
out vec3 colorGourand;
out float lambertGourand;
// in bool useGourand;

out vec4 position_world;
out vec4 position_model;
out vec4 normal;
out vec2 texcoords;
flat out vec2 material_id;

void gourandColors (){
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;
    vec4 n = normalize(normal); //// Normal do fragmento atual
    vec4 l = normalize(vec4(0.0,1.0,0.0,0.0) - position_world); //Sentido da fonte de luz em relação ao ponto atual.
    vec4 v = normalize(camera_position - position_world);  //Sentido da câmera em relação ao ponto atual
    vec4 r = -l + 2*n*dot(n,l); //Sentido da reflex�o especular ideal.
    
    lambertGourand = max(0, dot(n, l));

    // vec3 Kd; // Refletancia difusa
    // vec3 Ks; // Refletancia especular
    // vec3 Ka; // Refletancia ambiente
    // float q; // Expoente especular para o modelo de iluminacao de Phong
    vec3 Kd = vec3(0.1686, 0.6235, 0.9255);
    vec3 Ks = vec3(0.8,0.8,0.8);
    vec3 Ka = Kd/2;
    float q = 32.0;

    vec3 I = vec3(1.0,1.0,1.0); // Espectro da fonte de iluminacao
    vec3 Ia = vec3(0.2,0.2,0.2); // Espectro da luz ambiente
    vec3 lambert_diffuse_term = Kd * I * max(0,dot(n,l));
    vec3 ambient_term = Ka*Ia;
    vec3 phong_specular_term  = Ks * I * pow(max(0,dot(r,v)),q);

    colorGourand = lambert_diffuse_term + ambient_term + phong_specular_term;
}

void main()
{
    // A vari�vel gl_Position define a posi��o final de cada v�rtice
    // OBRIGATORIAMENTE em "normalized device coordinates" (NDC), onde cada
    // coeficiente estar� entre -1 e 1 ap�s divis�o por w.
    // Veja {+NDC2+}.
    //
    // O c�digo em "main.cpp" define os v�rtices dos modelos em coordenadas
    // locais de cada modelo (array model_coefficients). Abaixo, utilizamos
    // opera��es de modelagem, defini��o da c�mera, e proje��o, para computar
    // as coordenadas finais em NDC (vari�vel gl_Position). Ap�s a execu��o
    // deste Vertex Shader, a placa de v�deo (GPU) far� a divis�o por W. Veja
    // slides 41-67 e 69-86 do documento Aula_09_Projecoes.pdf.

    gl_Position = projection * view * model * model_coefficients;

    // Como as vari�veis acima  (tipo vec4) s�o vetores com 4 coeficientes,
    // tamb�m � poss�vel acessar e modificar cada coeficiente de maneira
    // independente. Esses s�o indexados pelos nomes x, y, z, e w (nessa
    // ordem, isto �, 'x' � o primeiro coeficiente, 'y' � o segundo, ...):
    //
    //     gl_Position.x = model_coefficients.x;
    //     gl_Position.y = model_coefficients.y;
    //     gl_Position.z = model_coefficients.z;
    //     gl_Position.w = model_coefficients.w;
    //

    // Agora definimos outros atributos dos v�rtices que ser�o interpolados pelo
    // rasterizador para gerar atributos �nicos para cada fragmento gerado.

    // Posi��o do v�rtice atual no sistema de coordenadas global (World).
    position_world = model * model_coefficients;

    // Posi��o do v�rtice atual no sistema de coordenadas local do modelo.
    position_model = model_coefficients;

    // Normal do v�rtice atual no sistema de coordenadas global (World).
    // Veja slides 123-151 do documento Aula_07_Transformacoes_Geometricas_3D.pdf.
    normal = inverse(transpose(model)) * normal_coefficients;
    normal.w = 0.0;

    // Calcula as cores por vertice se a flag é verdadeira (a flag é setada no "shader_fragment")
    // if (useGourand)
    //     gourandColors ();

    // Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
    texcoords = texture_coefficients;

    material_id = in_material_id;
}


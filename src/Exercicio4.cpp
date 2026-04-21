#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <string>

GLFWwindow* Window = nullptr;
GLuint Shader_programm;
GLuint Vao_quad;

const int WIDTH = 800;
const int HEIGHT = 600;

float Tempo_entre_frames = 0.0f;

glm::vec3 Cam_pos = glm::vec3(0.0f, 0.5f, 4.0f);
float Cam_yaw = 270.0f;
float Cam_pitch = -5.0f;
float Cam_speed = 5.0f;

float lastX = WIDTH / 2.0f, lastY = HEIGHT / 2.0f;
bool primeiro_mouse = true;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (primeiro_mouse) {
        lastX = xpos;
        lastY = ypos;
        primeiro_mouse = false;
        return;
    }

    float dx = xpos - lastX;
    float dy = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sens = 0.1f;
    Cam_yaw += dx * sens;
    Cam_pitch += dy * sens;

    if (Cam_pitch > 89.0f) Cam_pitch = 89.0f;
    if (Cam_pitch < -89.0f) Cam_pitch = -89.0f;
}

void inicializaOpenGL() {
    glfwInit();
    Window = glfwCreateWindow(WIDTH, HEIGHT, "Exemplo 5 - Multi Luz", nullptr, nullptr);
    glfwMakeContextCurrent(Window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(Window, key_callback);
    glfwSetCursorPosCallback(Window, mouse_callback);
}

void inicializaQuad() {
    float vertices[] = {
        -1,-1,  1,-1,  1,1,
        -1,-1,  1,1,  -1,1
    };

    glGenVertexArrays(1, &Vao_quad);
    glBindVertexArray(Vao_quad);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
}

void inicializaShaders() {

const char* vs = R"(
#version 400
layout(location = 0) in vec2 posicao;
out vec2 uv;
void main(){
    uv = posicao*0.5+0.5;
    gl_Position = vec4(posicao,0,1);
}
)";

const char* fs = R"(
#version 400

#define MAX_LUZES 4

in vec2 uv;
out vec4 cor_final;

uniform vec3 cam_origem;
uniform vec3 cam_horizontal;
uniform vec3 cam_vertical;
uniform vec3 cam_canto_inf_esq;

uniform int num_luzes;
uniform vec3 luzes_posicao[MAX_LUZES];

uniform vec3 esfera_centro;
uniform float esfera_raio;
uniform vec3 esfera_cor;
uniform float esfera_refletividade;

uniform vec3 esfera2_centro;
uniform float esfera2_raio;
uniform vec3 esfera2_cor;
uniform float esfera2_refletividade;

uniform vec3 esfera3_centro;
uniform float esfera3_raio;
uniform vec3 esfera3_cor;
uniform float esfera3_refletividade;

float intersecta_esfera(vec3 origem, vec3 direcao, vec3 centro, float raio){
    vec3 oc = origem - centro;
    float a = dot(direcao, direcao);
    float b = 2.0 * dot(direcao, oc);
    float c = dot(oc, oc) - raio * raio;
    float disc = b * b - 4 * a * c;
    if(disc < 0) return -1;
    float t1 = (-b - sqrt(disc)) / (2 * a);
    float t2 = (-b + sqrt(disc)) / (2 * a);
    if(t1 > 0) return t1;
    if(t2 > 0) return t2;
    return -1;
}

float intersecta_plano(vec3 origem, vec3 direcao, vec3 ponto_plano, vec3 normal_plano){
    float denom = dot(normal_plano, direcao);
    if(abs(denom) > 1e-6){
        float t = dot(ponto_plano - origem, normal_plano) / denom;
        if(t >= 0) return t;
    }
    return -1;
}

bool intersecta_cena(vec3 origem, vec3 direcao,
    out vec3 ponto, out vec3 normal, out vec3 cor, out float reflet){

    float t_min = 1e10;
    bool acertou = false;
    float t;

    t = intersecta_esfera(origem, direcao, esfera_centro, esfera_raio);
    if(t > 0 && t < t_min){
        t_min = t;
        ponto = origem + t * direcao;
        normal = normalize(ponto - esfera_centro);
        cor = esfera_cor;
        reflet = esfera_refletividade;
        acertou = true;
    }

    t = intersecta_esfera(origem, direcao, esfera2_centro, esfera2_raio);
    if(t > 0 && t < t_min){
        t_min = t;
        ponto = origem + t * direcao;
        normal = normalize(ponto - esfera2_centro);
        cor = esfera2_cor;
        reflet = esfera2_refletividade;
        acertou = true;
    }

    t = intersecta_esfera(origem, direcao, esfera3_centro, esfera3_raio);
    if(t > 0 && t < t_min){
        t_min = t;
        ponto = origem + t * direcao;
        normal = normalize(ponto - esfera3_centro);
        cor = esfera3_cor;
        reflet = esfera3_refletividade;
        acertou = true;
    }

    vec3 chao_normal = vec3(0,1,0);
    vec3 chao_ponto = vec3(0,-1,0);
    float t_chao = intersecta_plano(origem, direcao, chao_ponto, chao_normal);

    if(t_chao > 0 && t_chao < t_min){
        t_min = t_chao;
        ponto = origem + t_chao * direcao;
        normal = chao_normal;
        float check = mod(floor(ponto.x) + floor(ponto.z), 2.0);
        cor = mix(vec3(0.2), vec3(0.4), check);
        reflet = 0.2;
        acertou = true;
    }

    return acertou;
}

bool esta_na_sombra(vec3 ponto_de_impacto, vec3 normal, vec3 luz){
    vec3 direcao_luz = normalize(luz - ponto_de_impacto);
    float distancia_ate_luz = length(luz - ponto_de_impacto);
    vec3 origem_sombra = ponto_de_impacto + normal * 0.001;

    float t1 = intersecta_esfera(origem_sombra, direcao_luz, esfera_centro, esfera_raio);
    float t2 = intersecta_esfera(origem_sombra, direcao_luz, esfera2_centro, esfera2_raio);
    float t3 = intersecta_esfera(origem_sombra, direcao_luz, esfera3_centro, esfera3_raio);

    return (t1 > 0 && t1 < distancia_ate_luz) ||
           (t2 > 0 && t2 < distancia_ate_luz) ||
           (t3 > 0 && t3 < distancia_ate_luz);
}

vec3 calcula_phong_luz(vec3 ponto, vec3 normal, vec3 cor_material, vec3 luz){
    vec3 L = normalize(luz - ponto);
    vec3 V = normalize(cam_origem - ponto);

    float Kd = max(dot(normal, L), 0.0);
    vec3 difusa = cor_material * Kd;

    vec3 R = reflect(-L, normal);
    float Ks = pow(max(dot(R, V), 0.0), 32.0);
    vec3 especular = vec3(1.0) * Ks;

    return difusa * 0.75 + especular * 0.6;
}

vec3 cor_fundo(vec3 direcao){
    float t = direcao.y * 0.5 + 0.5;
    return mix(vec3(0.1,0.1,0.3), vec3(0.5,0.7,1.0), t);
}

void main(){

    vec3 pixel_no_mundo = cam_canto_inf_esq
                        + uv.x * cam_horizontal
                        + uv.y * cam_vertical;

    vec3 origem_raio = cam_origem;
    vec3 direcao_raio = normalize(pixel_no_mundo - cam_origem);

    vec3 cor_acumulada = vec3(0);
    float peso = 1.0;

    for(int bounce = 0; bounce < 4; bounce++){

        vec3 ponto, normal, cor_material;
        float refletividade;

        bool acertou = intersecta_cena(origem_raio, direcao_raio,
                                       ponto, normal, cor_material, refletividade);

        if(!acertou){
            cor_acumulada += peso * cor_fundo(direcao_raio);
            break;
        }

        vec3 cor_local = cor_material * 0.15;

        for(int i = 0; i < num_luzes; i++){
            vec3 luz = luzes_posicao[i];

            if(!esta_na_sombra(ponto, normal, luz)){
                cor_local += calcula_phong_luz(ponto, normal, cor_material, luz);
            }
        }

        cor_acumulada += peso * cor_local * (1.0 - refletividade);

        if(refletividade < 0.01) break;

        peso *= refletividade;
        origem_raio = ponto + normal * 0.001;
        direcao_raio = reflect(direcao_raio, normal);
    }

    cor_final = vec4(cor_acumulada,1);
}
)";

GLuint v = glCreateShader(GL_VERTEX_SHADER);
glShaderSource(v,1,&vs,nullptr);
glCompileShader(v);

GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
glShaderSource(f,1,&fs,nullptr);
glCompileShader(f);

Shader_programm = glCreateProgram();
glAttachShader(Shader_programm,v);
glAttachShader(Shader_programm,f);
glLinkProgram(Shader_programm);

glDeleteShader(v);
glDeleteShader(f);
}

glm::vec3 calculaFront(){
    float yaw = glm::radians(Cam_yaw);
    float pitch = glm::radians(Cam_pitch);

    return glm::normalize(glm::vec3(
        cos(yaw)*cos(pitch),
        sin(pitch),
        sin(yaw)*cos(pitch)
    ));
}

void trataTeclado(){
    glm::vec3 frente = calculaFront();
    glm::vec3 direita = glm::normalize(glm::cross(frente, glm::vec3(0,1,0)));

    float velocidade = Cam_speed * Tempo_entre_frames;

    if(glfwGetKey(Window,GLFW_KEY_W)) Cam_pos += frente * velocidade;
    if(glfwGetKey(Window,GLFW_KEY_S)) Cam_pos -= frente * velocidade;
    if(glfwGetKey(Window,GLFW_KEY_A)) Cam_pos -= direita * velocidade;
    if(glfwGetKey(Window,GLFW_KEY_D)) Cam_pos += direita * velocidade;
}

void enviaUniformsCamera(){

    glm::vec3 frente = calculaFront();
    glm::vec3 cima_mundo = glm::vec3(0,1,0);
    glm::vec3 direita = glm::normalize(glm::cross(frente, cima_mundo));
    glm::vec3 cima = glm::cross(direita, frente);

    float fov = 60.0f;
    float aspecto = (float)WIDTH / HEIGHT;
    float h = tan(glm::radians(fov)/2);

    glm::vec3 horizontal = 2*h*aspecto*direita;
    glm::vec3 vertical = 2*h*cima;
    glm::vec3 canto = Cam_pos + frente - h*aspecto*direita - h*cima;

    glUniform3fv(glGetUniformLocation(Shader_programm,"cam_origem"),1,glm::value_ptr(Cam_pos));
    glUniform3fv(glGetUniformLocation(Shader_programm,"cam_horizontal"),1,glm::value_ptr(horizontal));
    glUniform3fv(glGetUniformLocation(Shader_programm,"cam_vertical"),1,glm::value_ptr(vertical));
    glUniform3fv(glGetUniformLocation(Shader_programm,"cam_canto_inf_esq"),1,glm::value_ptr(canto));
}

void enviaUniformsCena(){

    glUniform3f(glGetUniformLocation(Shader_programm,"esfera_centro"),0,0,0);
    glUniform1f(glGetUniformLocation(Shader_programm,"esfera_raio"),1);
    glUniform3f(glGetUniformLocation(Shader_programm,"esfera_cor"),0.9,0.9,0.9);
    glUniform1f(glGetUniformLocation(Shader_programm,"esfera_refletividade"),0.9);

    glUniform3f(glGetUniformLocation(Shader_programm,"esfera2_centro"),-1.8,0,0.5);
    glUniform1f(glGetUniformLocation(Shader_programm,"esfera2_raio"),0.5);
    glUniform3f(glGetUniformLocation(Shader_programm,"esfera2_cor"),1,0.4,0.1);
    glUniform1f(glGetUniformLocation(Shader_programm,"esfera2_refletividade"),0.2);

    glUniform3f(glGetUniformLocation(Shader_programm,"esfera3_centro"),1.8,0,0.5);
    glUniform1f(glGetUniformLocation(Shader_programm,"esfera3_raio"),0.5);
    glUniform3f(glGetUniformLocation(Shader_programm,"esfera3_cor"),0.2,0.5,1);
    glUniform1f(glGetUniformLocation(Shader_programm,"esfera3_refletividade"),0.0);

    glm::vec3 luzes[] = {
        glm::vec3(3,4,2),
        glm::vec3(-4,3,1),
        glm::vec3(0,5,-3)
    };

    int n = 3;
    glUniform1i(glGetUniformLocation(Shader_programm,"num_luzes"), n);

    for(int i=0;i<n;i++){
        std::string nome = "luzes_posicao[" + std::to_string(i) + "]";
        glUniform3fv(glGetUniformLocation(Shader_programm,nome.c_str()),1,glm::value_ptr(luzes[i]));
    }
}

void loop(){

    float t0 = glfwGetTime();

    while(!glfwWindowShouldClose(Window)){

        float t = glfwGetTime();
        Tempo_entre_frames = t - t0;
        t0 = t;

        trataTeclado();

        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(Shader_programm);

        enviaUniformsCamera();
        enviaUniformsCena();

        glBindVertexArray(Vao_quad);
        glDrawArrays(GL_TRIANGLES,0,6);

        glfwSwapBuffers(Window);
        glfwPollEvents();
    }
}

int main(){
    inicializaOpenGL();
    inicializaShaders();
    inicializaQuad();
    loop();
    return 0;
}
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

GLFWwindow *Window = nullptr;
GLuint Shader_programm = 0;
GLuint Vao_cubo = 0;

int WIDTH = 800, HEIGHT = 600;

glm::vec3 Cam_pos(0, 0, 3);
glm::vec3 Cam_front(0, 0, -1);
glm::vec3 Cam_up(0, 1, 0);

float Cam_yaw = -90, Cam_pitch = 0;
float lastX = 400, lastY = 300;
bool primeiro_mouse = true;

float Tempo_entre_frames = 0, lastFrame = 0;

bool mouseCapturado = true;
int modoVisualizacao = 0;
int usarBlinn = 1;
int materialAtual = 0;

void atualizarCursor(){
    glfwSetInputMode(Window, GLFW_CURSOR,
        mouseCapturado ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    primeiro_mouse = true;
}

void key_callback(GLFWwindow*,int key,int, int action,int){
    if(action == GLFW_PRESS){
        if(key == GLFW_KEY_M){ 
            mouseCapturado =! mouseCapturado; 
            atualizarCursor();
        }
        if(key == GLFW_KEY_B) {
            usarBlinn =! usarBlinn;
        }
        if(key == GLFW_KEY_N) {
            materialAtual =! materialAtual;
        }

        if(key == GLFW_KEY_0) {
            modoVisualizacao = 0;
        }
        if(key == GLFW_KEY_1) {
            modoVisualizacao = 1;
        }
        if(key == GLFW_KEY_2) {
            modoVisualizacao = 2;
        }
        if(key == GLFW_KEY_3) {
            modoVisualizacao = 3;
        }
        if(key == GLFW_KEY_4) {
            modoVisualizacao = 4;
        }
    }
}

void mouse_callback(GLFWwindow*,double xpos,double ypos){
    if(!mouseCapturado) {
        return;
    }

    if(primeiro_mouse){
        lastX = xpos;
        lastY = ypos;
        primeiro_mouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos; 
    lastY = ypos;

    float sens = 0.1f;
    Cam_yaw += xoffset * sens;
    Cam_pitch += yoffset * sens;

    Cam_pitch = glm::clamp(Cam_pitch, -89.0f, 89.0f);

    glm::vec3 front;
    front.x = cos(glm::radians(Cam_yaw)) * cos(glm::radians(Cam_pitch));
    front.y = sin(glm::radians(Cam_pitch));
    front.z = sin(glm::radians(Cam_yaw)) * cos(glm::radians(Cam_pitch));
    Cam_front = glm::normalize(front);
}

void trataTeclado(){
    float velocidade = 2.5f * Tempo_entre_frames;
    glm::vec3 direita = glm::normalize(glm::cross(Cam_front, Cam_up));

    if(glfwGetKey(Window, GLFW_KEY_W) == GLFW_PRESS) {
        Cam_pos += velocidade * Cam_front;
    }
    if(glfwGetKey(Window, GLFW_KEY_S) == GLFW_PRESS) {
        Cam_pos -= velocidade * Cam_front;
    }
    if(glfwGetKey(Window, GLFW_KEY_A) == GLFW_PRESS) {
        Cam_pos -= direita * velocidade;
    }
    if(glfwGetKey(Window, GLFW_KEY_D) == GLFW_PRESS) {
        Cam_pos += direita * velocidade;
    }
}

void inicializaOpenGL(){
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    Window = glfwCreateWindow(WIDTH, HEIGHT, "Exercício 3 - Rodrigo Lehnen", NULL, NULL);
    glfwMakeContextCurrent(Window);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glfwSetKeyCallback(Window, key_callback);
    glfwSetCursorPosCallback(Window, mouse_callback);

    atualizarCursor();

    cout << "GPU: " << glGetString(GL_RENDERER) << endl;
}

// ================= CUBO =================
void inicializaCubo(){
float v[] = {
// Frente
0.5,0.5,0.5,0,0,1, 0.5,-0.5,0.5,0,0,1, -0.5,-0.5,0.5,0,0,1,
0.5,0.5,0.5,0,0,1, -0.5,-0.5,0.5,0,0,1, -0.5,0.5,0.5,0,0,1,

// Trás
0.5,0.5,-0.5,0,0,-1, 0.5,-0.5,-0.5,0,0,-1, -0.5,-0.5,-0.5,0,0,-1,
0.5,0.5,-0.5,0,0,-1, -0.5,-0.5,-0.5,0,0,-1, -0.5,0.5,-0.5,0,0,-1,

// Esquerda
-0.5,-0.5,0.5,-1,0,0, -0.5,0.5,0.5,-1,0,0, -0.5,-0.5,-0.5,-1,0,0,
-0.5,-0.5,-0.5,-1,0,0, -0.5,0.5,-0.5,-1,0,0, -0.5,0.5,0.5,-1,0,0,

// Direita
0.5,-0.5,0.5,1,0,0, 0.5,0.5,0.5,1,0,0, 0.5,-0.5,-0.5,1,0,0,
0.5,-0.5,-0.5,1,0,0, 0.5,0.5,-0.5,1,0,0, 0.5,0.5,0.5,1,0,0,

// Baixo
-0.5,-0.5,0.5,0,-1,0, 0.5,-0.5,0.5,0,-1,0, 0.5,-0.5,-0.5,0,-1,0,
0.5,-0.5,-0.5,0,-1,0, -0.5,-0.5,-0.5,0,-1,0, -0.5,-0.5,0.5,0,-1,0,

// Cima
-0.5,0.5,0.5,0,1,0, 0.5,0.5,0.5,0,1,0, 0.5,0.5,-0.5,0,1,0,
0.5,0.5,-0.5,0,1,0, -0.5,0.5,-0.5,0,1,0, -0.5,0.5,0.5,0,1,0
};

GLuint VBO;
glGenVertexArrays(1, &Vao_cubo);
glGenBuffers(1, &VBO);

glBindVertexArray(Vao_cubo);
glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);

glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);

glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
glEnableVertexAttribArray(1);
}

// ================= SHADER =================
GLuint compilaShader(const char* src, GLenum t){
GLuint shader = glCreateShader(t);
glShaderSource(shader , 1, &src, NULL);
glCompileShader(shader);
return shader;
}

void inicializaShaders(){

const char * vertex_shader = R"(#version 330 core
layout(location=0) in vec3 pos;
layout(location=1) in vec3 normal;

uniform mat4 model,view,proj;

out vec3 fragPos;
out vec3 fragNormal;

void main(){
vec4 w=model*vec4(pos,1);
fragPos=w.xyz;
fragNormal=mat3(transpose(inverse(model)))*normal;
gl_Position=proj*view*w;
})";

const char * fragment_shader = R"(#version 330 core

struct Light{ vec3 position; vec3 color; };
struct Material{ vec3 color; float Ka,Kd,Ks,shininess; };

#define MAX_LIGHTS 4

in vec3 fragPos;
in vec3 fragNormal;
out vec4 frag_colour;

uniform Light lights[MAX_LIGHTS];
uniform int numLights;

uniform Material material;
uniform vec3 viewPos;
uniform vec3 ambientLightColor;

uniform float att_a,att_b,att_c;

uniform int modoVisualizacao;
uniform int usarBlinn;

void main(){

vec3 N=normalize(fragNormal);
vec3 V=normalize(viewPos-fragPos);

if(modoVisualizacao==4){
frag_colour=vec4(N*0.5+0.5,1);
return;
}

vec3 ambient=material.Ka*ambientLightColor;
vec3 diffTot=vec3(0);
vec3 specTot=vec3(0);

for(int i=0;i<numLights;i++){

vec3 L=normalize(lights[i].position-fragPos);
float d=length(lights[i].position-fragPos);
float att=1.0/(att_a+att_b*d+att_c*d*d);

float diff=max(dot(N,L),0);
vec3 diffuse=material.Kd*diff*lights[i].color;

float spec;
if(usarBlinn==1){
vec3 H=normalize(L+V);
spec=pow(max(dot(N,H),0),material.shininess);
}else{
vec3 R=reflect(-L,N);
spec=pow(max(dot(R,V),0),material.shininess);
}

vec3 specular=material.Ks*spec*lights[i].color;

diffTot+=diffuse*att;
specTot+=specular*att;
}

if(modoVisualizacao==1)
frag_colour=vec4(ambient*material.color,1);
else if(modoVisualizacao==2)
frag_colour=vec4(diffTot*material.color,1);
else if(modoVisualizacao==3)
frag_colour=vec4(specTot*material.color,1);
else
frag_colour=vec4((ambient+diffTot+specTot)*material.color,1);
})";

GLuint vs = compilaShader(vertex_shader, GL_VERTEX_SHADER);
GLuint fs = compilaShader(fragment_shader, GL_FRAGMENT_SHADER);

Shader_programm = glCreateProgram();
glAttachShader(Shader_programm, vs);
glAttachShader(Shader_programm, fs);
glLinkProgram(Shader_programm);
}

void inicializaRenderizacao(){

glEnable(GL_DEPTH_TEST);

while(!glfwWindowShouldClose(Window)){

float tempo_atual = glfwGetTime();
Tempo_entre_frames = tempo_atual - lastFrame;
lastFrame = tempo_atual;

trataTeclado();

glClearColor(0.1, 0.1, 0.1, 1);
glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

glUseProgram(Shader_programm);

glm::mat4 view = glm::lookAt(Cam_pos, Cam_pos + Cam_front, Cam_up);
glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)WIDTH / HEIGHT, 0.1f, 100.f);

glUniformMatrix4fv(glGetUniformLocation(Shader_programm, "view"), 1, 0, glm::value_ptr(view));
glUniformMatrix4fv(glGetUniformLocation(Shader_programm, "proj"), 1, 0, glm::value_ptr(proj));

glm::mat4 model(1);
glUniformMatrix4fv(glGetUniformLocation(Shader_programm, "model"), 1, 0, glm::value_ptr(model));

glm::vec3 l1(2 * cos(tempo_atual), 1.5, 2 * sin(tempo_atual));
glm::vec3 l2(-2, 2, -2);

glUniform1i(glGetUniformLocation(Shader_programm, "numLights"), 2);

glUniform3fv(glGetUniformLocation(Shader_programm, "lights[0].position"), 1, glm::value_ptr(l1));
glUniform3f(glGetUniformLocation(Shader_programm, "lights[0].color"), 1, 1, 1);

glUniform3fv(glGetUniformLocation(Shader_programm, "lights[1].position"), 1, glm::value_ptr(l2));
glUniform3f(glGetUniformLocation(Shader_programm, "lights[1].color"), 0, 0, 1);

glUniform3f(glGetUniformLocation(Shader_programm, "ambientLightColor"), 0.2, 0.2, 0.2);

glUniform1f(glGetUniformLocation(Shader_programm, "att_a"), 1);
glUniform1f(glGetUniformLocation(Shader_programm, "att_b"), 0.1);
glUniform1f(glGetUniformLocation(Shader_programm, "att_c"), 0.05);

glUniform1i(glGetUniformLocation(Shader_programm, "modoVisualizacao"), modoVisualizacao);
glUniform1i(glGetUniformLocation(Shader_programm, "usarBlinn"), usarBlinn);

if(materialAtual == 0){
glUniform3f(glGetUniformLocation(Shader_programm, "material.color"), 1, 0.5, 0.2);
glUniform1f(glGetUniformLocation(Shader_programm, "material.Ka"), 0.1);
glUniform1f(glGetUniformLocation(Shader_programm, "material.Kd"), 0.7);
glUniform1f(glGetUniformLocation(Shader_programm, "material.Ks"), 0.3);
glUniform1f(glGetUniformLocation(Shader_programm, "material.shininess"), 32);
} else {
glUniform3f(glGetUniformLocation(Shader_programm, "material.color"), 0.8, 0.8, 0.8);
glUniform1f(glGetUniformLocation(Shader_programm, "material.Ka"), 0.05);
glUniform1f(glGetUniformLocation(Shader_programm, "material.Kd"), 0.2);
glUniform1f(glGetUniformLocation(Shader_programm, "material.Ks"), 1.0);
glUniform1f(glGetUniformLocation(Shader_programm, "material.shininess"), 128);
}

glUniform3fv(glGetUniformLocation(Shader_programm, "viewPos"), 1, glm::value_ptr(Cam_pos));

glBindVertexArray(Vao_cubo);
glDrawArrays(GL_TRIANGLES, 0, 36);

glfwSwapBuffers(Window);
glfwPollEvents();
}

glfwTerminate();
}

int main(){
inicializaOpenGL();
inicializaCubo();
inicializaShaders();
inicializaRenderizacao();
return 0;
}
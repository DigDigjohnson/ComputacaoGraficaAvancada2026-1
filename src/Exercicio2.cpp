#include <iostream>
#include <vector>
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

GLFWwindow* Window = nullptr;
GLuint Shader_programm = 0;
GLuint Vao_cubo = 0;

int WIDTH = 1440;
int HEIGHT = 1080;

float Tempo_entre_frames = 0.0f;
float Tempo_total = 0.0f;

float Cam_speed = 10.0f;
glm::vec3 Cam_pos(0.0f, 0.0f, 2.0f);
float Cam_yaw = 0.0f;
float Cam_pitch = 0.0f;

double lastX = WIDTH / 2.0;
double lastY = HEIGHT / 2.0;
bool primeiro_mouse = true;

float FOV = 67.0f;

bool modo_profundidade = false;
bool modo_wireframe = false;
bool efeito_distancia = true;
bool mouse_fps = true;

struct Cubo {
    glm::vec3 pos;
    glm::vec3 escala;
    glm::vec4 cor;
    bool cor_animada;
    bool orbitar;
};

vector<Cubo> cubos;

void redimensionaCallback(GLFWwindow* window, int width, int height)
{
    WIDTH = width;
    HEIGHT = height;
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (!mouse_fps) {
        return;
    }

    if (primeiro_mouse)
    {
        lastX = xpos;
        lastY = ypos;
        primeiro_mouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensibilidade = 0.1f;
    xoffset *= sensibilidade;
    yoffset *= sensibilidade;

    Cam_yaw += xoffset;
    Cam_pitch += yoffset;
}

void salvaFrame()
{
    int width = WIDTH, height = HEIGHT;
    vector<unsigned char> pixels(width * height * 3);

    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    FILE* f = fopen("frame.ppm", "wb");
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    fwrite(pixels.data(), 1, pixels.size(), f);
    fclose(f);

    cout << "Frame salvo como frame.ppm\n";

    int result = system("cd /Users/rodrigolehnen/Documents/Workspaces/College/FCG2025-1 && python3 ../convert.py");

    if (result == 0) {
        cout << "PNG gerado com sucesso!\n";
    } else {
        cout << "Erro ao converter PNG\n";
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_P) {
            modo_profundidade = !modo_profundidade;
        }

        if (key == GLFW_KEY_L)
        {
            modo_wireframe = !modo_wireframe;
            glPolygonMode(GL_FRONT_AND_BACK, modo_wireframe ? GL_LINE : GL_FILL);
        }

        if (key == GLFW_KEY_O) {
            efeito_distancia = !efeito_distancia;
        }

        if (key == GLFW_KEY_M)
        {
            mouse_fps = !mouse_fps;
            glfwSetInputMode(Window, GLFW_CURSOR,
                mouse_fps ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        }

        if (key == GLFW_KEY_KP_ADD) {
            FOV -= 2.0f;
        }
        if (key == GLFW_KEY_KP_SUBTRACT) {
            FOV += 2.0f;
        }

        if (key == GLFW_KEY_F) {
            salvaFrame();
        }
    }
}

void inicializaOpenGL()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    Window = glfwCreateWindow(WIDTH, HEIGHT, "Exercício 2 - Rodrigo Lehnen", nullptr, nullptr);
    glfwMakeContextCurrent(Window);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glfwSetFramebufferSizeCallback(Window, redimensionaCallback);
    glfwSetCursorPosCallback(Window, mouse_callback);
    glfwSetKeyCallback(Window, key_callback);
    glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);
}

void inicializaCubo()
{
    float points[] = {
        0.5,0.5,0.5, 0.5,-0.5,0.5, -0.5,-0.5,0.5,
        0.5,0.5,0.5, -0.5,-0.5,0.5, -0.5,0.5,0.5,
        0.5,0.5,-0.5, 0.5,-0.5,-0.5, -0.5,-0.5,-0.5,
        0.5,0.5,-0.5, -0.5,-0.5,-0.5, -0.5,0.5,-0.5,
        -0.5,-0.5,0.5, -0.5,0.5,0.5, -0.5,-0.5,-0.5,
        -0.5,-0.5,-0.5, -0.5,0.5,-0.5, -0.5,0.5,0.5,
        0.5,-0.5,0.5, 0.5,0.5,0.5, 0.5,-0.5,-0.5,
        0.5,-0.5,-0.5, 0.5,0.5,-0.5, 0.5,0.5,0.5,
        -0.5,-0.5,0.5, 0.5,-0.5,0.5, 0.5,-0.5,-0.5,
        0.5,-0.5,-0.5, -0.5,-0.5,-0.5, -0.5,-0.5,0.5,
        -0.5,0.5,0.5, 0.5,0.5,0.5, 0.5,0.5,-0.5,
        0.5,0.5,-0.5, -0.5,0.5,-0.5, -0.5,0.5,0.5
    };

    GLuint VBO;
    glGenVertexArrays(1, &Vao_cubo);
    glGenBuffers(1, &VBO);

    glBindVertexArray(Vao_cubo);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
}

GLuint compilaShader(const char* source, GLenum type)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    return shader;
}

void inicializaShaders()
{
    const char* vertex_shader = R"(
        #version 410 core
        layout(location = 0) in vec3 vertex_posicao;

        uniform mat4 transform;
        uniform mat4 view;
        uniform mat4 proj;

        out vec3 fragPos;

        void main() {
            vec4 world = transform * vec4(vertex_posicao,1.0);
            fragPos = world.xyz;
            gl_Position = proj * view * world;
        }
    )";

    const char* fragment_shader = R"(
        #version 410 core

        in vec3 fragPos;
        out vec4 frag_colour;

        uniform vec4 corobjeto;
        uniform float tempo;
        uniform bool cor_animada;
        uniform bool modo_profundidade;
        uniform bool efeito_distancia;
        uniform vec3 camPos;

        void main() {

            if (modo_profundidade) {
                float depth = gl_FragCoord.z;
                frag_colour = vec4(vec3(depth), 1.0);
                return;
            }

            vec4 cor = corobjeto;

            if (cor_animada) {
                cor = vec4(
                    (sin(tempo)+1)/2,
                    (cos(tempo)+1)/2,
                    0.5,1.0
                );
            }

            if (efeito_distancia) {
                float dist = length(camPos - fragPos);
                float fator = 1.0 / (1.0 + 0.2 * dist);
                cor.rgb *= fator;
            }

            frag_colour = cor;
        }
    )";

    GLuint vs = compilaShader(vertex_shader, GL_VERTEX_SHADER);
    GLuint fs = compilaShader(fragment_shader, GL_FRAGMENT_SHADER);

    Shader_programm = glCreateProgram();
    glAttachShader(Shader_programm, vs);
    glAttachShader(Shader_programm, fs);
    glLinkProgram(Shader_programm);

    glDeleteShader(vs);
    glDeleteShader(fs);
}

void transformacaoGenerica(float Tx,float Ty,float Tz,
                           float Sx,float Sy,float Sz,
                           float Rx,float Ry,float Rz)
{
    glm::mat4 transform(1.0f);

    transform = glm::translate(transform, glm::vec3(Tx,Ty,Tz));
    transform = glm::rotate(transform, glm::radians(Ry), glm::vec3(0,1,0));
    transform = glm::scale(transform, glm::vec3(Sx,Sy,Sz));

    GLuint loc = glGetUniformLocation(Shader_programm, "transform");
    glUniformMatrix4fv(loc,1,GL_FALSE,glm::value_ptr(transform));
}

void especificaMatrizVisualizacao()
{
    glm::vec3 front;
    front.x = cos(glm::radians(Cam_yaw)) * cos(glm::radians(Cam_pitch));
    front.y = sin(glm::radians(Cam_pitch));
    front.z = sin(glm::radians(Cam_yaw)) * cos(glm::radians(Cam_pitch));
    front = glm::normalize(front);

    glm::mat4 view = glm::lookAt(Cam_pos, Cam_pos + front, glm::vec3(0,1,0));

    GLuint loc = glGetUniformLocation(Shader_programm, "view");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));
}

void especificaMatrizProjecao()
{
    glm::mat4 proj = glm::perspective(glm::radians(FOV), (float)WIDTH/HEIGHT, 0.1f, 100.0f);

    GLuint loc = glGetUniformLocation(Shader_programm, "proj");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(proj));
}

void inicializaCamera()
{
    especificaMatrizVisualizacao();
    especificaMatrizProjecao();
}

void trataTeclado()
{
    float velocidade = Cam_speed * Tempo_entre_frames;

    glm::vec3 frente;
    frente.x = cos(glm::radians(Cam_yaw)) * cos(glm::radians(Cam_pitch));
    frente.y = sin(glm::radians(Cam_pitch));
    frente.z = sin(glm::radians(Cam_yaw)) * cos(glm::radians(Cam_pitch));
    frente = glm::normalize(frente);

    glm::vec3 direita = glm::normalize(glm::cross(frente, glm::vec3(0,1,0)));

    if (glfwGetKey(Window, GLFW_KEY_W) == GLFW_PRESS)
        Cam_pos += frente * velocidade;
    if (glfwGetKey(Window, GLFW_KEY_S) == GLFW_PRESS)
        Cam_pos -= frente * velocidade;
    if (glfwGetKey(Window, GLFW_KEY_A) == GLFW_PRESS)
        Cam_pos -= direita * velocidade;
    if (glfwGetKey(Window, GLFW_KEY_D) == GLFW_PRESS)
        Cam_pos += direita * velocidade;
}

void inicializaRenderizacao()
{
    float tempo_anterior = glfwGetTime();

    while(!glfwWindowShouldClose(Window))
    {
        float tempo_atual = glfwGetTime();
        Tempo_entre_frames = tempo_atual - tempo_anterior;
        tempo_anterior = tempo_atual;
        Tempo_total += Tempo_entre_frames;

        glClearColor(0.2f,0.3f,0.3f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(Shader_programm);
        inicializaCamera();

        glUniform1f(glGetUniformLocation(Shader_programm, "tempo"), Tempo_total);
        glUniform1i(glGetUniformLocation(Shader_programm, "modo_profundidade"), modo_profundidade);
        glUniform1i(glGetUniformLocation(Shader_programm, "efeito_distancia"), efeito_distancia);
        glUniform3fv(glGetUniformLocation(Shader_programm, "camPos"), 1, glm::value_ptr(Cam_pos));

        glBindVertexArray(Vao_cubo);

        for (auto& c : cubos)
        {
            glUniform1i(glGetUniformLocation(Shader_programm, "cor_animada"), c.cor_animada);
            glUniform4fv(glGetUniformLocation(Shader_programm, "corobjeto"), 1, &c.cor[0]);

            glm::vec3 pos = c.pos;

            if (c.orbitar)
            {
                float r = 2.0f;
                pos.x = cos(Tempo_total)*r;
                pos.z = sin(Tempo_total)*r;
            }

            transformacaoGenerica(
                pos.x, pos.y, pos.z,
                c.escala.x, c.escala.y, c.escala.z,
                0, Tempo_total * 50.0f, 0
            );

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(Window);
        glfwPollEvents();
        trataTeclado();
    }

    glfwTerminate();
}

void inicializaCena()
{
    cubos.push_back({{0, 0, 0},{1, 1, 1},{1, 0, 0, 1}, false, false});
    cubos.push_back({{2, 0, 0},{1, 1, 1},{0, 1, 0, 1}, true, false});
    cubos.push_back({{0, 0, 0},{0.5, 0.5, 0.5},{0, 0, 1, 1}, false, true});
}

int main()
{
    inicializaOpenGL();
    inicializaCubo();
    inicializaShaders();
    inicializaCena();
    inicializaRenderizacao();
    return 0;
}
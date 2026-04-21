// Câmera FPS — Exemplo 5
//
// Este exemplo é idêntico ao Exemplo 4 — mesma cena, mesmo ray tracing,
// mesma reflexão. A única adição é a câmera interativa no estilo FPS:
//   W/A/S/D — mover câmera
//   Mouse   — girar câmera
//
// Isso permite orbitar a esfera espelho e observar o reflexo de diferentes
// ângulos — algo impossível de fazer nos exemplos anteriores com câmera fixa.
//
// Como funciona a câmera FPS aqui:
//   Nos exemplos anteriores, cam_origem era um valor fixo calculado uma vez.
//   Agora, cam_origem muda a cada frame conforme o movimento do jogador.
//   Os vetores horizontal, vertical e canto_inf_esq são recalculados todo
//   frame a partir da nova posição e orientação da câmera.
//
//   O shader não sabe que a câmera se moveu — ele recebe os mesmos uniforms
//   de sempre (cam_origem, cam_horizontal, etc.), só que com valores novos.
//   Isso mostra que o ray tracer no shader é completamente independente de
//   como a câmera é controlada no Python.
//
// Controles:
//   W/A/S/D — mover câmera
//   Mouse   — girar câmera
//   ESC     — fechar

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <string>

// -----------------------------
// Variáveis globais
// -----------------------------

GLFWwindow* Window = nullptr;
GLuint Shader_programm;
GLuint Vao_quad;

const int WIDTH = 800;
const int HEIGHT = 600;

float Tempo_entre_frames = 0.0f;

// -----------------------------
// Estado da câmera FPS
// -----------------------------
// A câmera é definida por posição, yaw e pitch — igual aos exemplos de
// rasterização das aulas anteriores. A diferença é que aqui os vetores
// resultantes são enviados ao ray tracer como uniforms, não como matriz.

glm::vec3 Cam_pos = glm::vec3(0.0f, 0.5f, 4.0f);
float Cam_yaw = 270.0f;  // graus — 270 = olhando para -Z (em direção às esferas)
float Cam_pitch = -5.0f; // graus — levemente inclinado para baixo
float Cam_speed = 5.0f;

float lastX = WIDTH / 2.0f, lastY = HEIGHT / 2.0f;
bool primeiro_mouse = true;

// Protótipos das funções de callback
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// =============================================================================
// INICIALIZAÇÃO DO OPENGL
// =============================================================================

void inicializaOpenGL() {
    if (!glfwInit()) {
        std::cerr << "Falha ao inicializar GLFW" << std::endl;
        exit(-1);
    }

    Window = glfwCreateWindow(WIDTH, HEIGHT, "Câmera FPS — Exemplo 5", nullptr, nullptr);
    if (!Window) {
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContext_current(Window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Falha ao inicializar GLAD" << std::endl;
        exit(-1);
    }

    // Captura o cursor dentro da janela (modo FPS)
    glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetKeyCallback(Window, key_callback);
    glfwSetCursorPosCallback(Window, mouse_callback);

    std::cout << "Placa de vídeo: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Versão OpenGL: " << glGetString(GL_VERSION) << std::endl;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    /*
    Chamada automaticamente pelo GLFW toda vez que o mouse se move.
    Atualiza yaw (rotação horizontal) e pitch (rotação vertical) da câmera.
    */
    if (primeiro_mouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        primeiro_mouse = false;
        return;
    }

    // Quanto o mouse se moveu desde o último frame
    float dx = (float)xpos - lastX;
    float dy = lastY - (float)ypos; // invertido: mouse para cima = pitch positivo (olhar para cima)
    lastX = (float)xpos;
    lastY = (float)ypos;

    float sensibilidade = 0.1f; // quanto cada pixel de movimento rotaciona a câmera
    Cam_yaw += dx * sensibilidade;
    Cam_pitch += dy * sensibilidade;

    // Limita o pitch para não deixar a câmera "virar de cabeça pra baixo"
    if (Cam_pitch > 89.0f) Cam_pitch = 89.0f;
    if (Cam_pitch < -89.0f) Cam_pitch = -89.0f;
}

// =============================================================================
// QUAD DE TELA INTEIRA — idêntico aos exemplos anteriores
// =============================================================================

void inicializaQuad() {
    float vertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,

        -1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f
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

// =============================================================================
// SHADERS — idênticos ao Exemplo 4
// =============================================================================

void inicializaShaders() {
    const char* vertex_shader_src = R"(
        #version 400
        layout(location = 0) in vec2 posicao;
        out vec2 uv;
        void main() {
            uv = posicao * 0.5 + 0.5;
            gl_Position = vec4(posicao, 0.0, 1.0);
        }
    )";

    // Fragment shader idêntico ao Exemplo 4 — não sabe nada sobre FPS.
    // Recebe os mesmos uniforms de sempre; só os valores mudam a cada frame.
    const char* fragment_shader_src = R"(
        #version 400

        in  vec2 uv;
        out vec4 cor_final;

        uniform vec3 cam_origem;
        uniform vec3 cam_horizontal;
        uniform vec3 cam_vertical;
        uniform vec3 cam_canto_inf_esq;

        uniform vec3  esfera_centro;
        uniform float esfera_raio;
        uniform vec3  esfera_cor;
        uniform float esfera_refletividade;

        uniform vec3  esfera2_centro;
        uniform float esfera2_raio;
        uniform vec3  esfera2_cor;
        uniform float esfera2_refletividade;

        uniform vec3  esfera3_centro;
        uniform float esfera3_raio;
        uniform vec3  esfera3_cor;
        uniform float esfera3_refletividade;

        uniform vec3 luz_posicao;

        float intersecta_esfera(vec3 origem, vec3 direcao, vec3 centro, float raio) {
            vec3  oc = origem - centro;
            float a  = dot(direcao, direcao);
            float b  = 2.0 * dot(direcao, oc);
            float c  = dot(oc, oc) - raio*raio;
            float discriminante = b*b - 4.0*a*c;
            if (discriminante < 0.0) return -1.0;
            float t1 = (-b - sqrt(discriminante)) / (2.0 * a);
            float t2 = (-b + sqrt(discriminante)) / (2.0 * a);
            if (t1 > 0.0) return t1;
            if (t2 > 0.0) return t2;
            return -1.0;
        }
        
        float intersecta_plano(vec3 origem, vec3 direcao, vec3 ponto_plano, vec3 normal_plano) {
            float denom = dot(normal_plano, direcao);
            if (abs(denom) > 1e-6) {
                float t = dot(ponto_plano - origem, normal_plano) / denom;
                if (t >= 0.0) return t;
            }
            return -1.0;
        }

        bool intersecta_cena(vec3 origem, vec3 direcao,
                             out vec3 ponto, out vec3 normal,
                             out vec3 cor,   out float reflet) {
            float t_min   = 1e10;
            bool  acertou = false;
            float t;

            t = intersecta_esfera(origem, direcao, esfera_centro, esfera_raio);
            if (t > 0.0 && t < t_min) {
                t_min = t; ponto = origem + t*direcao;
                normal = normalize(ponto - esfera_centro);
                cor = esfera_cor; reflet = esfera_refletividade; acertou = true;
            }
            t = intersecta_esfera(origem, direcao, esfera2_centro, esfera2_raio);
            if (t > 0.0 && t < t_min) {
                t_min = t; ponto = origem + t*direcao;
                normal = normalize(ponto - esfera2_centro);
                cor = esfera2_cor; reflet = esfera2_refletividade; acertou = true;
            }
            t = intersecta_esfera(origem, direcao, esfera3_centro, esfera3_raio);
            if (t > 0.0 && t < t_min) {
                t_min = t; ponto = origem + t*direcao;
                normal = normalize(ponto - esfera3_centro);
                cor = esfera3_cor; reflet = esfera3_refletividade; acertou = true;
            }
            
            vec3 chao_normal = vec3(0.0, 1.0, 0.0);
            vec3 chao_ponto  = vec3(0.0, -1.0, 0.0);
            float t_chao = intersecta_plano(origem, direcao, chao_ponto, chao_normal);
            
            if (t_chao > 0.0 && t_chao < t_min) {
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

        bool esta_na_sombra(vec3 ponto_de_impacto, vec3 normal) {
            vec3  dir_luz       = normalize(luz_posicao - ponto_de_impacto);
            float dist_ate_luz  = length(luz_posicao - ponto_de_impacto);
            vec3  origem_shadow = ponto_de_impacto + normal * 0.001;
            float t1 = intersecta_esfera(origem_shadow, dir_luz, esfera_centro,  esfera_raio);
            float t2 = intersecta_esfera(origem_shadow, dir_luz, esfera2_centro, esfera2_raio);
            float t3 = intersecta_esfera(origem_shadow, dir_luz, esfera3_centro, esfera3_raio);
            return (t1 > 0.0 && t1 < dist_ate_luz)
                || (t2 > 0.0 && t2 < dist_ate_luz)
                || (t3 > 0.0 && t3 < dist_ate_luz);
        }

        vec3 calcula_phong(vec3 ponto, vec3 normal, vec3 cor_material) {
            vec3  L         = normalize(luz_posicao - ponto);
            vec3  V         = normalize(cam_origem  - ponto);
            vec3  ambiente  = cor_material * 0.15;
            float Kd        = max(dot(normal, L), 0.0);
            vec3  difusa    = cor_material * Kd * 0.75;
            vec3  R         = reflect(-L, normal);
            float Ks        = pow(max(dot(R, V), 0.0), 32.0);
            vec3  especular = vec3(1.0) * Ks * 0.6;
            return ambiente + difusa + especular;
        }

        vec3 cor_fundo(vec3 direcao) {
            float t = direcao.y * 0.5 + 0.5;
            return mix(vec3(0.1, 0.1, 0.3), vec3(0.5, 0.7, 1.0), t);
        }

        void main() {
            vec3 pixel_no_mundo = cam_canto_inf_esq
                                + uv.x * cam_horizontal
                                + uv.y * cam_vertical;

            vec3 origem_raio  = cam_origem;
            vec3 direcao_raio = normalize(pixel_no_mundo - cam_origem);

            vec3  cor_acumulada = vec3(0.0);
            float peso          = 1.0;

            for (int bounce = 0; bounce < 4; bounce++) {
                vec3  ponto, normal, cor_mat;
                float reflet;

                bool acertou = intersecta_cena(origem_raio, direcao_raio,
                                               ponto, normal, cor_mat, reflet);
                if (!acertou) {
                    cor_acumulada += peso * cor_fundo(direcao_raio);
                    break;
                }

                vec3 cor_local;
                if (esta_na_sombra(ponto, normal)) {
                    cor_local = cor_mat * 0.15;
                } else {
                    cor_local = calcula_phong(ponto, normal, cor_mat);
                }

                cor_acumulada += peso * cor_local * (1.0 - reflet);
                if (reflet < 0.01) break;

                peso         *= reflet;
                origem_raio   = ponto + normal * 0.001;
                direcao_raio  = reflect(direcao_raio, normal);
            }
            cor_final = vec4(cor_acumulada, 1.0);
        }
    )";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader_src, nullptr);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader_src, nullptr);
    glCompileShader(fs);

    Shader_programm = glCreateProgram();
    glAttachShader(Shader_programm, vs);
    glAttachShader(Shader_programm, fs);
    glLinkProgram(Shader_programm);

    glDeleteShader(vs);
    glDeleteShader(fs);
}

// =============================================================================
// CÂMERA FPS
// =============================================================================

glm::vec3 calculaFront() {
    /*
    Calcula o vetor 'frente' da câmera a partir do yaw e pitch atuais.

    yaw   = rotação horizontal (esquerda/direita), em graus
    pitch = rotação vertical   (cima/baixo),       em graus

    A fórmula converte ângulos esféricos para vetor cartesiano:
      x = cos(yaw) * cos(pitch)
      y = sin(pitch)
      z = sin(yaw) * cos(pitch)
    */
    float yaw_rad = glm::radians(Cam_yaw);
    float pitch_rad = glm::radians(Cam_pitch);

    glm::vec3 frente;
    frente.x = cos(yaw_rad) * cos(pitch_rad);
    frente.y = sin(pitch_rad);
    frente.z = sin(yaw_rad) * cos(pitch_rad);

    return glm::normalize(frente);
}

void trataTeclado() {
    /*
    Lê as teclas WASD a cada frame e move a câmera na direção correspondente.
    Multiplicamos pela velocidade e pelo tempo entre frames para que o
    movimento seja independente do FPS.
    */
    glm::vec3 frente = calculaFront();
    glm::vec3 direita = glm::normalize(glm::cross(frente, glm::vec3(0.0f, 1.0f, 0.0f)));

    float velocidade = Cam_speed * Tempo_entre_frames;

    if (glfwGetKey(Window, GLFW_KEY_W) == GLFW_PRESS)
        Cam_pos += frente * velocidade;
    if (glfwGetKey(Window, GLFW_KEY_S) == GLFW_PRESS)
        Cam_pos -= frente * velocidade;
    if (glfwGetKey(Window, GLFW_KEY_A) == GLFW_PRESS)
        Cam_pos -= direita * velocidade;
    if (glfwGetKey(Window, GLFW_KEY_D) == GLFW_PRESS)
        Cam_pos += direita * velocidade;
}

void enviaUniformsCamera() {
    /*
    Reconstrói os vetores do plano de pixels a cada frame com base na
    posição e orientação atuais da câmera.

    O shader recebe exatamente os mesmos uniforms dos exemplos anteriores —
    a câmera FPS é transparente para o ray tracer.
    */
    glm::vec3 frente = calculaFront();
    glm::vec3 cima_mundo = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 direita = glm::normalize(glm::cross(frente, cima_mundo));
    glm::vec3 cima = glm::cross(direita, frente);

    float fov = 60.0f;
    float aspecto = (float)WIDTH / (float)HEIGHT;
    float half_h = tan(glm::radians(fov) / 2.0f);
    float half_w = half_h * aspecto;

    glm::vec3 horizontal = 2.0f * half_w * direita;
    glm::vec3 vertical = 2.0f * half_h * cima;
    glm::vec3 canto_inf_esq = Cam_pos + frente - half_w * direita - half_h * cima;

    glUniform3fv(glGetUniformLocation(Shader_programm, "cam_origem"), 1, glm::value_ptr(Cam_pos));
    glUniform3fv(glGetUniformLocation(Shader_programm, "cam_horizontal"), 1, glm::value_ptr(horizontal));
    glUniform3fv(glGetUniformLocation(Shader_programm, "cam_vertical"), 1, glm::value_ptr(vertical));
    glUniform3fv(glGetUniformLocation(Shader_programm, "cam_canto_inf_esq"), 1, glm::value_ptr(canto_inf_esq));
}

// =============================================================================
// CENA — idêntica ao Exemplo 4
// =============================================================================

void enviaUniformsCena() {
    glUniform3f(glGetUniformLocation(Shader_programm, "esfera_centro"), 0.0f, 0.0f, 0.0f);
    glUniform1f(glGetUniformLocation(Shader_programm, "esfera_raio"), 1.0f);
    glUniform3f(glGetUniformLocation(Shader_programm, "esfera_cor"), 0.9f, 0.9f, 0.9f);
    glUniform1f(glGetUniformLocation(Shader_programm, "esfera_refletividade"), 0.9f);

    glUniform3f(glGetUniformLocation(Shader_programm, "esfera2_centro"), -1.8f, 0.0f, 0.5f);
    glUniform1f(glGetUniformLocation(Shader_programm, "esfera2_raio"), 0.5f);
    glUniform3f(glGetUniformLocation(Shader_programm, "esfera2_cor"), 1.0f, 0.4f, 0.1f);
    glUniform1f(glGetUniformLocation(Shader_programm, "esfera2_refletividade"), 0.0f);

    glUniform3f(glGetUniformLocation(Shader_programm, "esfera3_centro"), 1.8f, 0.0f, 0.5f);
    glUniform1f(glGetUniformLocation(Shader_programm, "esfera3_raio"), 0.5f);
    glUniform3f(glGetUniformLocation(Shader_programm, "esfera3_cor"), 0.2f, 0.5f, 1.0f);
    glUniform1f(glGetUniformLocation(Shader_programm, "esfera3_refletividade"), 0.0f);

    glUniform3f(glGetUniformLocation(Shader_programm, "luz_posicao"), 3.0f, 4.0f, 2.0f);
}

// =============================================================================
// LOOP PRINCIPAL
// =============================================================================

void inicializaRenderizacao() {
    std::cout << "\n--- Câmera FPS — Exemplo 5 ---" << std::endl;
    std::cout << "  Mesmo ray tracing do Exemplo 4, com câmera interativa." << std::endl;
    std::cout << "  W/A/S/D — mover câmera" << std::endl;
    std::cout << "  Mouse   — girar câmera" << std::endl;
    std::cout << "  ESC     — fechar\n" << std::endl;
    std::cout << "  Dica: orbite a esfera espelho e observe como o reflexo" << std::endl;
    std::cout << "  muda de ângulo — cada pixel dispara um raio diferente.\n" << std::endl;

    float tempo_anterior = (float)glfwGetTime();

    while (!glfwWindowShouldClose(Window)) {
        float tempo_atual = (float)glfwGetTime();
        Tempo_entre_frames = tempo_atual - tempo_anterior;
        tempo_anterior = tempo_atual;

        trataTeclado();

        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(0, 0, WIDTH, HEIGHT);

        glUseProgram(Shader_programm);
        enviaUniformsCamera(); // recalculado todo frame com a posição atual
        enviaUniformsCena();   // estático — a cena não muda

        glBindVertexArray(Vao_quad);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(Window);
        glfwPollEvents();
    }

    glfwTerminate();
}

// =============================================================================
// MAIN
// =============================================================================

int main() {
    inicializaOpenGL();
    inicializaShaders();
    inicializaQuad();
    inicializaRenderizacao();
    return 0;
}
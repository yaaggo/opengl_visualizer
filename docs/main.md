# Entrada e Inicialização da Aplicação - `main.cpp`

O arquivo [main.cpp](file:///C:/Users/yagog/Documents/coding/comp-grafic/opengl_visualizer/src/main.cpp) é o ponto de entrada oficial do programa. Sua responsabilidade básica é inicializar a biblioteca **FreeGLUT**, configurar os modos de exibição gráfica, configurar a janela e registrar os *callbacks* globais do sistema.

---

## 🔍 Análise Detalhada do Código

### 1. Estruturas de Configuração e Estado
O programa define duas estruturas simples para encapsular as variáveis globais de configuração da janela:

```cpp
struct app_config {
    int window_width;
    int window_height;
    const char* window_title;
};

struct app_state {
    app_config config;
};

app_state g_app;
```

* **Motivação:** Centralizar as propriedades da janela (como largura, altura e título) em uma estrutura global `g_app`. Isso facilita modificações futuras e mantém o estado organizado em vez de utilizar diversas variáveis globais dispersas.

### 2. Inicialização do Estado
A função `init_app_state()` define os valores padrão da janela:

```cpp
void init_app_state() {
    g_app.config.window_width = 1280;
    g_app.config.window_height = 720;
    g_app.config.window_title = "OpenGL Visualizer";
}
```
* **Motivação:** A resolução `1280x720` (proporção 16:9) é estabelecida como padrão. A proporção é de extrema importância porque o sistema de redimensionamento (`reshape_callback`) foi projetado para manter a proporção de 16:9 em qualquer resolução via técnica de *letterboxing* (faixas pretas laterais ou verticais).

### 3. Função Principal (`main`)
A função `main` orquestra a inicialização e entra no loop principal do GLUT:

```cpp
int main(int argc, char** argv) {
    init_app_state();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(g_app.config.window_width, g_app.config.window_height);
    glutCreateWindow(g_app.config.window_title);
    glutFullScreen();
```

* **`glutInitDisplayMode(...)`**:
  * `GLUT_DOUBLE`: Habilita o buffer duplo (*Double Buffering*). Essencial em aplicações interativas para evitar oscilações visuais (*flickering*), renderizando o próximo quadro em um buffer oculto antes de trocá-lo com o buffer visível na tela.
  * `GLUT_RGB`: Habilita o buffer de cores no formato RGB.
  * `GLUT_DEPTH`: Habilita o buffer de profundidade (*Depth Buffer* ou *Z-Buffer*), que rastreia a distância de cada pixel em relação à câmera. Fundamental para a renderização correta de sólidos 3D (para que objetos mais próximos ocultem os mais distantes).
* **`glutFullScreen()`**: Força a inicialização da aplicação em tela cheia para maximizar a imersão e garantir que toda a área útil do monitor seja aproveitada.

### 4. Registro de Callbacks
Os callbacks são pontes de eventos que ligam eventos do sistema operacional (renderização de quadros, cliques de mouse, movimento de cursor, pressionamento de teclas) às funções internas que processam esses eventos:

```cpp
    glutDisplayFunc(display_callback);
    glutReshapeFunc(reshape_callback);
    glutMouseFunc(mouse_callback);
    glutMotionFunc(motion_callback);
    glutPassiveMotionFunc(passive_motion_callback);
    glutKeyboardFunc(keyboard_callback);
    glutKeyboardUpFunc(keyboard_up_callback);
    glutSpecialFunc(special_callback);
    glutTimerFunc(16, timer_callback, 0);
    
    glutMainLoop();
    return 0;
}
```

* **Função dos Callbacks Registrados:**
  * `display_callback`: Invocado toda vez que a tela precisa ser redesenhada.
  * `reshape_callback`: Chamado quando a janela é redimensionada ou maximizada, ajustando a área útil de projeção.
  * `mouse_callback`: Detecta cliques (pressionar e soltar) dos botões esquerdo, direito e da roda do mouse.
  * `motion_callback`: Registra o movimento do cursor do mouse enquanto algum botão está pressionado (arrastar).
  * `passive_motion_callback`: Rastreia o movimento livre do cursor sem botões pressionados (útil para detectar *hover* em botões).
  * `keyboard_callback` / `keyboard_up_callback`: Capturam teclas alfanuméricas padrão (pressionar e soltar).
  * `special_callback`: Captura teclas especiais do teclado (como as setas direcionais).
  * `glutTimerFunc(16, timer_callback, 0)`: Configura um temporizador para executar o callback a cada 16 milissegundos. Isso força uma taxa de atualização de aproximadamente **60 FPS** (1000ms / 16ms ≈ 62.5 FPS), garantindo animações fluidas.
  * `glutMainLoop()`: Entrega o controle de execução para o loop infinito do GLUT, que fica aguardando e despachando os eventos acima.

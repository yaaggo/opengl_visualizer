# Gerenciador de Estados e Viewport - `modules.cpp` / `modules.h`

Os arquivos [modules.h](file:///C:/Users/yagog/Documents/coding/comp-grafic/opengl_visualizer/src/modules.h) e [modules.cpp](file:///C:/Users/yagog/Documents/coding/comp-grafic/opengl_visualizer/src/modules.cpp) implementam a **arquitetura de roteamento** da aplicação. Eles funcionam como o "roteador" ou "despachante" do simulador, contendo a máquina de estados que redireciona os eventos globais de entrada e desenho para o módulo ativo.

Além disso, este arquivo gerencia o sistema de **Aspect Ratio (Proporção) Fixo**, que garante que o simulador não distorça as imagens ao ser executado em monitores de diferentes resoluções.

---

## 🔍 Análise Detalhada do Código

### 1. Máquina de Estados (Enumeração)
O cabeçalho `modules.h` expõe a enumeração `module_type` e variáveis de controle globais:

```cpp
typedef enum { MENU, BEZIER, VISUALIZER, PROJECTIONS, TEXTURE } module_type;
extern module_type current_module;
extern int viewport_width, viewport_height, viewport_x, viewport_y;
```

* **`current_module`:** Guarda o estado ativo atual (o módulo visível na tela). O padrão inicial é `MENU`.
* **Variáveis de Viewport (`viewport_...`):** Definem os limites e deslocamento da área útil de renderização gráfica calculados dinamicamente para preservar o aspecto de tela.

### 2. Roteamento de Callbacks
Em `modules.cpp`, todas as funções registradas como callbacks pelo `main.cpp` contêm estruturas condicionais `switch` baseadas em `current_module`. Por exemplo, a função de renderização `display_callback()`:

```cpp
void display_callback() {
    switch (current_module) {
        case MENU:
            menu_display();
            break;
        case BEZIER:
            bezier_display();
            break;
        case VISUALIZER:
            visualizer_display();
            break;
        case PROJECTIONS:
            projections_display();
            break;
        case TEXTURE:
            texture_display();
            break;
    }
}
```

* **Motivação:** Essa abordagem mantém os módulos de renderização completamente desacoplados. O loop do GLUT chama `display_callback()`, e este redireciona para a função apropriada do módulo ativo. O mesmo padrão de `switch` é utilizado para eventos de mouse (`mouse_callback`, `motion_callback`, `passive_motion_callback`), teclado (`keyboard_callback`, `keyboard_up_callback`, `special_callback`) e loops de física/tempo (`timer_callback`).

### 3. Tecla Universal de Escape (Retorno ao Menu)
No callback de teclado, existe um tratamento comum para a tecla `ESC` (código ASCII `27`):

```cpp
void keyboard_callback(unsigned char key, int x, int y) {
    (void)x;
    (void)y;
    if (key == 27) { // 27 = Tecla ESC
        if (current_module == MENU) {
            exit(0); // Se já está no menu, fecha o programa
        }
        current_module = MENU; // Caso contrário, volta ao menu
        glutPostRedisplay();
        return;
    }
    // ... despacha a tecla para o módulo correspondente ...
}
```

* **Motivação:** Oferecer uma experiência de usuário consistente e previsível. Apertar `ESC` em qualquer simulação retorna o usuário com segurança para o Menu Principal. Caso a tecla seja apertada no próprio Menu, o aplicativo é finalizado graciosamente.

### 4. Controle Dinâmico da Proporção da Tela (*Letterboxing*)
A função mais sofisticada do gerenciador de módulos é o `reshape_callback()`, que lida com o redimensionamento da janela do simulador:

```cpp
void reshape_callback(int width, int height) {
    float target_aspect_ratio = 16.0f / 9.0f;
    float window_aspect_ratio = (float)width / (float)height;

    if (height == 0) {
        height = 1;
    }

    if (window_aspect_ratio > target_aspect_ratio) {
        // A janela é mais larga que 16:9 -> Adiciona barras pretas nas laterais (esquerda e direita)
        viewport_height = height;
        viewport_width = (int)(height * target_aspect_ratio);
        viewport_x = (width - viewport_width) / 2;
    } else {
        // A janela é mais alta que 16:9 -> Adiciona barras pretas acima e abaixo
        viewport_width = width;
        viewport_height = (int)(width / target_aspect_ratio);
        viewport_y = (height - viewport_height) / 2;
    }

    glViewport(viewport_x, viewport_y, viewport_width, viewport_height);
    glutPostRedisplay();
}
```

* **Como Funciona:**
  1. Define que a proporção visual ideal é **16:9** (largura / altura = 1.7778).
  2. Calcula a proporção real da janela atual do usuário (`window_aspect_ratio`).
  3. Se a janela do usuário for mais larga do que 16:9, ajusta a altura da renderização para ocupar 100% da tela (`viewport_height = height`) e reduz proporcionalmente a largura (`viewport_width`), centralizando a imagem horizontalmente (`viewport_x = (width - viewport_width) / 2`).
  4. Se a janela do usuário for mais alta, faz o oposto: ocupa 100% da largura, reduz a altura e centraliza verticalmente (`viewport_y = (height - viewport_height) / 2`).
  5. Configura o espaço de desenho efetivo do OpenGL via `glViewport(x, y, w, h)`.
* **Motivação:** Impedir que o conteúdo visual pareça esticado ou achatado quando a janela é redimensionada ou quando o programa roda em monitores com proporções de tela diferentes (como 4:3, 16:10 ou ultra-widescreen 21:9).

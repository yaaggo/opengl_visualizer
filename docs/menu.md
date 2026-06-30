# Menu Principal - `menu.cpp` / `menu.h`

Os arquivos [menu.h](file:///C:/Users/yagog/Documents/coding/comp-grafic/opengl_visualizer/src/menu.h) e [menu.cpp](file:///C:/Users/yagog/Documents/coding/comp-grafic/opengl_visualizer/src/menu.cpp) controlam a tela inicial da aplicação. Este módulo apresenta uma interface bidimensional (2D) clássica contendo o título do simulador, quatro botões de seleção de módulos e créditos com o nome dos autores.

---

## 🔍 Análise Detalhada do Código

### 1. Configuração da Projeção 2D
Para desenhar elementos bidimensionais fixos (como botões e textos de interface) que não dependem de uma câmera em movimento, o OpenGL utiliza uma projeção ortográfica 2D. Isso é feito no início de `menu_display()`:

```cpp
void menu_display() {
    glViewport(viewport_x, viewport_y, viewport_width, viewport_height);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 800.0, 0.0, 450.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
```

* **`glDisable(GL_DEPTH_TEST)` e `glDisable(GL_LIGHTING)`**: Desativa o teste de profundidade e a iluminação, pois em uma interface de usuário 2D chapada queremos apenas pintar cores planas, sem preocupação com sombras ou faces ocultas.
* **`gluOrtho2D(0.0, 800.0, 0.0, 450.0)`**: Mapeia o espaço de coordenadas da janela em uma grade fixa virtual de **800 unidades de largura por 450 unidades de altura** (proporção exata de 16:9). A origem `(0, 0)` fica no canto inferior esquerdo e o ponto `(800, 450)` no canto superior direito.
* **Motivação:** Ao fixar essa resolução virtual interna, os botões mantêm suas posições e tamanhos relativos perfeitos em qualquer resolução física de tela, facilitando a programação do layout.

### 2. Renderização de Fontes Vetoriais Centradas (Stroke Fonts)
Ao contrário do texto em bitmap comum (`draw_text` do `utils.cpp`), o menu utiliza fontes vetoriais baseadas em linhas (*Stroke Fonts*), que podem ter sua escala e largura alteradas. O menu define uma função auxiliar para desenhar o texto centrado:

```cpp
static void draw_stroke_text_centered(float center_x, float center_y, float scale, float line_width, const std::string& text) {
    glPushMatrix();
    glLineWidth(line_width);
    float text_w = (float)glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char*)text.c_str()) * scale;
    float start_x = center_x - text_w / 2.0f;
    float start_y = center_y - (119.0f * scale / 2.0f);
    glTranslatef(start_x, start_y, 0.0f);
    glScalef(scale, scale, scale);
    for (char c : text) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
    }
    glPopMatrix();
    glLineWidth(1.0f);
}
```

* **`glutStrokeLength(GLUT_STROKE_ROMAN, ...)`**: Retorna a largura acumulada do texto em unidades de coordenadas vetoriais. Multiplicando essa largura pela escala do desenho, descobrimos a largura real na tela.
* **Cálculo de Centralização (`start_x`, `start_y`):** Subtrai metade da largura calculada do ponto central desejado (`center_x`). O mesmo é feito na vertical (a altura padrão de caractere desta fonte é aproximadamente 119 unidades).
* **`glScalef` / `glTranslatef`**: Aplica as transformações geométricas para mover a matriz de modelagem e posicionar e escalar o texto corretamente.

### 3. Feedback Visual de Interação (Hover e Click)
Os quatro botões de seleção de módulos são definidos no centro da tela. Para cada botão, o código faz testes geométricos para verificar se o mouse está sobre ele e se o botão esquerdo está pressionado:

```cpp
    float b_center_x = 400.0f;
    float b_width = 260.0f;
    float b_x1 = b_center_x - b_width / 2.0f;
    float b_x2 = b_center_x + b_width / 2.0f;
    float bezier_y1 = 290.0f;
    float bezier_y2 = 330.0f;

    // 1. Botão BEZIER
    bool hover_bezier = (menu_mouse_x >= b_x1 && menu_mouse_x <= b_x2 && menu_mouse_y >= bezier_y1 && menu_mouse_y <= bezier_y2);
    if (hover_bezier) {
        if (menu_click_state == GLUT_DOWN) {
            glColor3f(0.18f, 0.12f, 0.24f); // Roxo escuro se pressionado
        } else {
            glColor3f(0.353f, 0.243f, 0.459f); // Roxo médio ao passar o mouse
        }
    } else {
        glColor3f(0.282f, 0.192f, 0.369f); // Roxo padrão do botão
    }
    
    // Desenha o preenchimento do botão
    glBegin(GL_QUADS);
    glVertex2f(b_x1, bezier_y1);
    glVertex2f(b_x2, bezier_y1);
    glVertex2f(b_x2, bezier_y2);
    glVertex2f(b_x1, bezier_y2);
    glEnd();
```

* **Feedback Visual:** Três cores distintas são aplicadas conforme a interação do usuário. Além disso, as bordas externas do botão se tornam mais brilhantes quando há um *hover* (foco do mouse), utilizando a primitiva `GL_LINE_LOOP`.

### 4. Mapeamento de Coordenadas do Mouse
O GLUT retorna as coordenadas do mouse no formato clássico de janelas do SO: origem `(0, 0)` no canto **superior esquerdo** e valores finais em pixels físicos da tela. O menu precisa traduzir essas coordenadas para o sistema virtual do `gluOrtho2D` (onde o `Y` cresce de baixo para cima e a grade mede `800x450`):

```cpp
void menu_motion(int x, int y) {
    float local_x = x - viewport_x;
    float local_y = y - viewport_y;
    menu_mouse_x = (local_x / viewport_width) * 800.0f;
    menu_mouse_y = (1.0f - local_y / viewport_height) * 450.0f;
    glutPostRedisplay();
}
```

* **Tratamento:**
  1. Subtrai os deslocamentos (`viewport_x`, `viewport_y`) gerados pelo sistema de *letterboxing* para obter a coordenada local de renderização.
  2. Normaliza dividindo pelas dimensões ativas da janela (`viewport_width`, `viewport_height`).
  3. Multiplica pela resolução do espaço de projeção (`800` de largura, `450` de altura).
  4. Inverte o eixo Y (`1.0f - ...`) para alinhar com o padrão do OpenGL onde o zero é a base da tela.
* **Chaveamento de Módulos:** No callback `menu_mouse`, se o estado do clique for `GLUT_UP` (soltar o botão) dentro das coordenadas de um botão específico, a variável global `current_module` é alterada (ex: `current_module = BEZIER`), completando a transição de tela.

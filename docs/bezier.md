# Curva de Bézier - `bezier.cpp` / `bezier.h`

Os arquivos [bezier.h](file:///C:/Users/yagog/Documents/coding/comp-grafic/opengl_visualizer/src/bezier.h) e [bezier.cpp](file:///C:/Users/yagog/Documents/coding/comp-grafic/opengl_visualizer/src/bezier.cpp) controlam a demonstração interativa das curvas de Bézier cúbicas (grau 3). O usuário pode visualizar a curva interpolada e arrastar individualmente quatro pontos de controle na tela usando o mouse para modificar o traçado da curva em tempo real.

---

## 🔍 Análise Detalhada do Código

### 1. Definição dos Pontos de Controle
A curva de Bézier de grau 3 necessita de exatamente 4 pontos de controle tridimensionais (para fins geométricos, o eixo Z é mantido em zero):

```cpp
GLfloat control_points[4][3] = {
    {200.0f, 400.0f, 0.0f},
    {500.0f, 150.0f, 0.0f},
    {1100.0f, 150.0f, 0.0f},
    {1400.0f, 400.0f, 0.0f}
};
```

* **Estrutura:** Um array bidimensional contendo as coordenadas X, Y e Z de cada ponto. No início, os pontos formam um traçado simétrico na tela.
* **Projeção de Tela:** O módulo define uma tela de coordenadas virtuais de **1600 por 900 unidades**, mas inverte o eixo Y para que o ponto zero seja o topo da tela (`gluOrtho2D(0, 1600, 900, 0)`).

### 2. Avaliador de Curvas Nativo do OpenGL (`glMap1f`)
Em vez de implementar a equação polinomial de Bernstein manualmente via código para cada ponto intermediário da curva, o código tira proveito dos **Avaliadores 1D** nativos do OpenGL clássico:

```cpp
    glLineWidth(3.0f);
    glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 4, &control_points[0][0]);
    glEnable(GL_MAP1_VERTEX_3);
    glBegin(GL_LINE_STRIP);
    for (float t = 0.0f; t <= 1.0f; t += 0.01f) {
        glEvalCoord1f((GLfloat)t / 1.0);
    }
    glEnd();
    glDisable(GL_MAP1_VERTEX_3);
    glLineWidth(1.0f);
```

* **`glMap1f(...)`**: Define o mapa matemático da curva:
  * `GL_MAP1_VERTEX_3`: Informa que a saída gerada pelo avaliador serão vértices 3D (X, Y, Z).
  * `0.0`, `1.0`: O intervalo de variação do parâmetro $t$ (de 0% a 100%).
  * `3`: O passo de deslocamento na memória (*stride*) entre coordenadas consecutivas de pontos (3 floats).
  * `4`: O número de pontos de controle (grau da curva + 1).
  * `&control_points[0][0]`: O ponteiro para o início da matriz contendo as coordenadas dos pontos.
* **`glEnable(GL_MAP1_VERTEX_3)`**: Ativa a geração de vértices gerados pelo avaliador.
* **`glEvalCoord1f(t)`**: Avalia as equações da curva de Bézier para um determinado valor do parâmetro $t \in [0.0, 1.0]$ e envia o vértice correspondente ao pipeline do OpenGL. O loop desenha 100 pequenos segmentos de reta conectados (`GL_LINE_STRIP`) para aproximar visualmente a suavidade da curva.
* **Motivação:** Demonstrar o funcionamento dos pipelines de avaliação matemática acelerada por hardware do OpenGL clássico.

### 3. Seleção e Arraste dos Pontos de Controle
Para tornar a interface interativa, o programa rastreia a proximidade do clique do mouse em relação aos quatro pontos de controle.

Na função `bezier_mouse()`:
```cpp
void bezier_mouse(int button, int state, int x, int y) {
    Point2D mouse_pos = map_mouse_to_ortho(x, y);

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            for (int i = 0; i < 4; i++) {
                float dx = mouse_pos.x - control_points[i][0];
                float dy = mouse_pos.y - control_points[i][1];
                if (std::sqrt(dx*dx + dy*dy) <= SELECTION_RADIUS * 2.0f) {
                    selected_point = i; // Encontrou o ponto clicado
                    break;
                }
            }
        } else if (state == GLUT_UP) {
            selected_point = -1; // Solta o ponto
        }
    }
    glutPostRedisplay();
}
```

* **Detecção de Clique:** Quando o usuário clica com o botão esquerdo, a distância euclidiana entre a posição do mouse e cada ponto de controle é testada contra um raio de seleção (`SELECTION_RADIUS * 2.0f`). Se o mouse estiver próximo o suficiente, o índice correspondente é salvo em `selected_point`.

Na função `bezier_motion()`:
```cpp
void bezier_motion(int x, int y) {
    if (selected_point != -1) {
        Point2D mouse_pos = map_mouse_to_ortho(x, y);

        // Limita o movimento do ponto dentro do contêiner gráfico 2D superior
        if (mouse_pos.x < 50.0f + SELECTION_RADIUS)   mouse_pos.x = 50.0f + SELECTION_RADIUS;
        if (mouse_pos.x > 1550.0f - SELECTION_RADIUS) mouse_pos.x = 1550.0f - SELECTION_RADIUS;
        if (mouse_pos.y < 50.0f + SELECTION_RADIUS)   mouse_pos.y = 50.0f + SELECTION_RADIUS;
        if (mouse_pos.y > 530.0f - SELECTION_RADIUS)  mouse_pos.y = 530.0f - SELECTION_RADIUS;

        control_points[selected_point][0] = mouse_pos.x;
        control_points[selected_point][1] = mouse_pos.y;
        glutPostRedisplay();
    }
}
```

* **Limitação de Movimento (Clamp):** Impede que o usuário arraste um ponto para fora do contêiner desenhado na tela, preservando a harmonia da interface e o layout 2D.
* **`glutPostRedisplay()`**: Solicita que o GLUT redesenhe a tela imediatamente para refletir as novas posições no próximo quadro.

### 4. Desenho do Polígono de Controle e Polígono Auxiliar
Para facilitar a visualização do efeito de Bézier, uma linha tracejada é desenhada conectando os pontos de controle na ordem correta:

```cpp
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0x00FF);
    glColor3f(0.5f, 0.5f, 0.6f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < 4; i++) {
        glVertex2f(control_points[i][0], control_points[i][1]);
    }
    glEnd();
    glDisable(GL_LINE_STIPPLE);
```

* **`glLineStipple(...)`**: Define um padrão de tracejado de linha (máscara de bits `0x00FF`). É desativada logo em seguida para não afetar outros traçados do sistema.
* **Motivação:** Demonstrar o invólucro convexo (*convex hull*) da curva. As curvas de Bézier ficam sempre contidas dentro do polígono formado por seus pontos de controle.

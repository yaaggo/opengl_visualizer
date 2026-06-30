# Projeções e Volume de Recorte - `projections.cpp` / `projections.h`

Os arquivos [projections.h](file:///C:/Users/yagog/Documents/coding/comp-grafic/opengl_visualizer/src/projections.h) e [projections.cpp](file:///C:/Users/yagog/Documents/coding/comp-grafic/opengl_visualizer/src/projections.cpp) implementam um dos experimentos mais educativos do simulador. O objetivo deste módulo é ilustrar a diferença mecânica e geométrica entre a **Projeção Perspectiva** (`gluPerspective`) e a **Projeção Ortográfica** (`glOrtho`).

O módulo realiza isso por meio de uma tela dividida (*Split-Screen*):
1. **Lado Esquerdo:** Exibe a cena 3D final gerada a partir da câmera configurada.
2. **Lado Direito:** Exibe uma visão em 3D de órbita externa que mostra a própria câmera (um cubo branco), o ponto focal (uma esfera vermelha) e o **volume de corte (frustum)** em linhas amarelas com os planos *Near* e *Far* desenhados em cores translúcidas.

---

## 🔍 Análise Detalhada do Código

### 1. Renderização em Múltiplos Viewports (Split-Screen)
Para desenhar duas visões independentes na mesma janela física, o código manipula a função `glViewport` dinamicamente durante a fase de renderização da tela:

```cpp
    // 1. Calcula a posição física em pixels do viewport esquerdo
    float rx_left = 50.0f / 1600.0f;
    float ry_left = 350.0f / 900.0f;
    float rw_left = (650.0f - 50.0f) / 1600.0f;
    float rh_left = (850.0f - 350.0f) / 900.0f;
    int px_left = viewport_x + (int)(rx_left * viewport_width);
    int py_left = viewport_y + (int)(ry_left * viewport_height);
    int pw_left = (int)(rw_left * viewport_width);
    int ph_left = (int)(rh_left * viewport_height);

    // Define a viewport ativa do lado esquerdo e limpa apenas esta área da tela
    glViewport(px_left, py_left, pw_left, ph_left);
    glEnable(GL_SCISSOR_TEST);
    glScissor(px_left, py_left, pw_left, ph_left);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // ... Desenha a cena projetada (Toro e eixos coordenados) ...
    glDisable(GL_SCISSOR_TEST);
```

* **`glScissor(...)` e `GL_SCISSOR_TEST`**: O teste de tesoura (*Scissor Test*) restringe a área onde a limpeza de buffers (`glClear`) pode atuar. Sem isso, chamar `glClear` em uma viewport limparia a tela inteira, apagando o conteúdo desenhado nas outras seções.
* **Mapeamento de Coordenadas:** O código calcula a posição física da viewport em pixels com base na resolução de tela atual (`viewport_width` e `viewport_height`) para garantir que os painéis fiquem alinhados com o layout desenhado em 2D.

### 2. Cálculo dos Vetores de Base da Câmera
Para desenhar o volume de recorte tridimensional com a orientação e rotação correta da câmera no espaço, precisamos calcular a orientação espacial da câmera (seus vetores unitários de direção: frente, direita e cima). Isso é feito na função `get_camera_basis()` utilizando álgebra vetorial:

```cpp
static void get_camera_basis(Vec3* forward, Vec3* right, Vec3* up) {
    Vec3 eye = make_vec3(cam_eyeX, cam_eyeY, cam_eyeZ);
    Vec3 center = make_vec3(cam_cX, cam_cY, cam_cZ);
    Vec3 world_up = normalize_vec3(make_vec3(cam_upX, cam_upY, cam_upZ), make_vec3(0.0f, 1.0f, 0.0f));

    // 1. Vetor de direção "frente" (Olhar) = normaliza(centro - olho)
    *forward = normalize_vec3(sub_vec3(center, eye), make_vec3(0.0f, 0.0f, -1.0f));
    
    // 2. Vetor lateral "direita" = produto vetorial (frente x mundo_cima)
    *right = normalize_vec3(cross_vec3(*forward, world_up), make_vec3(1.0f, 0.0f, 0.0f));
    
    // 3. Vetor real "cima" da câmera = produto vetorial (direita x frente)
    *up = normalize_vec3(cross_vec3(*right, *forward), make_vec3(0.0f, 1.0f, 0.0f));
}
```

* **Fundamentação Matemática:**
  * O vetor **forward** aponta na direção de visão da câmera (do olho para o ponto de foco).
  * O vetor **right** é perpendicular ao plano formado pelo vetor de visão e pelo vetor de topo global (geralmente $+Y$). O produto vetorial $\vec{f} \times \vec{up}_{world}$ fornece esse vetor ortogonal.
  * O vetor **up** local da câmera é recalculado fazendo $\vec{r} \times \vec{f}$ para garantir ortogonalidade perfeita, corrigindo distorções caso o vetor `cam_up` fornecido pelo usuário não seja perpendicular ao vetor de visão.

### 3. Matriz de Rotação Customizada do Frustum
Uma vez obtidos os vetores de base da câmera, é necessário rotacionar o volume geométrico do frustum para alinhar-se perfeitamente com a direção em que a câmera está olhando. Em vez de chamar as funções `glRotatef` para cada ângulo, o código constrói uma **Matriz de Rotação Manual** de mudança de base:

```cpp
    float rot_matrix[16] = {
         sX,  sY,  sZ, 0.0f, // Vetor lateral (Right)
         uX,  uY,  uZ, 0.0f, // Vetor vertical local (Up)
        -fX, -fY, -fZ, 0.0f, // Vetor de profundidade invertido (-Forward)
        0.0f,0.0f,0.0f, 1.0f 
    };
    glPushMatrix();
    glTranslatef(cam_eyeX, cam_eyeY, cam_eyeZ); // Move para a posição do olho da câmera
    glMultMatrixf(rot_matrix);                 // Aplica a rotação de orientação
    // ... Desenha os planos Near/Far e as retas convergentes ...
    glPopMatrix();
```

* **Matriz de Mudança de Base:** Essa matriz $4 \times 4$ colapsa as direções locais da câmera diretamente no pipeline do OpenGL. O uso do vetor $-Forward$ no terceiro vetor coluna se deve ao fato de que, no espaço da câmera do OpenGL, a direção para onde a lente aponta é convencionada como o eixo $-Z$ negativo.

### 4. Desenho de Sólidos Translúcidos (Transparência)
Os planos de recorte próximo (*Near plane*) e distante (*Far plane*) são renderizados como faces coloridas semitransparentes. Para que possamos ver os objetos que estão atrás e dentro do frustum, habilitamos a mistura de canais alfa (Blending):

```cpp
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE); // Desativa escrita no Z-buffer temporariamente

        glColor4f(1.0f, 0.12f, 0.12f, 0.26f); // Vermelho com 26% de opacidade (Near)
        glBegin(GL_QUADS);
        glVertex3f(-nearW/2, -nearH/2, -p_zNear);
        glVertex3f( nearW/2, -nearH/2, -p_zNear);
        // ... vértices ...
        glEnd();

        glDepthMask(GL_TRUE); // Reativa escrita no Z-buffer
        glDisable(GL_BLEND);
```

* **`glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)`**: Define a fórmula clássica de interpolação linear de cores para transparência: $Cor_{final} = Cor_{nova} \cdot Alfa_{novo} + Cor_{existente} \cdot (1 - Alfa_{novo})$.
* **`glDepthMask(GL_FALSE)`**: Impede que o desenho dos polígonos translúcidos grave dados de profundidade no *Z-Buffer*. Sem isso, as faces translúcidas impediriam o desenho de qualquer objeto opaco localizado atrás delas (mesmo sendo transparentes), gerando artefatos visuais de recorte incorreto.

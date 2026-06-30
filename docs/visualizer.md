# Visualizador de Primitivas 2D/3D - `visualizer.cpp` / `visualizer.h`

Os arquivos [visualizer.h](file:///C:/Users/yagog/Documents/coding/comp-grafic/opengl_visualizer/src/visualizer.h) e [visualizer.cpp](file:///C:/Users/yagog/Documents/coding/comp-grafic/opengl_visualizer/src/visualizer.cpp) implementam um dos módulos mais complexos do simulador. O visualizador permite alternar entre o desenho bidimensional (2D) de primitivas geométricas básicas (pontos, linhas, retângulos, círculos) e a renderização tridimensional (3D) de sólidos geométricos (cubos, esferas, cones, toros e o famoso Bule de Utah).

O módulo expõe o controle de câmera, iluminação local, propriedades de materiais e mapeamento de textura em tempo real.

---

## 🔍 Análise Detalhada do Código

### 1. Sistema de Câmera e Navegação
O arquivo suporta dois modos de câmera dependendo do estado (`current_mode_3d`):

#### A. Navegação 2D (Pan & Zoom)
No modo 2D, a projeção é ortográfica e a câmera suporta movimentação lateral (*panning*) e zoom:
* **Zoom:** Controlado girando a roda do mouse (ou botões do mouse 3 e 4). Ele multiplica os limites da chamada `glOrtho`:
  ```cpp
  glOrtho(-460.0 * camera_2d_zoom, 460.0 * camera_2d_zoom, -250.0 * camera_2d_zoom, 250.0 * camera_2d_zoom, -1000.0, 1000.0);
  ```
* **Arrastar (Panning):** Segurar o botão direito do mouse e arrastar atualiza `camera_2d_pos_x` e `camera_2d_pos_y`, transladando a cena 2D antes do desenho:
  ```cpp
  glTranslatef(camera_2d_pos_x, camera_2d_pos_y, 0.0f);
  ```

#### B. Navegação 3D (FPS / Orbit)
No modo 3D, a projeção é perspectiva (`gluPerspective`) e o usuário tem controle livre sobre a câmera:
* **Orbitar:** Arrastar com o botão direito do mouse rotaciona a visualização nos eixos X (pitch) e Y (yaw):
  ```cpp
  glRotatef(camera_rot_x, 1.0f, 0.0f, 0.0f);
  glRotatef(camera_rot_y, 0.0f, 1.0f, 0.0f);
  ```
* **Movimentação Teclado (WASD):** Pressionar as teclas `W`, `A`, `S` ou `D` desloca a câmera no espaço tridimensional. O estado das teclas é mantido em um array booleano `keys_state[256]` e atualizado a cada quadro na função `visualizer_update()`:
  ```cpp
  void visualizer_update() {
      if (current_mode_3d) {
          if (keys_state['w']) camera_pos_z += 0.05f; // Move para frente
          if (keys_state['s']) camera_pos_z -= 0.05f; // Move para trás
          if (keys_state['a']) camera_pos_x += 0.05f; // Move para esquerda
          if (keys_state['d']) camera_pos_x -= 0.05f; // Move para direita
          // ... limites e travas ...
      }
  }
  ```

### 2. Iluminação Local (Modelo de Iluminação de Phong)
Quando a iluminação é habilitada na interface ("Iluminacao + Solido"), o OpenGL habilita seu mecanismo interno de cálculo de luz:

```cpp
    if (is_lighting_enabled) {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

        GLfloat light_pos[4] = { light_3d_x, light_3d_y, light_3d_z, 1.0f };
        GLfloat light_diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat light_ambient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };

        glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
        glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    }
```

* **`GL_LIGHTING` e `GL_LIGHT0`:** Liga o pipeline de iluminação e a fonte de luz número zero.
* **`GL_COLOR_MATERIAL`**: Diz ao OpenGL para utilizar a cor atualmente selecionada via `glColor3f` como componente refletiva padrão do objeto (ambiente e difusa). Isso poupa a escrita de repetidas chamadas de `glMaterialfv` para a cor básica do sólido.
* **`light_pos` (Coordenadas Homogêneas):** O quarto componente configurado em `1.0f` define que esta é uma **luz pontual** posicional (como uma lâmpada localizada nas coordenadas X, Y, Z), em vez de uma luz direcional (como o Sol, que teria valor `0.0f`).

### 3. Propriedades de Material e Efeito de Especularidade
Para que os sólidos 3D pareçam realistas e apresentem pontos de brilho metálico ou plástico (*specular highlights*), configuramos os parâmetros do material de superfície:

```cpp
    GLfloat mat_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat mat_diffuse[] = { 0.851f, 0.753f, 0.949f, 1.0f };
    GLfloat mat_specular[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat mat_shininess[] = { 50.0f };
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
```

* **`GL_SPECULAR`**: Define a cor do reflexo especular (no caso, branco brilhante `0.8f, 0.8f, 0.8f`).
* **`GL_SHININESS`**: Controla o foco do brilho (expoente de Phong). Quanto maior o valor (máximo `128.0f`), menor e mais concentrado é o ponto brilhante na superfície do objeto, fazendo com que ele pareça mais polido ou metálico.

### 4. Geração Automática de Coordenadas de Textura (`TexGen`)
O mapeamento de texturas exige que cada vértice do objeto possua coordenadas UV no intervalo $[0.0, 1.0]$. O Cubo e o Bule desenhados possuem suporte nativo ou coordenadas embutidas, mas outras primitivas do GLUT (como `SolidSphere`, `SolidCone` e `SolidTorus`) não fornecem coordenadas de textura automaticamente.

Para contornar isso, o código usa o recurso de **Geração Automática de Coordenadas de Textura** do OpenGL:

```cpp
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    GLfloat s_plane[] = { 1.0f / obj_3d_size, 0.0f, 0.0f, 0.0f };
    GLfloat t_plane[] = { 0.0f, 1.0f / obj_3d_size, 0.0f, 0.0f };
    glTexGenfv(GL_S, GL_OBJECT_PLANE, s_plane);
    glTexGenfv(GL_T, GL_OBJECT_PLANE, t_plane);
```

* **`GL_OBJECT_LINEAR`**: O OpenGL calcula as coordenadas de textura com base nas coordenadas de vértice do objeto original (antes de qualquer rotação ou câmera).
* **Plano de Projeção (`s_plane` / `t_plane`):** Define a escala de aplicação da textura no espaço. O valor é dividido pelo tamanho do objeto (`1.0f / obj_3d_size`) para garantir que a textura estique perfeitamente acompanhando as mudanças dimensionais do objeto de forma linear.

### 5. Renderização do Cubo Texturizado Manualmente
Para o Cubo, o mapeamento é feito vértice a vértice para garantir que a imagem não distorça:

```cpp
void draw_textured_cube(float size) {
    float h = size / 2.0f;
    glBegin(GL_QUADS);
    // Face Frontal
    glNormal3f(0.0f, 0.0f, 1.0f); // Vetor normal apontando para frente (eixo +Z)
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-h, -h,  h);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( h, -h,  h);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( h,  h,  h);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-h,  h,  h);
    // ... repete o processo definindo normais e coordenadas UV para as outras 5 faces ...
    glEnd();
}
```

* **`glNormal3f(x, y, z)`:** Define o vetor normal de iluminação para a face. Essencial para que os cálculos de sombreamento de Phong saibam para onde a superfície aponta e como a luz rebate nela.
* **`glTexCoord2f(u, v)`**: Mapeia o pixel correspondente da imagem (coordenada bidimensional entre 0 e 1) ao vértice tridimensional associado imediatamente depois (`glVertex3f`).

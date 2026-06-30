# Iluminação e Mapeamento de Texturas - `texture.cpp` / `texture.h`

Os arquivos [texture.h](file:///C:/Users/yagog/Documents/coding/comp-grafic/opengl_visualizer/src/texture.h) e [texture.cpp](file:///C:/Users/yagog/Documents/coding/comp-grafic/opengl_visualizer/src/texture.cpp) gerenciam a simulação avançada de mapeamento de texturas e iluminação sobre um sólido 3D rotatório (cubo). Este módulo serve para ilustrar a aplicação de materiais em superfícies e como os parâmetros da fonte de luz afetam o visual de uma cena.

---

## 🔍 Análise Detalhada do Código

### 1. Textura de Contingência (Checkerboard Procedural)
Se os arquivos de imagem das texturas (localizados na pasta `/assets`) falharem em carregar ou estiverem ausentes por algum motivo, o sistema evita falhar com uma tela em branco e gera programaticamente na memória uma textura de padrão xadrez (Checkerboard) utilizando as cores de destaque do projeto:

```cpp
static GLuint create_checkerboard_texture() {
    const int width = 64;
    const int height = 64;
    GLubyte image[width][height][4];
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            // Aplica um padrão xadrez alternando a cor a cada 8 pixels
            bool is_beige = (((i & 0x8) == 0) ^ ((j & 0x8) == 0));
            image[i][j][0] = is_beige ? 234 : 55;  // R (Bege ou Roxo)
            image[i][j][1] = is_beige ? 205 : 37;  // G
            image[i][j][2] = is_beige ? 194 : 73;  // B
            image[i][j][3] = 255;                  // Alfa (Opaco)
        }
    }
    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    // ... filtros lineares ...
    return tex_id;
}
```

* **Lógica Binária Xadrez:** A operação `((i & 0x8) == 0) ^ ((j & 0x8) == 0)` utiliza aritmética de bits para alternar entre verdadeiro e falso a cada 8 linhas e 8 colunas, gerando o clássico tabuleiro de xadrez.
* **Cores Customizadas:** Bege (`RGB: 234, 205, 194`) e Roxo (`RGB: 55, 37, 73`) preservam a paleta de cores moderna da aplicação mesmo na ausência de imagens de disco.

### 2. Componentes da Fonte de Luz e Intensidade
O módulo expõe sliders para controle fino dos componentes de luz e material baseados no modelo de Phong:

```cpp
    GLfloat light_pos[4] = { light_pos_x, light_pos_y, light_pos_z, 1.0f };
    GLfloat light_ambient[4] = { light_ambient_val * light_intensity, light_ambient_val * light_intensity, light_ambient_val * light_intensity, 1.0f };
    GLfloat light_diffuse[4] = { light_diffuse_val * light_intensity, light_diffuse_val * light_intensity, light_diffuse_val * light_intensity, 1.0f };
    GLfloat light_specular[4] = { light_intensity, light_intensity, light_intensity, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
```

* **`light_intensity`**: Um multiplicador escalar simples que aumenta ou diminui o brilho global da luz multiplicando os vetores ambiente, difuso e especular.
* **`light_ambient`**: Luz indireta constante que incide sobre todas as superfícies de forma uniforme, simulando a luz que rebate nas paredes. Impede que as partes que não recebem luz direta fiquem totalmente pretas.
* **`light_diffuse`**: Luz direta que incide sobre o objeto e é espalhada em todas as direções de maneira uniforme. A intensidade do brilho depende do ângulo de incidência do raio de luz em relação à normal da face (cálculo de cosseno da lei de Lambert). É a componente mais importante para revelar o volume tridimensional e os detalhes das texturas.

### 3. Integração de Textura com Iluminação
Quando uma textura 2D está habilitada (`glEnable(GL_TEXTURE_2D)`) junto com a iluminação, o OpenGL realiza a multiplicação dos pixels da imagem (*texels*) pela cor resultante do cálculo de iluminação de cada vértice:

```cpp
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_ids[selected_texture_idx]);
    draw_textured_cube(2.0f);
    glDisable(GL_TEXTURE_2D);
```

* **`glBindTexture(...)`**: Vincula a textura selecionada no painel esquerdo (madeira, metal, sci-fi ou pedra) para ser utilizada no desenho subsequente do cubo.
* **Fórmula de Mistura Nativa (GL_MODULATE):** O modo padrão do OpenGL multiplica a iluminação pelo pixel da imagem: $Cor_{final} = Cor_{textura} \times Cor_{iluminacao}$. Assim, as partes do cubo voltadas para a lâmpada mostram a imagem clara, enquanto as faces opostas sombreadas mostram a textura escurecida, dando o efeito de realismo 3D.

### 4. Animação de Rotação Automática
A animação do cubo é controlada no loop do temporizador (`timer_callback` do `modules.cpp` que invoca `texture_update`):

```cpp
void texture_update() {
    if (auto_rotate) {
        cube_rot_y += 0.5f;
        if (cube_rot_y >= 360.0f) {
            cube_rot_y -= 360.0f;
        }
    }
}
```

* **Interação:** Pressionar a **barra de espaço** inverte o estado da flag booleana `auto_rotate`. Quando ativa, a rotação em torno do eixo Y (`cube_rot_y`) é incrementada continuamente a cada quadro, simulando um expositor giratório.

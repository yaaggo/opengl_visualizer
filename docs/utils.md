# Funções Utilitárias - `utils.cpp` / `utils.h`

Os arquivos [utils.h](file:///C:/Users/yagog/Documents/coding/comp-grafic/opengl_visualizer/src/utils.h) e [utils.cpp](file:///C:/Users/yagog/Documents/coding/comp-grafic/opengl_visualizer/src/utils.cpp) reúnem rotinas matemáticas e gráficas utilitárias que são reutilizadas por múltiplos módulos do simulador. Isso evita a duplicação de lógica essencial, como renderização de strings de texto, desenhos geométricos simples e carregamento de texturas a partir do disco.

---

## 🔍 Análise Detalhada do Código

### 1. Detecção de Colisão 2D (AABB)
A função `collision` implementa um teste simples de colisão baseado em caixas delimitadoras alinhadas aos eixos (*Axis-Aligned Bounding Boxes*):

```cpp
bool collision(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}
```

* **Princípio Matemático:** Roda quatro testes rápidos de desigualdade para verificar se duas regiões retangulares se sobrepõem nos eixos X e Y. Se todas as condições forem verdadeiras, as caixas se intersectam.
* **Motivação:** Embora o projeto seja focado em visualizações matemáticas, este utilitário básico está disponível para o mapeamento de limites de colisão e interações de interface bidimensionais.

### 2. Renderização de Textos com Fontes Bitmapeadas
Como o OpenGL puro desenha apenas primitivas simples (pontos, linhas e triângulos), a escrita de texto requer o uso da biblioteca utilitária do GLUT. A função `draw_text` encapsula este processo:

```cpp
void draw_text(float x, float y, void *font, const std::string& text) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(font, c);
    }
}
```

* **`glRasterPos2f(x, y)`:** Define a posição raster inicial da tela (onde o desenho de pixels do caractere deve começar).
* **`glutBitmapCharacter(font, c)`:** Renderiza um caractere individual usando uma das fontes padrão do GLUT (como `GLUT_BITMAP_HELVETICA_18` ou `GLUT_BITMAP_HELVETICA_12`). O próprio GLUT gerencia o avanço horizontal da posição raster a cada caractere desenhado.
* **Motivação:** Fornecer um método conveniente para imprimir rótulos, legendas explicativas e blocos de código na tela do simulador de forma rápida.

### 3. Algoritmo para Desenho de Círculos Preenchidos
Diferente de APIs de desenho 2D convencionais, o OpenGL não possui uma função do tipo `glDrawCircle`. Para renderizar um círculo preenchido, é necessário aproximar sua forma aproximando-a com um polígono regular de múltiplos lados. A função `draw_circle` faz isso utilizando a primitiva `GL_TRIANGLE_FAN`:

```cpp
void draw_circle(float cx, float cy, float radius, int segments) {
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy); // 1. O centro do círculo é o ponto inicial comum dos triângulos
        for (int i = 0; i <= segments; i++) {
            float theta = 2.0f * 3.1415926f * (float)i / (float)segments;
            float x = radius * cosf(theta);
            float y = radius * sinf(theta);
            glVertex2f(cx + x, cy + y); // 2. Calcula cada vértice da circunferência
        }
    glEnd();
}
```

* **Princípio Geométrico:** O `GL_TRIANGLE_FAN` cria um leque de triângulos, onde todos compartilham o primeiro vértice enviado (o centro do círculo `cx, cy`). Conforme avançamos com o ângulo $\theta$ de $0$ a $2\pi$ radianos divididos pelo número de `segments`, calculamos os pontos na borda usando coordenadas polares $x = r \cdot \cos(\theta)$ e $y = r \cdot \sin(\theta)$.
* **Motivação:** Desenhar círculos preenchidos de forma eficiente na interface gráfica e nas primitivas interativas 2D.

### 4. Carregamento de Texturas em OpenGL
A função `load_texture` lê uma imagem do disco (em formatos como PNG ou JPG) e a envia para a memória da GPU como uma textura OpenGL:

```cpp
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

GLuint load_texture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    // ... lógica de fallback caso o arquivo falhe em carregar ...
```

* **Tratamento de Caminho Alternativo (Fallback):**
  Se a imagem falhar ao carregar no caminho especificado (por exemplo, `assets/crate.png`), a função tenta um caminho alternativo adicionando `../` no início. Isso é muito importante pois, dependendo de onde o executável é disparado (da raiz do projeto ou de dentro da pasta `/bin`), a pasta relativa `assets/` mudaria de local:
  ```cpp
  if (!data) {
      std::string fallbackPath = "../" + std::string(path);
      data = stbi_load(fallbackPath.c_str(), &width, &height, &nrComponents, 0);
  }
  ```

* **Geração dos Parâmetros da Textura:**
  Se os dados de pixel forem obtidos com sucesso, a textura é gerada na GPU:
  ```cpp
  if (data) {
      GLenum format = GL_RGB;
      if (nrComponents == 1)      format = GL_RED;
      else if (nrComponents == 3) format = GL_RGB;
      else if (nrComponents == 4) format = GL_RGBA;

      glBindTexture(GL_TEXTURE_2D, textureID);
      glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
      
      // Filtros e Envelopamento de Texturas (Wrapping/Filtering)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      stbi_image_free(data);
  }
  ```
  * **`glGenTextures(1, &textureID)`:** Gera um identificador numérico único para a textura.
  * **`glBindTexture(GL_TEXTURE_2D, textureID)`:** Vincula o identificador ao alvo de textura 2D ativo.
  * **`glTexImage2D(...)`:** Transfere a matriz unidimensional de bytes lida pelo `stb_image` para a GPU, alocando a memória da textura com a largura, altura e formato corretos (RGB ou RGBA).
  * **`GL_REPEAT`**: Configura o comportamento de repetição. Se as coordenadas de textura passarem de `1.0f`, a imagem se repetirá ciclicamente.
  * **`GL_LINEAR`**: Habilita interpolação bilinear na interpolação de pixel. Garante que a textura pareça suave mesmo quando aproximada ou ampliada (evitando o visual pixelado).
  * **`stbi_image_free(data)`:** Libera a memória RAM do computador que continha a imagem original lida, pois ela já foi copiada e está armazenada na memória dedicada da placa de vídeo (VRAM).

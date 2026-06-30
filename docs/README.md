# Simulador OpenGL - Documentação do Sistema

Bem-vindo à central de documentação do **Simulador OpenGL**. Este projeto é um aplicativo interativo desenvolvido em C++ utilizando a biblioteca **FreeGLUT** e a API **OpenGL**. O objetivo do simulador é demonstrar, de forma prática e visual, conceitos fundamentais de Computação Gráfica, como curvas, transformações 2D/3D, projeções geométricas (perspectiva e ortográfica), iluminação, materiais e mapeamento de texturas.

Esta documentação foi estruturada para apresentar o código-fonte de maneira incremental e lógica, facilitando o entendimento de sua arquitetura e de cada um dos módulos.

---

## 🛠️ Como Compilar e Executar o Projeto

O projeto utiliza um `Makefile` configurado para compilação com o compilador `g++` no Windows.

### Requisitos
* Compilador **GCC/G++** (MinGW ou similar).
* Biblioteca **FreeGLUT** instalada e configurada nas variáveis do compilador (`freeglut`, `opengl32`, `glu32`).

### Comandos de Compilação
No terminal, na raiz do projeto:

* **Compilar o projeto:**
  ```bash
  make
  ```
  Isso criará os diretórios `obj/` (para arquivos de objeto compilados) e `bin/` (contendo o executável final `visualizer.exe`).

* **Compilar e Executar automaticamente:**
  ```bash
  make run
  ```

* **Limpar os arquivos gerados (limpeza de build):**
  ```bash
  make clean
  ```

---

## 🗺️ Sumário e Ordem de Leitura Recomendada

Para compreender o código em uma ordem que faça sentido conceitual e estrutural, recomendamos seguir os links abaixo na sequência indicada:

### 1. Núcleo da Aplicação (Infraestrutura)
Esta seção cobre o ponto de entrada do programa, o controle do ciclo de vida da janela e o sistema central que gerencia os diferentes módulos (telas) do simulador.

* **[Ponto de Entrada e Configurações (main.cpp)](main.md):** Analisa a inicialização da janela gráfica com FreeGLUT, as configurações iniciais de tela cheia e o registro das funções de callback.
* **[Gerenciador de Módulos (modules.h / modules.cpp)](modules.md):** Explica a máquina de estados que chaveia os callbacks de entrada (mouse, teclado) e renderização para o módulo ativo (Menu, Bezier, Visualizer, Projections, Texture). Também detalha o ajuste dinâmico de resolução (*letterboxing* 16:9).
* **[Utilitários de Apoio (utils.h / utils.cpp)](utils.md):** Examina as funções auxiliares comuns de renderização de texto na tela, detecção de colisão AABB (caixa delimitadora), desenho de círculos e carregamento de arquivos de imagem (.png/.jpg) para texturas.
* **[Carregamento de Imagens (stb_image.h)](stb_image.md):** Apresenta brevemente a biblioteca externa *single-header* utilizada para carregar dados de pixel de arquivos de disco.
* **[Arquitetura do Sistema e OpenGL por Trás](architecture_opengl.md):** Um guia técnico completo detalhando a composição dos módulos, o fluxo de eventos de callback, e como os principais recursos do OpenGL (Matrizes, Luz, Textura, Blending e Curvas) operam sob o capô.

### 2. Módulos Interativos (Aplicações Práticas)
Cada um destes arquivos implementa uma tela interativa independente com conceitos específicos de Computação Gráfica.

* **[Menu Principal (menu.h / menu.cpp)](menu.md):** A tela inicial do simulador, que desenha botões em 2D usando projeção ortogonal com suporte a feedback visual de mouse (*hover* e clique) para alternar entre as demonstrações.
* **[Curva de Bézier (bezier.h / bezier.cpp)](bezier.md):** Uma demonstração interativa de curvas de Bézier cúbicas (grau 3) utilizando avaliadores nativos do OpenGL (`glMap1f`), permitindo arrastar os 4 pontos de controle na tela com o mouse em tempo real.
* **[Visualizador de Formas 2D/3D (visualizer.h / visualizer.cpp)](visualizer.md):** Um ambiente completo para visualizar primitivas 2D (ponto, linha, retângulo, círculo) e sólidos 3D (cubo, esfera, cone, toro, bule) com controle de parâmetros dimensionais, iluminação local, sombreamento e translação/rotação controlada pelo teclado e mouse.
* **[Projeções e Câmeras (projections.h / projections.cpp)](projections.md):** Uma das partes mais didáticas do projeto. Uma tela dividida em duas visões: a esquerda mostra o resultado final projetado (Perspectiva ou Ortográfica), e a direita mostra uma visão externa em 3D da própria câmera física, exibindo o frustum de recorte e os planos *Near* e *Far* translúcidos. Permite ajustar 13 variáveis em tempo real.
* **[Texturas e Iluminação Global (texture.h / texture.cpp)](texture.md):** Demonstração focada no mapeamento de texturas (madeira, metal, sci-fi, pedra) em um cubo rotatório, com ajuste fino dos parâmetros da fonte de luz (posição, intensidade, componentes ambiente e difusa) através de sliders na tela.

---

## 🎨 Design do Sistema e Interface Gráfica
Todas as telas do simulador compartilham uma identidade visual coerente:
* **Paleta de Cores:** Fundo roxo escuro, contêineres e painéis em tons mais profundos de roxo e bordas/textos em cor de areia/bege, proporcionando alta legibilidade e um visual moderno.
* **Painéis de Código na Tela:** As telas de Bézier, Visualizador, Projeções e Texturas possuem painéis na parte inferior que exibem **dinamicamente as linhas exatas de código OpenGL** que correspondem ao estado atual da cena. Se você rotacionar a câmera, o valor numérico em `gluLookAt` ou `glRotatef` é atualizado na tela, tornando o simulador uma excelente ferramenta de aprendizado.

# Biblioteca Externa de Carregamento de Imagens - `stb_image.h`

O arquivo [stb_image.h](file:///C:/Users/yagog/Documents/coding/comp-grafic/opengl_visualizer/src/stb_image.h) é uma biblioteca de terceiros integrada ao projeto. Criada por Sean Barrett, ela é amplamente utilizada no desenvolvimento de jogos e computação gráfica por ser uma biblioteca do tipo *single-header file* (contendo tanto declarações quanto a implementação em um único arquivo de cabeçalho).

---

## 🔍 Visão Geral e Utilização

### 1. Inclusão no Projeto
Para que a implementação do cabeçalho seja compilada, é necessário definir a macro `STB_IMAGE_IMPLEMENTATION` em exatamente um arquivo C++ antes de incluí-lo. No nosso projeto, isso é feito dentro do arquivo `utils.cpp`:

```cpp
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
```

* **Por que isso é necessário?** O arquivo `.h` contém o código-fonte C de carregamento de imagens envolvido por diretivas do pré-processador. Definir a macro faz com que o compilador de fato gere o código das funções de decodificação de imagem em `utils.o`. Sem essa definição em apenas um local, o ligador (*linker*) reclamaria de símbolos indefinidos. Se incluído em múltiplos lugares com a macro, acusaria definição duplicada.

### 2. Principais Funções Utilizadas

A principal função invocada no utilitário de carregamento de texturas é `stbi_load`:

```cpp
unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
```

* **Parâmetros:**
  1. `path` (const char*): Caminho relativo ou absoluto para o arquivo de imagem no disco.
  2. `&width` (int*): Ponteiro para inteiro onde a largura da imagem (em pixels) será gravada.
  3. `&height` (int*): Ponteiro para inteiro onde a altura da imagem (em pixels) será gravada.
  4. `&nrComponents` (int*): Ponteiro para inteiro onde a quantidade de canais de cor da imagem (ex: 1 para tons de cinza, 3 para RGB, 4 para RGBA) será retornada.
  5. `0` (int): Parâmetro que especifica o número de canais desejado para a saída. Passar `0` diz à biblioteca para preservar o formato original da imagem física.
* **Retorno:** Retorna um ponteiro para um bloco de memória alocado no *heap* (`unsigned char*`) contendo os bytes brutos dos pixels sequenciados horizontalmente de cima para baixo. Em caso de erro (arquivo não encontrado ou formato corrompido), retorna `NULL`.

Após transferir a imagem para o OpenGL, a memória de pixels temporária na CPU deve ser desalojada:

```cpp
stbi_image_free(data);
```
* **Motivação:** Como a imagem agora reside inteiramente na memória de vídeo (VRAM) da GPU gerenciada pelo OpenGL, manter essa cópia na memória do sistema (RAM) causaria vazamento de memória (*memory leak*).

---

## 💡 Motivações para a Escolha da stb_image

1. **Facilidade de Instalação:** Não requer dependências externas complexas de compilação ou vinculação de DLLs adicionais como `libpng`, `libjpeg` ou `DevIL`. É portátil e funciona nativamente em Windows, macOS e Linux.
2. **Suporte Amplo de Formatos:** Decodifica automaticamente JPEG, PNG, BMP, GIF, PSD, TGA e outros formatos populares, facilitando o uso de assets personalizados de arte 2D no simulador.

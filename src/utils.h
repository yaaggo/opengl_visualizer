// Função para checar a colisão entre dois objetos
bool collision(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2);

// Desenha uma string na posição (x, y) determinada com a fonte escolhida
void draw_text(float x, float y, void *font, const unsigned char* text);

// Função auxiliar para desenhar círculos
void draw_circle(float cx, float cy, float radius, int segments);
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

// Constantes

#define P 4397 // Número primo para a função de hash

// Tabela Hash

typedef struct {
    int32_t chave;
    vEB *valor;
    bool deletado;
} No;

typedef struct {
    int32_t tam, cont;
    No *tabela;
    uint64_t a1, a2, a3, a4, a5;
} TabelaHash;

void IniciarTabelaHash(TabelaHash* T, int32_t tam) {
    T->tam = tam;
    T->cont = 0;

    T->tabela = (No*) malloc(tam * sizeof(No));

    for (int i = 0; i < tam; i++) {
        T->tabela[i].chave = -1;
        T->tabela[i].valor = NULL;
        T->tabela[i].deletado = false;
    }
    
    // Coeficientes aleatórios (entre 2 e P - 1) para o hash 5-independente
    T->a1 = (rand() % (P - 2)) + 2;
    T->a2 = (rand() % (P - 2)) + 2;
    T->a3 = (rand() % (P - 2)) + 2;
    T->a4 = (rand() % (P - 2)) + 2;
    T->a5 = (rand() % (P - 2)) + 2;
}

typedef struct van_emde_boas {
    int64_t min, max;
    TabelaHash cluster;
    struct van_emde_boas *resumo;
} vEB;

// Funções auxiliares para manipular os índices
int get_c(int x) { return x >> (W >> 1); };
int get_i(int x) { return x & ((1 << (W >> 1)) - 1); };
int tupla(int c, int i) { return (c << (W >> 1)) | i; };
// Função para trocar valores

int Sucessor(vEB *V, int x) {
    int i = get_i(x), c = get_c(x); // Não estava no pseudo-código

    if (x < V->min)
        return V->min;

    if (i < V->cluster[c]->max)
        return tupla(c, Sucessor(V->cluster[c], i));

    c = Sucessor(V->resumo, c); // Reutilizando a variável c (melhor do que criar c')
    return tupla(c, V->cluster[c]->min);
}

void Incluir(vEB *V, int x) {
    int i = get_i(x), c = get_c(x); // Não estava no pseudo-código

    if (V->min == NULO) 
        V->min = V->max = x;
    else {
        if (x < V->min) {
            int temp = V->min; V->min = x; x = temp; // Troca x <-> V->min
        }
        
        if (x > V->max)
            V->max = x;

        // 

        if (V->cluster[c]->min == NULO) 
            Incluir(V->resumo, c);
        
        Incluir(V->cluster[c], i);
    }
}

void Remover(vEB *V, int x) {
    int i = get_i(x), c = get_c(x); // Não estava no pseudo-código

    if (x == V->min) {
        c = V->resumo->min;
        i = V->cluster[c]->min;

        if (c == NULO)
            V->min = NULO; return;

        x = V->min = tupla(c, i);
    }

    Remover(V->cluster[c], i);
    
    if (V->cluster[c]->min == NULO)
        Remover(V->resumo, c);

    if (V->resumo->min == NULO)
        V->max = V->min;
    else {
        c = V->resumo->max; // Reutilizando a variável c (melhor do que criar c')
        V->max = tupla(c, V->cluster[c]->max);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Como usar: %s <arquivo>\n", argv[0]);
        return 1;
    }

    // Lendo o arquivo de entrada
    FILE *arquivo = fopen(argv[1], "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir arquivo");
        return 1;
    }

    char tag[4]; int a;
    while (fscanf(arquivo, "%s %d", tag, &a) >= 1) { // a será opcional
        if (strcmp(tag, "INC") == 0) {
            // PASS
        } else if (strcmp(tag, "REM") == 0) {
            // PASS
        } else if (strcmp(tag, "SUC") == 0) {
            printf("SUC %d\n", a);

            // PASS
        } else if (strcmp(tag, "PRE") == 0) {
            printf("PRE %d\n", a);

        } else if (strcmp(tag, "IMP") == 0) {
            printf("IMP\n");
            
            // PASS
        } else {
            // fprintf(stderr, "Tag inválida: %s\n", tag);
            // return 1;
        }
    }
    
    return 0;
}
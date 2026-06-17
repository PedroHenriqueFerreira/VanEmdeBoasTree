#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// Constantes

#define INITIAL_TABLE_SIZE 4 // Tamanho inicial da tabela hash
#define P 65537 // Número primo para a função de hash
#define W 32 // Número de bits para o universo (2^32)

typedef struct van_emde_boas vEB;

// -------------- Tabela Hash --------------

typedef struct {
    int32_t chave;
    vEB *valor; 
    bool ativo;
} No;

typedef struct {
    int32_t tam, cont;
    No *tabela;
    uint64_t a1, a2, a3, a4, a5;
} TabelaHash;

void IniciarTabela(TabelaHash *T, int32_t tam) {
    T->tam = tam;
    T->cont = 0;

    T->tabela = (No*) malloc(tam * sizeof(No));

    // Inicializa a tabela com chaves -1 (indicando posição vazia) e valores NULL
    for (int32_t i = 0; i < tam; i++) {
        T->tabela[i].chave = -1;
        T->tabela[i].valor = NULL;
        T->tabela[i].ativo = true;
    }
    
    // Coeficientes aleatórios (entre 2 e P - 1) para o hash 5-independente
    T->a1 = (rand() % (P - 2)) + 2;
    T->a2 = (rand() % (P - 2)) + 2;
    T->a3 = (rand() % (P - 2)) + 2;
    T->a4 = (rand() % (P - 2)) + 2;
    T->a5 = (rand() % (P - 2)) + 2;
}

int32_t MapearChave(TabelaHash *T, int32_t chave) {
    uint64_t k = (uint64_t) chave;
    uint64_t soma = T->a5 * k * k * k * k + 
                    T->a4 * k * k * k + 
                    T->a3 * k * k + 
                    T->a2 * k + 
                    T->a1;

    return (soma % P) % T->tam;
}

void IncluirChaveInterno(TabelaHash *T, int32_t chave, vEB *valor) {
    int32_t idx = MapearChave(T, chave);

    // Encontra a posição correta para incluir usando sondagem linear
    while (T->tabela[idx].chave != -1 && T->tabela[idx].ativo && T->tabela[idx].chave != chave) {
        idx = (idx + 1) % T->tam;
    }

    // Insere a chave e o valor na posição encontrada
    if (T->tabela[idx].chave != chave || !T->tabela[idx].ativo) {
        T->tabela[idx].chave = chave;
        T->tabela[idx].valor = valor;
        T->tabela[idx].ativo = true;
        T->cont++;
    }
}

void RedimensionarTabela(TabelaHash *T, int32_t tam) {
    int32_t tam_velho = T->tam;
    No* tabela = T->tabela;
    
    IniciarTabela(T, tam);
    
    // Reinsere os elementos da tabela antiga na nova tabela
    for (int32_t i = 0; i < tam_velho; i++) {
        if (tabela[i].chave != -1 && tabela[i].ativo) {
            IncluirChaveInterno(T, tabela[i].chave, tabela[i].valor);
        }
    }

    free(tabela);
}

void DobrarTabela(TabelaHash *T) { 
    RedimensionarTabela(T, T->tam * 2); 
}

void DividirTabela(TabelaHash *T) { 
    RedimensionarTabela(T, T->tam / 2 > INITIAL_TABLE_SIZE ? T->tam / 2 : INITIAL_TABLE_SIZE); 
}

void IncluirChave(TabelaHash *T, int32_t chave, vEB *valor) {
    IncluirChaveInterno(T, chave, valor);

    if (T->cont > T->tam / 2) 
        DobrarTabela(T);
}

int32_t EncontrarChave(TabelaHash *T, unsigned int chave, No **valor) {
    if (T->tam == 0) 
        return -1;

    int32_t idx = MapearChave(T, chave);
    int32_t idx_inicial = idx;
    
    // Sondagem linear para encontrar a chave
    while (T->tabela[idx].chave != -1) {
        if (T->tabela[idx].chave == chave) {
            if (!T->tabela[idx].ativo) 
                return -1;

            *valor = &T->tabela[idx];

            return idx;
        }

        idx = (idx + 1) % T->tam;

        if (idx == idx_inicial) 
            return -1;
    }

    return -1;
}

void DeletarChave(TabelaHash* T, int32_t chave) {
    No* no = NULL;
    int32_t idx = EncontrarChave(T, chave, &no);

    if (idx != -1 && no) {
        no->ativo = false;
        T->cont--;
        
        if (T->cont < (T->tam / 4) && T->tam > INITIAL_TABLE_SIZE)
            DividirTabela(T);
    }
}

// Árvore Van Emde Boas

struct van_emde_boas {
    unsigned int min, max;
    bool vazio;
    TabelaHash *clusters;
    struct van_emde_boas *resumo;
};

vEB* Iniciar(int w) {
    vEB* V = (vEB*) malloc(sizeof(vEB));
    
    V->vazio = true;
    V->resumo = NULL;

    if (w > 1)
        IniciarTabela(V->clusters, INITIAL_TABLE_SIZE);

    return V;
}

// Funções auxiliares para dividir x em <c, i>
unsigned int get_c(unsigned int x, int w) { return x >> (w >> 1); };
unsigned int get_i(unsigned int x, int w) { return x & ((1 << (w >> 1)) - 1); };

// Função para combinar <c, i> em x
unsigned int tupla(unsigned int c, unsigned int i, int w) { return (c << (w >> 1)) | i; };

// Retorna true se encontrou sucessor, false caso contrário (não estava no pseudo-código)
bool Sucessor(vEB* V, unsigned int x, int w, unsigned int *saida) {
    if (x < V->min) {
        *saida = V->min; return true;
    }

    // Não estava no pseudo-código
    if (x >= V->max) 
        return false;

    int32_t c = get_c(x, w), i = get_i(x, w);

    No* cluster = NULL;
    if (EncontrarChave(V->clusters, c, &cluster) != -1) { // Encontra o cluster de c
        if (i < cluster->valor->max)
            return tupla(c, Sucessor(cluster->valor, i, w >> 1, saida), w);
    }

    if (V->resumo != NULL) {
        int64_t c_ = Sucessor(V->resumo, c, w >> 1);

        if (c_ != INT64_MAX) {
            cluster = NULL;
            EncontrarChave(V->clusters, c_, &cluster);
    
            return tupla(c_, cluster->valor->min, w);
        }
    }

    return INT64_MAX;
}

// Simétrico ao Sucessor
int64_t Predecessor(vEB* V, unsigned int x, int w) {
    if (x > V->max) 
        return V->max;

    if (x <= V->min) 
        return INT64_MIN;

    int32_t c = get_c(x, w), i = get_i(x, w);

    No* cluster = NULL;
    if (EncontrarChave(V->clusters, c, &cluster) != -1) {
        if (i > cluster->valor->min)
            return tupla(c, Predecessor(cluster->valor, i, w >> 1), w);
    }

    if (V->resumo != NULL) {
        int64_t c_ = Predecessor(V->resumo, c, w >> 1);
        if (c_ != INT64_MIN) {
            cluster = NULL;
            EncontrarChave(V->clusters, c_, &cluster);

            return tupla(c_, cluster->valor->max);
        }
    }

    if (V->min < x) 
        return V->min;

    return INT64_MIN;
}

void Incluir(vEB* V, unsigned int x, int w) {
    // Se x já está presente, não faz nada (não estava no pseudo-código)
    if (!V->vazio && (V->min == x || V->max == x)) 
        return;

    if (V->vazio) { // Equivalente a "se V->min == NULO" no pseudo-código
        V->min = V->max = x; V->vazio = false;
    } else {    
        if (x < V->min) {
            unsigned int temp = x; x = V->min; V->min = temp; // Troca x <-> V->min
        }
    
        if (x > V->max) 
            V->max = x;
    
        int c = get_c(x, w), i = get_i(x, w); // Divide x em <c, i>

        // Adiciona a chave c na tabela de clusters se ainda não existir
        No* cluster = NULL;
        if (EncontrarChave(V->clusters, c, &cluster) == -1) {
            IncluirChave(&V->clusters, c, Iniciar(w >> 1));
            EncontrarChave(V->clusters, c, &cluster);
        }
    
        if (w > 1) { // Se w = 1, só pode conter no máximo 1 elemento (não estava no pseudo-código)
            if (cluster->valor->vazio) { // "se V->cluster[c]->min == NULO" no pseudo-código
                if (V->resumo == NULL)
                    V->resumo = Iniciar(w >> 1);
                
                Incluir(V->resumo, c, w >> 1);
            }
    
            Incluir(cluster->valor, i, w >> 1);
        }
    }
}

void Remover(vEB* V, unsigned int x, int w) {
    // Se a árvore estiver vazia, não faz nada (não estava no pseudo-código)
    if (V->vazio) 
        return;

    unsigned int c = get_c(x, w), i = get_i(x, w); // Divide x em <c, i>

    if (x == V->min) {
        if (V->resumo == NULL || V->resumo->vazio) { // "se V->resumo->min == NULO" no pseudo-código
            V->vazio = true; return;
        }

        c = V->resumo->min;

        No* cluster = NULL;
        EncontrarChave(V->clusters, c, &cluster);

        i = cluster->valor->min;

        x = V->min = tupla(c, i, w);
    }

    No* cluster = NULL;
    if (EncontrarChave(V->clusters, c, &cluster) != -1) {
        Remover(cluster->valor, i, w >> 1);

        if (cluster->valor->vazio) {
            DeletarChave(V->clusters, c);

            if (V->resumo != NULL)
                Remover(V->resumo, c, w >> 1);
        }

        if (V->resumo == NULL || V->resumo->vazio) { // "se V->resumo->min == NULO" no pseudo-código
            V->max = V->min;
        } else {
            unsigned int c_ = V->resumo->max;

            // Encontra o cluster de c_ para atualizar o max de V
            cluster = NULL;
            EncontrarChave(V->clusters, c_, &cluster);

            V->max = tupla(c_, cluster->valor->max, w);    
        }
    }
}

void Imprimir(vEB* V, int w) {
    // Caso a árvore esteja vazia, não imprime nada
    if (V->min == INT64_MAX && V->max == INT64_MIN)
        return;

    printf("Min: %ld", V->min);

    if (V->resumo != NULL) {
        for (int32_t i = 0; i < (1 << (W >> 1)); i++) {
            No* cluster = NULL;
            if (EncontrarChave(V->clusters, i, &cluster) != -1 && cluster->valor != NULL) {
                if (cluster->valor->min != INT64_MAX) {
                    int64_t curr = cluster->valor->min;
                    printf(", C[%d]: %ld", i, curr);
                    
                    int64_t proximo = Sucessor(cluster->valor, curr, w >> 1);

                    while (proximo != INT64_MAX) {
                        printf(", %ld", proximo);
                        proximo = Sucessor(cluster->valor, proximo, w >> 1);
                    }
                }
            }
        }
    }

    printf("\n");
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

    srand(time(NULL));
    vEB* V = Iniciar(W);

    char tag[4]; int64_t a;
    while (fscanf(arquivo, "%s %ld", tag, &a) >= 1) { // a será opcional
        if (strcmp(tag, "INC") == 0) {
            Incluir(V, a, W);
        } else if (strcmp(tag, "REM") == 0) {
            Remover(V, a, W);
        } else if (strcmp(tag, "SUC") == 0) {
            printf("SUC %ld\n", a);

            int64_t suc = Sucessor(V, a, W);
            
            if (suc == INT64_MAX) {
                printf("+INF\n");
            } else {
                printf("%ld\n", suc);
            }
        } else if (strcmp(tag, "PRE") == 0) {
            printf("PRE %ld\n", a);

            int64_t pre = Predecessor(V, a, W);

            if (pre == INT64_MIN) {
                printf("-INF\n");
            } else {
                printf("%ld\n", pre);
            }
        } else if (strcmp(tag, "IMP") == 0) {
            printf("IMP\n");
            
            Imprimir(V, W);
        } else {
            // fprintf(stderr, "Tag inválida: %s\n", tag);
            // return 1;
        }
    }
    
    return 0;
}
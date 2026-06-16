#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

// Constantes

#define INITIAL_TABLE_SIZE 4 // Tamanho inicial da tabela hash
#define P 4397 // Número primo para a função de hash
#define W 32 // Número de bits para o universo (2^32)

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

void IniciarTabela(TabelaHash *T, int32_t tam) {
    T->tam = tam;
    T->cont = 0;

    T->tabela = (No*) malloc(tam * sizeof(No));

    // Inicializa a tabela com chaves -1 (indicando posição vazia) e valores NULL
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

int MapearChave(TabelaHash *T, int32_t chave) {
    uint64_t chave = (uint64_t) chave;
    uint64_t soma = T->a5 * pow(chave, 4) + T->a4 * pow(chave, 3) + T->a3 * pow(chave, 2) + T->a2 * pow(chave, 1) + T->a1;

    return (soma % P) % T->tam;
}

void IncluirChaveInterno(TabelaHash *T, int32_t chave, vEB *valor) {
    int idx = MapearChave(T, chave);

    // Encontra a posição correta para incluir usando sondagem linear
    while (T->tabela[idx].chave != -1 && !T->tabela[idx].deletado && T->tabela[idx].chave != chave) {
        idx = (idx + 1) % T->tam;
    }

    // Insere a chave e o valor na posição encontrada
    if (T->tabela[idx].chave != chave || T->tabela[idx].deletado) {
        T->tabela[idx].chave = chave;
        T->tabela[idx].valor = valor;
        T->tabela[idx].deletado = false;
        T->cont++;
    }
}

void RedimensionarTabela(TabelaHash *T, int32_t tam) {
    int32_t tam_velho = T->tam;
    No* tabela = T->tabela;
    
    IniciarTabela(T, tam);
    
    // Reinsere os elementos da tabela antiga na nova tabela
    for (int i = 0; i < tam_velho; i++) {
        if (tabela[i].chave != -1 && !tabela[i].deletado) {
            IncluirChaveInterno(T, tabela[i].chave, tabela[i].valor);
        }
    }

    free(tabela);
}

void DobrarTabela(TabelaHash *T) { RedimensionarTabela(T, T->tam * 2); }
void DividirTabela(TabelaHash *T) { RedimensionarTabela(T, max(T->tam / 2, INITIAL_TABLE_SIZE)); }

void IncluirChave(TabelaHash *T, int32_t chave, vEB *valor) {
    IncluirChaveInterno(T, chave, valor);

    if (T->cont > T->tam / 2) 
        DobrarTabela(T);
}

int EncontrarChave(TabelaHash *T, int32_t chave, No **valor) {
    if (T->tam == 0) 
        return -1;

    int idx = MapearChave(T, chave);
    int idx_inicial = idx;
    
    // Sondagem linear para encontrar a chave
    while (T->tabela[idx].chave != -1) {
        if (T->tabela[idx].chave == chave) {
            if (T->tabela[idx].deletado) 
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
    int idx = EncontrarChave(T, chave, &no);

    if (idx != -1 && no) {
        no->deletado = true;
        T->cont--;
        
        if (T->cont < (T->tam / 4) && T->tam > INITIAL_TABLE_SIZE)
            DividirTabela(T);
    }
}

// Árvore Van Emde Boas

typedef struct van_emde_boas {
    int64_t min, max;
    TabelaHash clusters;
    struct van_emde_boas *resumo;
} vEB;

vEB* Iniciar() {
    vEB* V = (vEB*) malloc(sizeof(vEB));
    
    V->min = INT64_MAX;
    V->max = INT64_MIN;

    IniciarTabela(&V->clusters, INITIAL_TABLE_SIZE);

    V->resumo = NULL;

    return V;
}

// Funções auxiliares para dividir x em <c, i>
int32_t get_c(int64_t x) { return x >> (W >> 1); };
int32_t get_i(int64_t x) { return x & ((1 << (W >> 1)) - 1); };

// Função para combinar <c, i> em x
int64_t tupla(int32_t c, int32_t i) { return ((int64_t) c << (W >> 1)) | i; };

int Sucessor(vEB *V, int x) {
    int i = get_i(x), c = get_c(x); // Não estava no pseudo-código

    if (x < V->min)
        return V->min;

    if (i < V->cluster[c]->max)
        return tupla(c, Sucessor(V->cluster[c], i));

    c = Sucessor(V->resumo, c); // Reutilizando a variável c (melhor do que criar c')
    return tupla(c, V->cluster[c]->min);
}

int64_t Sucessor(vEB* V, int64_t x) {
    if (x < V->min) 
        return V->min;

    if (x >= V->max) 
        return INT64_MAX;

    int32_t c = get_c(x), i = get_i(x);

    No* clusterNode = NULL;
    if (EncontrarChave(&V->clusters, c, &clusterNode) != -1) {
        if (i < clusterNode->valor->max)
            return tupla(c, Sucessor(clusterNode->valor, i));
    }

    if (V->resumo != NULL) {
        int64_t j = Sucessor(V->resumo, c);
        if (j != INT64_MAX) {
            No* nextClusterNode = NULL;
            hashFind(&V->clusters, (int32_t) j, &nextClusterNode);

            return tupla((int32_t) j, (int32_t) nextClusterNode->valor->min);
        }
    }

    return INT64_MAX;
}

void Incluir(vEB* V, int64_t x) {
    // Se x já está presente, não faz nada (não estava no pseudo-código)
    if (V->min == x || V->max == x) 
        return;

    if (V->min > V->max) { // Equivalente a "se V->min == NULO" no pseudo-código
        V->min = V->max = x;
    } else {
        if (x < V->min) {
            int64_t temp = x; x = V->min; V->min = temp; // Troca x <-> V->min
        }
    
        if (x > V->max) 
            V->max = x;
    
        int32_t c = get_c(x), i = get_i(x);
    
        // Adiciona a chave c na tabela de clusters se ainda não existir
        No* cluster = NULL;
        if (EncontrarChave(&V->clusters, c, &cluster) == -1) {
            IncluirChave(&V->clusters, c, Iniciar());
            EncontrarChave(&V->clusters, c, &cluster);
        }
    
        if (cluster->valor->min > cluster->valor->max) { // "se V->cluster[c]->min == NULO" no pseudo-código
            if (V->resumo == NULL)
                V->resumo = Iniciar();
            
            Incluir(V->resumo, c);
        }

        Incluir(cluster->valor, i);
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
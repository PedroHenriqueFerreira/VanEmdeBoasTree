#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// Constantes
#define INITIAL_TABLE_SIZE 4 // Tamanho inicial da tabela hash
#define P 65537 // Número primo para a função de hash (maior que 2^16)
#define W 32 // Número de bits para o universo (2^32)

typedef struct van_emde_boas vEB;

// Item da tabela hash
typedef struct {
    uint32_t chave;
    vEB *valor; 
    bool ativo;
} No;

// Tabela hash com sondagem linear e hash 5-independente
typedef struct {
    uint32_t tam, cont;
    No *tabela;
    uint32_t a1, a2, a3, a4, a5;
} TabelaHash;

// Inicializa a tabela hash com o tamanho especificado
void IniciarTabela(TabelaHash *T, uint32_t tam) {
    T->tam = tam;
    T->cont = 0;

    T->tabela = (No*) malloc(tam * sizeof(No));

    // Inicializa a tabela com chaves -1 (indicando posição vazia) e valores NULL
    for (uint32_t i = 0; i < tam; i++) {
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

// Função de hash 5-independente para mapear a chave ao índice da tabela
uint32_t MapearChave(TabelaHash *T, uint32_t chave) {
    uint64_t k = (uint64_t) chave;
    uint64_t soma = T->a5;

    soma = (soma * k + T->a4) % P; 
    soma = (soma * k + T->a3) % P; 
    soma = (soma * k + T->a2) % P; 
    soma = (soma * k + T->a1) % P;

    // soma = k^4 * a1 + k^3 * a2 + k^2 * a3 + k^1 * a4 + a5 (mod P)

    return (uint32_t) (soma % T->tam);
}

// Função interna para incluir chave e valor sem redimensionar a tabela
void IncluirChaveInterno(TabelaHash *T, uint32_t chave, vEB *valor) {
    uint32_t idx = MapearChave(T, chave);

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

// Redimensiona a tabela para o novo tamanho, re-inserindo os elementos
void RedimensionarTabela(TabelaHash *T, uint32_t tam) {
    uint32_t tam_velho = T->tam;
    No* tabela = T->tabela;
    
    IniciarTabela(T, tam);
    
    // Reinsere os elementos da tabela antiga na nova tabela
    for (uint32_t i = 0; i < tam_velho; i++) {
        if (tabela[i].chave != -1 && tabela[i].ativo) {
            IncluirChaveInterno(T, tabela[i].chave, tabela[i].valor);
        }
    }

    free(tabela);
}

// Dobra o tamanho da tabela
void DobrarTabela(TabelaHash *T) { 
    RedimensionarTabela(T, T->tam * 2); 
}

// Reduz o tamanho da tabela pela metade, mas nunca abaixo do tamanho inicial
void DividirTabela(TabelaHash *T) { 
    RedimensionarTabela(T, T->tam / 2 > INITIAL_TABLE_SIZE ? T->tam / 2 : INITIAL_TABLE_SIZE); 
}

// Inclui a chave e o valor, redimensionando a tabela se necessário
void IncluirChave(TabelaHash *T, uint32_t chave, vEB *valor) {
    IncluirChaveInterno(T, chave, valor);

    if (T->cont > T->tam / 2) 
        DobrarTabela(T);
}

// Retorna o índice da chave encontrada ou -1 se não encontrada.
int32_t EncontrarChave(TabelaHash *T, uint32_t chave, No **valor) {
    if (T->tam == 0) 
        return -1;

    uint32_t idx = MapearChave(T, chave);
    uint32_t idx_inicial = idx;
    
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

// Marca a chave como inativa e redimensiona a tabela se necessário
void DeletarChave(TabelaHash* T, uint32_t chave) {
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
    uint32_t min, max;
    bool vazio;
    TabelaHash *clusters;
    struct van_emde_boas *resumo;
};

// Função para inicializar uma árvore van Emde Boas de universo 2^w
vEB* AlocarVEB(uint32_t w) {
    vEB* V = (vEB*) malloc(sizeof(vEB));
    
    V->vazio = true;
    V->resumo = NULL;
    V->clusters = NULL;

    if (w > 1) {
        V->clusters = (TabelaHash*) malloc(sizeof(TabelaHash));
        IniciarTabela(V->clusters, INITIAL_TABLE_SIZE);
    }

    return V;
}

// Função para liberar a memória alocada pela árvore van Emde Boas
void LiberarVEB(vEB* V) {
    // Caso base: se V for NULL, não há nada para liberar
    if (V == NULL) 
        return;
    
    // Libera recursivamente os clusters se existirem
    if (V->clusters != NULL) {
        for (uint32_t i = 0; i < V->clusters->tam; i++) {
            if (V->clusters->tabela[i].chave != -1 && V->clusters->tabela[i].ativo)
                LiberarVEB(V->clusters->tabela[i].valor);
        }

        free(V->clusters->tabela);
        free(V->clusters);
    }
    
    // Libera recursivamente o resumo se existir
    if (V->resumo != NULL)
        LiberarVEB(V->resumo);

    free(V);
}

// Função auxiliar para dividir x em <c, ...>
uint32_t get_c(uint32_t x, uint32_t w) { return x >> (w >> 1); };
// Função auxiliar para dividir x em <..., i>
uint32_t get_i(uint32_t x, uint32_t w) { return x & ((1U << (w >> 1)) - 1); };

// Função para combinar <c, i> em x
uint32_t tupla(uint32_t c, uint32_t i, uint32_t w) { return (c << (w >> 1)) | i; };

// Retorna true se encontrou sucessor, false caso contrário
bool SucessorVEB(vEB* V, uint32_t x, uint32_t w, uint32_t *saida) {
    if (V->vazio)
        return false; 

    // Caso base (não estava no pseudo-código)
    if (w == 1) {
        if (x < V->max) {
            *saida = V->max; return true;
        }

        return false;
    }

    if (x < V->min) {
        *saida = V->min; return true;
    }

    // Não estava no pseudo-código, mas foi mencionado pelo professor
    if (x >= V->max) 
        return false;

    uint32_t c = get_c(x, w), i = get_i(x, w);

    No* cluster = NULL;
    if (EncontrarChave(V->clusters, c, &cluster) != -1) { // Encontra o cluster de c
        if (i < cluster->valor->max) {
            uint32_t suc;
            bool has_suc = SucessorVEB(cluster->valor, i, w >> 1, &suc);
            *saida = tupla(c, suc, w); return has_suc;
        }
    }

    if (V->resumo != NULL) {
        uint32_t c_;
        if (SucessorVEB(V->resumo, c, w >> 1, &c_)) {
            cluster = NULL;
            EncontrarChave(V->clusters, c_, &cluster);
    
            *saida = tupla(c_, cluster->valor->min, w); return true;
        }
    }

    return false;
}

// Retorna true se encontrou predecessor, false caso contrário
bool PredecessorVEB(vEB* V, uint32_t x, uint32_t w, uint32_t *saida) {
    if (V->vazio)
        return false;

    // Caso base (não estava no pseudo-código)
    if (w == 1) {
        if (x > V->min) {
            *saida = V->min; return true;
        }

        return false;
    }

    if (x > V->max) {
        *saida = V->max; return true;
    }

    if (x <= V->min) 
        return false;

    uint32_t c = get_c(x, w), i = get_i(x, w);

    No* cluster = NULL;
    if (EncontrarChave(V->clusters, c, &cluster) != -1) {
        if (i > cluster->valor->min) {
            uint32_t pre;
            bool has_pre = PredecessorVEB(cluster->valor, i, w >> 1, &pre);

            *saida = tupla(c, pre, w); return has_pre;
        }
    }

    if (V->resumo != NULL) {
        uint32_t c_;
        bool has_pre = PredecessorVEB(V->resumo, c, w >> 1, &c_);
        if (has_pre) {
            cluster = NULL;
            EncontrarChave(V->clusters, c_, &cluster);

            *saida = tupla(c_, cluster->valor->max, w); return true;
        }
    }
    
    if (V->min < x) {
        *saida = V->min; return true;
    }    
    
    return false;
}

void IncluirVEB(vEB* V, uint32_t x, uint32_t w) {
    // Não estava no pseudo-código
    if (!V->vazio) {
        // Se x já está presente, não faz nada
        if (V->min == x || V->max == x)
            return;
        
        // Caso base para w = 1
        if (w == 1) {
            if (x < V->min) {
                V->max = V->min;
                V->min = x;
            } else if (x > V->max)
                V->max = x;

            return;
        }
    }

    if (V->vazio) { // Equivalente a "se V->min == NULO" no pseudo-código
        V->min = V->max = x; V->vazio = false;
    } else {    
        if (x < V->min) {
            uint32_t temp = x; x = V->min; V->min = temp; // Troca x <-> V->min
        }
    
        if (x > V->max) 
            V->max = x;
    
        uint32_t c = get_c(x, w), i = get_i(x, w); // Divide x em <c, i>

        // Adiciona a chave c na tabela de clusters se ainda não existir
        No* cluster = NULL;
        if (EncontrarChave(V->clusters, c, &cluster) == -1) {
            IncluirChave(V->clusters, c, AlocarVEB(w >> 1));
            EncontrarChave(V->clusters, c, &cluster);
        }

        if (cluster->valor->vazio) { // "se V->cluster[c]->min == NULO" no pseudo-código
            if (V->resumo == NULL)
                V->resumo = AlocarVEB(w >> 1);
            
            IncluirVEB(V->resumo, c, w >> 1);
        }

        IncluirVEB(cluster->valor, i, w >> 1);
    
    }
}

void RemoverVEB(vEB* V, uint32_t x, uint32_t w) {
    // Se a árvore estiver vazia, não faz nada (não estava no pseudo-código)
    if (V->vazio) 
        return;

    // Caso base (não estava no pseudo-código)
    if (w == 1) {
        if (x == V->min && x == V->max)
            V->vazio = true;
        else if (x == V->min)
            V->min = V->max;
        else if (x == V->max)
            V->max = V->min;

        return;
    }    

    uint32_t c = get_c(x, w), i = get_i(x, w); // Divide x em <c, i>

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
        RemoverVEB(cluster->valor, i, w >> 1);

        if (cluster->valor->vazio) {
            LiberarVEB(cluster->valor);
            DeletarChave(V->clusters, c);

            if (V->resumo != NULL)
                RemoverVEB(V->resumo, c, w >> 1);
        }

        if (V->resumo == NULL || V->resumo->vazio) { // "se V->resumo->min == NULO" no pseudo-código
            V->max = V->min;
        } else {
            uint32_t c_ = V->resumo->max;

            // Encontra o cluster de c_ para atualizar o max de V
            cluster = NULL;
            EncontrarChave(V->clusters, c_, &cluster);

            V->max = tupla(c_, cluster->valor->max, w);    
        }
    }
}

// Função para imprimir os elementos da árvore van Emde Boas
void ImprimirVEB(vEB* V, uint32_t w) {
    // Caso a árvore esteja vazia, não imprime nada
    if (V->vazio) {
        printf("\n"); // Linha nova se não houver nada para imprimir
        return;
    }

    printf("Min: %u", V->min);

    if (V->resumo != NULL && !V->resumo->vazio) {
        uint32_t c = V->resumo->min, curr;

        while (true) {
            No* cluster = NULL;
            EncontrarChave(V->clusters, c, &cluster);
            
            curr = cluster->valor->min;
            printf(", C[%u]: %u", c, curr);
            
            while (SucessorVEB(cluster->valor, curr, w >> 1, &curr))
                printf(", %u", curr);
        
            if (!SucessorVEB(V->resumo, c, w >> 1, &c))
                break;
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
    vEB* V = AlocarVEB(W);

    char tag[4]; uint32_t a;
    while (fscanf(arquivo, "%s %u", tag, &a) >= 1) { // a será opcional
        if (strcmp(tag, "INC") == 0)
            IncluirVEB(V, a, W);
        else if (strcmp(tag, "REM") == 0)
            RemoverVEB(V, a, W);
        else if (strcmp(tag, "SUC") == 0) {
            printf("SUC %u\n", a);

            uint32_t suc;
            if (SucessorVEB(V, a, W, &suc))
                printf("%u\n", suc);
            else
                printf("+INF\n");
        } else if (strcmp(tag, "PRE") == 0) {
            printf("PRE %u\n", a);

            uint32_t pre;
            if (PredecessorVEB(V, a, W, &pre))
                printf("%u\n", pre);
            else
                printf("-INF\n");
        } else if (strcmp(tag, "IMP") == 0) {
            printf("IMP\n");
            
            ImprimirVEB(V, W);
        } else {
            // fprintf(stderr, "Tag inválida: %s\n", tag);
            // return 1;
        }
    }
    
    return 0;
}
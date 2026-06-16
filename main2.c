#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>

//  10100110000001000
// 1 0100110000001000

#define INITIAL_SIZE 4
#define P 4397 // Número primo para a função de hash
#define MSQRT 65536 // Raiz quadrada do universo 2^32
#define INF_PLUS INT64_MAX
#define INF_MINUS INT64_MIN

typedef struct vEBTree vEBTree;

typedef struct {
    int32_t key;
    bool deleted;
    vEBTree* pointer;
} Node;

typedef struct {
    int32_t size;
    int32_t filled;
    Node* table;
    uint64_t a1, a2, a3, a4, a5;
} HashTable;

struct vEBTree {
    int64_t min;
    int64_t max;
    HashTable clusters;
    vEBTree* resumo;
};

// Protótipos
void initHashTable(HashTable* ht, int32_t size);
void hashInsertInternal(HashTable* ht, int32_t key, vEBTree* ptr);
int32_t hashInsert(HashTable* ht, int32_t key, vEBTree* tree);
int hashFind(HashTable* ht, int32_t key, Node** outNode);
int32_t hashDelete(HashTable* ht, int32_t key);

vEBTree* createVEBTree();
int64_t vEBInsert(vEBTree* tree, int64_t x);
int64_t vEBSuccessor(vEBTree* tree, int64_t x);
int64_t vEBPredecessor(vEBTree* tree, int64_t x);
int64_t vEBRemove(vEBTree* tree, int64_t x);
void vEBPrint(vEBTree* tree);

// --- Funções de Hash Table ---

void initHashTable(HashTable* ht, int32_t size) {
    ht->size = size;
    ht->filled = 0;
    ht->table = (Node*)malloc(size * sizeof(Node));
    for (int i = 0; i < size; i++) {
        ht->table[i].key = -1;
        ht->table[i].deleted = false;
        ht->table[i].pointer = NULL;
    }
    
    // Coeficientes para hash 5-independente
    ht->a1 = (rand() % (P - 2)) + 2;
    ht->a2 = (rand() % (P - 2)) + 2;
    ht->a3 = (rand() % (P - 2)) + 2;
    ht->a4 = (rand() % (P - 2)) + 2;
    ht->a5 = (rand() % (P - 2)) + 2;
}

int hashFunction(HashTable* ht, int32_t k) {
    uint64_t k_u = (uint64_t)k;
    uint64_t sum = ht->a5 * k_u * k_u * k_u * k_u +
                   ht->a4 * k_u * k_u * k_u +
                   ht->a3 * k_u * k_u +
                   ht->a2 * k_u + 
                   ht->a1;
    return (sum % P) % ht->size;
}

void tableDoubling(HashTable* ht) {
    int32_t old_size = ht->size;
    Node* old_table = ht->table;
    
    initHashTable(ht, old_size * 2);
    
    for (int i = 0; i < old_size; i++) {
        if (old_table[i].key != -1 && !old_table[i].deleted) {
            hashInsertInternal(ht, old_table[i].key, old_table[i].pointer);
        }
    }
    free(old_table);
}

void tableHalving(HashTable* ht) {
    int32_t old_size = ht->size;
    Node* old_table = ht->table;
    
    int32_t new_size = old_size / 2;
    if (new_size < INITIAL_SIZE) new_size = INITIAL_SIZE;
    
    initHashTable(ht, new_size);
    
    for (int i = 0; i < old_size; i++) {
        if (old_table[i].key != -1 && !old_table[i].deleted) {
            hashInsertInternal(ht, old_table[i].key, old_table[i].pointer);
        }
    }
    free(old_table);
}

void hashInsertInternal(HashTable* ht, int32_t key, vEBTree* ptr) {
    int idx = hashFunction(ht, key);
    while (ht->table[idx].key != -1 && !ht->table[idx].deleted && ht->table[idx].key != key) {
        idx = (idx + 1) % ht->size;
    }
    if (ht->table[idx].key != key || ht->table[idx].deleted) {
        ht->table[idx].key = key;
        ht->table[idx].deleted = false;
        ht->table[idx].pointer = ptr;
        ht->filled++;
    }
}

int32_t hashInsert(HashTable* ht, int32_t key, vEBTree* tree) {
    hashInsertInternal(ht, key, tree);
    if (ht->filled > ht->size / 2) {
        tableDoubling(ht);
    }
    return 1;
}

int hashFind(HashTable* ht, int32_t key, Node** outNode) {
    if (ht->size == 0) return -1;
    int idx = hashFunction(ht, key);
    int start_idx = idx;
    
    while (ht->table[idx].key != -1) {
        if (ht->table[idx].key == key) {
            if (ht->table[idx].deleted) return -1;
            if (outNode) *outNode = &ht->table[idx];
            return idx;
        }
        idx = (idx + 1) % ht->size;
        if (idx == start_idx) break;
    }
    return -1;
}

int32_t hashDelete(HashTable* ht, int32_t key) {
    Node* node = NULL;
    int idx = hashFind(ht, key, &node);
    if (idx != -1 && node) {
        node->deleted = true;
        ht->filled--;
        if (ht->filled < ht->size / 4 && ht->size > INITIAL_SIZE) {
            tableHalving(ht);
        }
        return idx;
    }
    return -1;
}

// --- Funções da Árvore vEB ---

vEBTree* createVEBTree() {
    vEBTree* tree = (vEBTree*)malloc(sizeof(vEBTree));
    tree->min = INF_PLUS;
    tree->max = INF_MINUS;
    initHashTable(&tree->clusters, INITIAL_SIZE);
    tree->resumo = NULL;
    return tree;
}

int64_t vEBInsert(vEBTree* tree, int64_t x) {
    if (tree->min == x || tree->max == x) return x;

    if (tree->min > tree->max) {
        tree->min = x;
        tree->max = x;
        return x;
    }

    if (x < tree->min) {
        int64_t aux = x;
        x = tree->min;
        tree->min = aux;
    }

    if (x > tree->max) {
        tree->max = x;
    }

    int32_t i = x / MSQRT;
    int32_t lo = x % MSQRT;

    Node* clusterNode = NULL;
    if (hashFind(&tree->clusters, i, &clusterNode) == -1) {
        vEBTree* newCluster = createVEBTree();
        hashInsert(&tree->clusters, i, newCluster);
        hashFind(&tree->clusters, i, &clusterNode);
    }

    vEBTree* ithCluster = clusterNode->pointer;
    vEBInsert(ithCluster, lo);

    if (ithCluster->min == ithCluster->max) {
        if (tree->resumo == NULL) {
            tree->resumo = createVEBTree();
        }
        vEBInsert(tree->resumo, i);
    }

    return x;
}

int64_t vEBSuccessor(vEBTree* tree, int64_t x) {
    if (x < tree->min) return tree->min;
    if (x >= tree->max) return INF_PLUS;

    int32_t i = x / MSQRT;
    int32_t lo = x % MSQRT;

    Node* clusterNode = NULL;
    if (hashFind(&tree->clusters, i, &clusterNode) != -1) {
        vEBTree* ithCluster = clusterNode->pointer;
        if (lo < ithCluster->max) {
            return ((int64_t)MSQRT * i) + vEBSuccessor(ithCluster, lo);
        }
    }

    if (tree->resumo != NULL) {
        int64_t j = vEBSuccessor(tree->resumo, i);
        if (j != INF_PLUS) {
            Node* nextClusterNode = NULL;
            hashFind(&tree->clusters, (int32_t)j, &nextClusterNode);
            return ((int64_t)MSQRT * j) + nextClusterNode->pointer->min;
        }
    }
    return INF_PLUS;
}

int64_t vEBPredecessor(vEBTree* tree, int64_t x) {
    if (x > tree->max) return tree->max;
    if (x <= tree->min) return INF_MINUS;

    int32_t i = x / MSQRT;
    int32_t lo = x % MSQRT;

    Node* clusterNode = NULL;
    if (hashFind(&tree->clusters, i, &clusterNode) != -1) {
        vEBTree* ithCluster = clusterNode->pointer;
        if (lo > ithCluster->min) {
            return ((int64_t)MSQRT * i) + vEBPredecessor(ithCluster, lo);
        }
    }

    if (tree->resumo != NULL) {
        int64_t j = vEBPredecessor(tree->resumo, i);
        if (j != INF_MINUS) {
            Node* prevClusterNode = NULL;
            hashFind(&tree->clusters, (int32_t)j, &prevClusterNode);
            return ((int64_t)MSQRT * j) + prevClusterNode->pointer->max;
        }
    }
    
    if (tree->min < x) return tree->min;
    return INF_MINUS;
}

int64_t vEBRemove(vEBTree* tree, int64_t x) {
    if (tree->min == tree->max && tree->max == x) {
        tree->min = INF_PLUS;
        tree->max = INF_MINUS;
        return x;
    }

    if (tree->min == x) {
        if (tree->resumo == NULL || tree->resumo->min == INF_PLUS) {
            tree->min = INF_PLUS;
            tree->max = INF_MINUS;
            return x;
        }
        int32_t j = tree->resumo->min;
        Node* jthNode = NULL;
        hashFind(&tree->clusters, j, &jthNode);
        x = ((int64_t)j * MSQRT) + jthNode->pointer->min;
        tree->min = x;
    }

    int32_t i = x / MSQRT;
    int32_t lo = x % MSQRT;

    Node* ithNode = NULL;
    if (hashFind(&tree->clusters, i, &ithNode) != -1) {
        vEBTree* ithCluster = ithNode->pointer;
        vEBRemove(ithCluster, lo);

        if (ithCluster->min == INF_PLUS) {
            hashDelete(&tree->clusters, i);
            if (tree->resumo != NULL) {
                vEBRemove(tree->resumo, i);
            }
        }

        if (tree->max == x) {
            if (tree->resumo == NULL || tree->resumo->min == INF_PLUS) {
                tree->max = tree->min;
            } else {
                int32_t j = tree->resumo->max;
                Node* jthNode = NULL;
                hashFind(&tree->clusters, j, &jthNode);
                tree->max = ((int64_t)j * MSQRT) + jthNode->pointer->max;
            }
        }
        return x;
    }
    return -1;
}

void vEBPrint(vEBTree* tree) {
    if (tree->min == INF_PLUS && tree->max == INF_MINUS) {
        return; // Árvore vazia, a impressão detalhada de clusters não gera nada
    }

    printf("Min: %ld", tree->min);

    if (tree->resumo != NULL) {
        for (int i = 0; i < MSQRT; i++) {
            Node* ithNode = NULL;
            if (hashFind(&tree->clusters, i, &ithNode) != -1 && ithNode->pointer != NULL) {
                vEBTree* ithCluster = ithNode->pointer;
                if (ithCluster->min != INF_PLUS) {
                    int64_t curr = ithCluster->min;
                    printf(", C[%d]: %ld", i, curr);

                    int64_t next = vEBSuccessor(ithCluster, curr);
                    while (next != INF_PLUS) {
                        printf(", %ld", next);
                        next = vEBSuccessor(ithCluster, next);
                    }
                }
            }
        }
    }
    printf("\n");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Erro: Forneça o arquivo de entrada como argumento.\n");
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        printf("Não foi possível abrir o arquivo %s.\n", argv[1]);
        return 1;
    }

    srand(time(NULL)); // Inicializa seed para hash
    vEBTree* tree = createVEBTree();

    char command[10];
    int64_t value;

    while (fscanf(file, "%s", command) != EOF) {
        if (strcmp(command, "INC") == 0) {
            fscanf(file, "%ld", &value);
            vEBInsert(tree, value);
        } 
        else if (strcmp(command, "REM") == 0) {
            fscanf(file, "%ld", &value);
            vEBRemove(tree, value);
        } 
        else if (strcmp(command, "SUC") == 0) {
            fscanf(file, "%ld", &value);
        
            printf("SUC %ld\n", value);

            int64_t res = vEBSuccessor(tree, value);
            if (res == INF_PLUS) {
                printf("+INF\n");
            } else {
                printf("%ld\n", res);
            }
        } 
        else if (strcmp(command, "PRE") == 0) {
            fscanf(file, "%ld", &value);

            printf("PRE %ld\n", value);

            int64_t res = vEBPredecessor(tree, value);
            if (res == INF_MINUS) {
                printf("-INF\n");
            } else {
                printf("%ld\n", res);
            }
        } 
        else if (strcmp(command, "IMP") == 0) {
            printf("IMP\n");
            vEBPrint(tree);
        }
    }

    fclose(file);
    return 0;
}
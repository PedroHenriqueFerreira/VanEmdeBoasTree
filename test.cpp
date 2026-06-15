#include<stdio.h>
#include<stdlib.h>
#include <random>

using namespace std;

int main() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(2, 5);

    for (int i = 0; i < 10; i++)
        printf("Gerando 10 números aleatórios entre 2 e 5: %d\n", (rand() % (5 - 2)) + 2);


    return 0;
}
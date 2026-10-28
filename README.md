# Árvore van Emde Boas com Espaço Linear

Este repositório contém a implementação de uma Árvore van Emde Boas. A estrutura foi desenvolvida para suportar um tamanho de palavra de 32 bits e utiliza uma abordagem de espaço linear no número de elementos, implementada através de uma Tabela Hash com sondagem linear e redimensionamento dinâmico (*table doubling* e *table halving*).

---

## 1. Linguagem de Programação Usada

* **Linguagem:** C
* **Versão utilizada:** GCC 15.2.0
* **Sistema Operacional:** Ubuntu 25.10

---

## 2. Como rodar o seu programa

O projeto inclui um `makefile` para automatizar e facilitar a compilação e execução. O programa lê os comandos a partir de um arquivo de texto (`.txt`).

Abra o terminal no diretório onde os arquivos do código estão localizados e utilize os comandos abaixo:

1. **Compilação:**
   Para compilar o código-fonte e gerar o executável (`programa.exe`), execute:
   ```bash
      make build
   ```

2. **Execução padrão:**
Para rodar o programa utilizando o arquivo de entrada padrão (`entrada.txt`), execute:
   ```bash
      make run
   ```


3. **Execução com arquivo personalizado:**
Para testar um arquivo diferente (por exemplo, `entrada2.txt`), execute:
   ```bash
      make run INPUT=entrada2.txt
   ```


4. **Alternativa manual (sem makefile):**
Para compilar e executar manualmente, execute:
   ```bash
      gcc -o programa.exe main.c
      ./programa.exe entrada.txt
   ```

---

## 3. Descrição das Funções e Estruturas

A árvore van Emde Boas foi implementada de forma a otimizar o uso de memória em espaço linear utilizando uma tabela de dispersão interna em cada nível para armazenar apenas os clusters não vazios.

### Estruturas Base

* `struct No`: Representa um item na tabela hash. Contém a `chave` (índice do cluster), um ponteiro `valor` (que aponta para a subárvore vEB daquele cluster) e um indicador de `ativo` para controlar a ocupação e remoção lógica (exclusão preguiçosa).
* `struct TabelaHash`: Estrutura de endereçamento aberto que gerencia os clusters. Contém o tamanho atual (`tam`), quantidade de itens armazenados (`cont`), o vetor de instâncias `tabela`, e os coeficientes (`a1` a `a5`) utilizados na função matemática de dispersão 5-independente.
* `struct van_emde_boas` (vEB): A estrutura central. Armazena os valores mínimo e máximo do conjunto (`min` e `max`), uma flag booleana `vazio` (já que valores `uint32_t` não comportam representações negativas de nulo), a tabela hash de filhos (`clusters`) e um ponteiro para a árvore de agrupamentos não vazios (`resumo`).

### Funções da Tabela Hash

* `IniciarTabela(TabelaHash *T, uint32_t tam)`: Aloca o vetor de forma contígua e gera os 5 coeficientes aleatórios necessários para o hash 5-independente.
* `MapearChave(TabelaHash *T, uint32_t chave)`: Função hash 5-independente. Calcula um polinômio modular de grau 4 para garantir espalhamento uniforme, usando operações em `uint64_t` temporárias para evitar overflows.
* `RedimensionarTabela`, `DobrarTabela`, `DividirTabela`: Funções responsáveis pela realocação dinâmica de memória. Dobram o vetor ao ultrapassar 50% de ocupação e cortam pela metade ao cair abaixo de 25%.
* `IncluirChave` e `IncluirChaveInterno`: Inserem um novo item na tabela resolvendo colisões via sondagem linear.
* `EncontrarChave`: Realiza a busca de uma chave na tabela.
* `DeletarChave`: Realiza a remoção lógica de um cluster alterando a flag `ativo` para falso e desencadeia a redução da tabela se necessário.

### Funções Auxiliares

* `get_c(uint32_t x, uint32_t w)` e `get_i(uint32_t x, uint32_t w)`: Extraem a porção superior (índice do cluster) e a porção inferior (posição dentro do cluster) da palavra, com base na quantidade de bits atual `w`.
* `tupla(uint32_t c, uint32_t i, uint32_t w)`: Junta a porção superior e inferior para reconstruir a chave original no seu respetivo nível `w`.

### Funções Principais (Árvore vEB)

* `AlocarVEB(uint32_t w)`: Aloca e inicializa dinamicamente uma nova árvore van Emde Boas, marcando-a como vazia e inicializando a tabela hash de clusters caso não seja o caso base ($w > 1$).
* `LiberarVEB(vEB* V)`: Função recursiva que varre a estrutura de baixo para cima limpando as subárvores ativas e libertando a memória.
* `IncluirVEB(vEB* V, uint32_t x, uint32_t w)`: Insere um número estritamente dentro da sua posição correspondente na hierarquia, instanciando novos agrupamentos recursivamente.
* `RemoverVEB(vEB* V, uint32_t x, uint32_t w)`: Realiza a exclusão do nó solicitado, atualizando os limites `min` e `max` adequadamente e efetuando a limpeza de memória de clusters que fiquem vazios.
* `SucessorVEB(vEB* V, uint32_t x, uint32_t w, uint32_t *saida)`: Identifica o menor valor armazenado estritamente maior que $x$. Retorna `true` se encontrar e grava o valor no ponteiro `saida`.
* `PredecessorVEB(vEB* V, uint32_t x, uint32_t w, uint32_t *saida)`: Identifica o maior valor armazenado estritamente menor que $x$. Retorna `true` se encontrar e grava o valor no ponteiro `saida`.
* `ImprimirVEB(vEB* V, uint32_t w)`: Imprime o estado da estrutura de dados revelando o `min` global e itera de forma linear pelos elementos nos clusters ativados utilizando a própria função sucessor.

---

## 4. Escolhas Arquiteturais

* **Alocação Contígua (`No*`):** A tabela hash utiliza um vetor direto de estruturas em vez de um vetor de ponteiros (`No`). Isso garante que os nós fiquem vizinhos na memória, maximizando a localidade de cache da CPU durante a sondagem linear e agilizando a liberação de memória com um único `free`.
* **Hash 5-Independente ($P = 65537$):** O uso de um polinômio de grau 4 garante o caos matemático necessário para mitigar o problema de agrupamentos da sondagem linear, assegurando tempo médio constante. O primo foi ajustado para ser maior que 65536, evitando colisões matemáticas prematuras no mapeamento dos clusters.
* **uint32_t para Valores e Índices:** A escolha de `uint32_t` para representar os valores e índices foi feita para garantir compatibilidade e eficiência em sistemas de 32 bits, além de que o sinal não é necessário para a representação dos dados.
* **Impressao de Árvore Vazia:** Quando a árvore vEB estiver vazia, o programa imprime "\n" para indicar claramente a ausência de elementos, em vez de não imprimir nada, facilitando a leitura e interpretação dos resultados.

---

## 5. Arquivos onde as estruturas e funções se encontram

Para simplificar a compilação e evitar dependências complexas em sistemas de teste, o código-fonte foi centralizado:

* **`main.c`**: Contém todo o código-fonte (algoritmos da tabela hash e da árvore van Emde Boas e o ponto de entrada `main()`). Esse arquivo concentra toda a lógica de tratamento de colisões e gerência de espaço linear, assim como faz a leitura e formatação do arquivo `.txt` recebido por argumento linha por linha (`INC`, `REM`, `SUC`, `PRE`, `IMP`).
* **`makefile`**: Script responsável pela automação, permitindo realizar o *build* facilmente no compilador GCC e testar com rapidez os arquivos de entrada e saída.

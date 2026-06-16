# Árvore van Emde Boas com Espaço Linear

Este repositório contém a implementação de uma Árvore van Emde Boas. A estrutura foi desenvolvida para suportar um tamanho de palavra de 32 bits e utiliza uma abordagem de espaço linear no número de elementos, implementada através de uma Tabela Hash com sondagem linear e redimensionamento dinâmico (*table doubling* e *table halving*).

---

## 1. Dados do(a) Aluno(a)

* **Nome:** DENISE RÍSIA ANDRADE CARVALHO
* **Matrícula:** 603124

---

## 2. Linguagem de Programação Usada

* **Linguagem:** C
* **Versão utilizada/testada:** GCC 15.2.0

---

## 3. Como rodar o seu programa

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

## 4. Descrição das Funções e Estruturas

A árvore van Emde Boas foi implementada de forma a otimizar o uso de memória (espaço linear $O(n)$) utilizando uma tabela de dispersão interna em cada nível para armazenar apenas os clusters não vazios.

### Estruturas Base

* `struct No`: Representa um item na tabela hash. Contém a `chave` (índice do cluster), um ponteiro `valor` (que aponta para a subárvore vEB daquele cluster) e uma flag booleana `deletado` para exclusão preguiçosa.
* `struct TabelaHash`: Estrutura de endereçamento aberto que gerencia os clusters. Contém o tamanho atual (`tam`), quantidade de itens armazenados (`cont`), o vetor de instâncias `tabela`, e os coeficientes (`a1` a `a5`) utilizados na função matemática de dispersão 5-independente.
* `struct van_emde_boas` (vEB): A estrutura central. Armazena os valores mínimo e máximo do conjunto (`min` e `max`), a tabela hash de filhos (`clusters`) e um ponteiro para a árvore de agrupamentos não vazios (`resumo`).

### Funções da Tabela Hash

* `IniciarTabela(TabelaHash *T, int32_t tam)`: Aloca o vetor de forma contígua e gera os 5 coeficientes aleatórios necessários para o hash 5-independente.
* `MapearChave(TabelaHash *T, int32_t chave)`: Função hash 5-independente. Calcula um polinômio modular de grau 4 para garantir espalhamento uniforme e busca em tempo esperado de $O(1)$.
* `RedimensionarTabela`, `DobrarTabela`, `DividirTabela`: Funções responsáveis pela realocação dinâmica de memória. Dobram o vetor ao ultrapassar 50% de ocupação e cortam pela metade ao cair abaixo de 25%, garantindo a exigência de espaço de memória $O(n)$.
* `IncluirChave` e `IncluirChaveInterno`: Inserem um novo item na tabela resolvendo colisões via sondagem linear.
* `EncontrarChave`: Realiza a busca de uma chave na tabela.
* `DeletarChave`: Realiza a remoção lógica de um cluster sinalizando a flag `deletado`.

### Funções Auxiliares

* `get_c(int64_t x)` e `get_i(int64_t x)`: Extraem a porção superior (índice do cluster) e a porção inferior (posição dentro do cluster) de uma palavra de 32 bits.
* `tupla(int64_t c, int64_t i)`: Junta a porção superior e inferior para reconstruir a chave original.

### Funções Principais (Árvore vEB)

* `Iniciar()`: Aloca e inicializa uma nova árvore van Emde Boas, setando os limites iniciais e inicializando a tabela hash de clusters.
* `Incluir(vEB* V, int64_t x)`: Insere um número inteiro estritamente dentro da sua posição correspondente na hierarquia, instanciando novos agrupamentos recursivamente se necessário.
* `Remover(vEB* V, int64_t x)`: Realiza a exclusão do nó solicitado, atualizando os limites `min` e `max` adequadamente e limpando clusters vazios em níveis inferiores.
* `Sucessor(vEB* V, int64_t x)`: Identifica o menor valor armazenado estritamente maior que $x$. Retorna `+INF` se não houver nenhum.
* `Predecessor(vEB* V, int64_t x)`: Identifica o maior valor armazenado estritamente menor que $x$. Retorna `-INF` se não houver nenhum.
* `Imprimir(vEB* V)`: Imprime o estado do nível mais alto da estrutura de dados, revelando o `min` global e todos os elementos nos clusters não vazios de acordo com a especificação requerida.

---

## 5. Arquivos onde as estruturas e funções se encontram

Para simplificar a compilação e evitar dependências complexas em sistemas de teste, o código-fonte foi centralizado:

* **`main.c`**: Contém todo o código-fonte (estruturas genéricas de Hash, algoritmos da árvore van Emde Boas e o ponto de entrada `main()`). Esse arquivo concentra toda a lógica de tratamento de colisões e gerência de espaço linear, assim como faz a leitura e formatação do arquivo `.txt` recebido por argumento linha por linha (`INC`, `REM`, `SUC`, `PRE`, `IMP`).
* **`makefile`**: Script responsável pela automação, permitindo realizar o *build* facilmente no compilador GCC e testar com rapidez os arquivos de entrada e saída.
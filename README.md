# LeftRecursionElimination

Projeto com a implementação de um algoritmo que elimina recursões à esquerda diretas e indiretas.

A entrada do programa é um arquivo de texto com uma grámatica para análise, seguindo o seguinte padrão:

*X-aXb|e*


  * Todos os símbolos devem ter somente **um** charactere
  * As letras **maiúsculas** representam estados **Não-Terminais**
  * As letras **minúsculas** representam estados **Terminais**
  * Pode ser utilizado no máximo 26 símbolos como **Não-Terminais** - um para cada letra maiúscula do alfabeto
  * A letra **S** representa o símbolo de início da gramática
  * A letra **e** represeta nosso "ε"(*epsilon*), portando lembrar de **não** utilizá-la para representar um terminal
  * $ representa nosso símbolo final
  * O símbolo "|" (*pipe*) separa produções
      * Pode-se optar pela não utilização do "pipe", basta escrever a produção em uma linha separada
      

Lembrando que a existência de produções-ε na gramática de entrada não garante a corretude da gramática de saída.
      

Para executar o código:
```sh
$ make
$ ./a.out -f <input file name>
```
Você pode também escrever o resultado em um arquivo:
```sh
$ make
$ ./a.out -f <input file name> -o <result file name>
```

Após os passos acima, será impresso na tela a gramática resultante, sem recursões à esquerda.

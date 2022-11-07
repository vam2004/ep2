# Lista ligadas
## Definições iniciais
- Ligação: Um objeto (A) está ligado a um objeto (B) por (L), se existe uma ligação (L) em (B) que associa (A) a (B).
- Nó: É um objeto que contém a carga útil e ligações para outros nós.
- Indexável: Um objeto é dito indexável, se existe uma função que a associa a cada índice um estado.
- Localmente consistemente indexável: Um objeto indexável é dito localmente consistetemente indexável, se os estados associdados a cada um dos índices se mantêm constantes após serem modificados ou definidos. 
- Globalmente consistemente indexável: Um objeto Localmente indexável é dito globalmente consistetemente indexável, se toda alteração do estado associado a um índice não altera os estados associdados aos outros índice. 
- Lista ligada encadeada (LLE): É uma estrutura indexável que possui uma ou mais ligações a nós.
- Lista ligada duplamente encadeada (LLDE): É uma lista encada que possui exatamente duas ligações para nós.
- Ligação de entrada: É uma ligação que permite o acesso ao nó de entrada, se existe.
- Nós subsequentes: São nós que podem acessado através de um nó de entrada.
- Nó superior: Dado um nó (A), um nó (C) é superior a (A) se, e somente se, (C) é sucessor de (A); ou existe um nó (B) superior a (A) tal que (C) é sucessor de (B).
- Nó inferior: Dado um nó (C), um nó (A) é inferior a (C) se, e somente se, (A) é antecessor de (C); ou existe um nó (B) antecessor a (C) tal que (A) é antecessor de (B).
- Ligação antecessora: Dado um objeto (B), a ligação antecessora é uma ligação (L) tal que se um nó (A) está ligado a (B) por (L), então (A) é o nó antecessor de (B).
- Ligação sucessora: Dado um objeto (B), a ligação sucessora é uma ligação (L) tal que se um nó (A) está ligado a (B) por (L), então (A) é o nó sucessor de (B).
- Nó irregular: Dado um espaço amostral (O) e uma lista enc, então um nó (A) pertencente 
## Garantias de uma LLDE 
- Consistência de ligação cruzada: Ocorre quando dado um nó (B), então:
	+ Se um nó (A) está ligado a (B) por uma ligação antecessora, então (B) estará ligado a (A) por uma ligação sucessora.
	+ Se um nó (C) está ligado a (B) por uma ligação sucessora, então (B) estará ligado a (C) por uma ligação antecessora.
- Não circularidade das ligações: Seja (A) e (B) quaisquer dois nós. Dessa forma, isto ocorre, quando (A) é um nó superior a (B) se, e somente se, (A) não é um nó inferior a (B).
- Não-circularidade das ligações de entrada: Isto ocorre quando dado uma LLDE (L) e quaisquer duas ligações (A) e (B) de entrada pertencente a (L), então ocorre a não circularidade das ligações (A) e (B).
- Dupla anulabilidade das ligações de entrada: Seja uma LLDE (L) com duas ligações de entradas (A) e (B). Dessa forma, isto ocorre quando, se existe um nó (a) ligado a (L) por (A) se, e somente se, existe um nó (b) ligado a (L) por (A).
- Regularidade estrita dos nós: ocorre quando dado um espaço amostral tal que todo nó inserido na lista pertence ao espaço amostral, então todo nó pertencente a lista pertence ao espaço amostral.
- Anulidade superior de um nó de entrada: Ocorre quando dado um nó (A) de entrada, então (A) não possui uma nó sucessor; ou existe um nó superior (B) tal que (B) não possui um nó sucessor.
- Anulidade inferior de um nó de entrada: Ocorre quando dado um nó (A) de entrada, então (A) não possui uma nó inferior; ou existe um nó inferior (B) tal que (B) não possui um nó inferior.
- Anulidade do nó de entrada: Ocorre quando dado um objeto (O), uma ligação de entrada (A) e um nó (a) de entrada ligado a (O) por (A), então:
	+ Se (A) é uma ligação antecessora, então ocorre a anulidade inferior de (a).
	+ Se (A) é uma ligação sucessora, então ocorre a anulidade superior de (a).
- Anulidade dos nós de entradas: ocorre quando dado um objeto, então ocorre a anulidade do nó de entrada para todo nó de entrada desse objeto.


### Projeto 2 - Acesso informático aos Quartos de Banho
##### Objetivos do trabalho:

 - criar programas multithread;
 - promover a intercomunicação entre processos através de canais com nome (named pipes ou FIFOs);
 - evitar conflitos entre entidades concorrentes, por via de mecanismos de sincronização.

##### Descrição Geral:
São enviados pedidos de acesso por intermédio de um processo multithread U, relativo aos cliente, que serão lidos por um servidor, Q. O tempo de duração de cada cliente é gerado aleatoriamente, podendo tomar valores de 300 a 1199 ms. O servidor recebe esses pedidos e envia uma resposta para o cliente, permitindo-o, ou não, utlilzar o quarto de banho. O utilizador pode limitar o número de lugares no quarto de banho, assim como o número de threads a atender os pedidos

##### Registo das operações:

 - IWANT: cliente faz pedido inicial, ou seja, sempre que U1 escreve no fifo público;
 - RECVD: servidor acusa receção de pedido, isto é, sempre que Q1 lê do fifo público a mensagem que U1 escreveu e cria a thread responsável pelo pedido;
 - ENTER: servidor diz que aceitou pedido;
 - IAMIN: cliente acusa a utilização do Quarto de Banho, ou seja, sempre que U1 lê do fifo privado a mensagem de Q1, quando esta permite a utilização do quarto de banho;
 - TIMUP: servidor diz que terminou o tempo de utilização;
 - 2LATE: servidor rejeita pedido por Quarto de Banho já ter encerrado;
 - CLOSD: cliente acusa informação de que o Quarto de Banho está fechado;
 - FAILD: cliente já não consegue receber resposta do servidor;
 - GAVUP: servidor já não consegue responder a pedido porque FIFO privado do cliente fechou.


##### Pormenores de Implementação Relevantes

 - getElapsedTime(): função presente no ficheiro timer.c, utilizada para calcular o tempo decorrido desde o ínicio do programa até ao instante em causa. Esta função assume um papel relevante na implementação do projeto, uma vez que controla o espaço de tempo em que são geradas threads. Para além disso, verifica também se o tempo de utilização de cada cliente excede o tempo limite do servidor.
 - após o tempo de duração definido pelo utilizador para os dois programas, ambos terminam deixando as respetivas threads concluir a sua função.

## Trabalho Prático Individual 02: Simulação Aplicativo de Streaming em C

**Resumo:**

Este trabalho prático pretende desenvolver uma aplicação console que simule o funcionamento básico de aplicativos de streaming. A comunicação entre o cliente e o servidor será realizada por meio de uma conexão UDP, com o servidor sendo capaz de lidar com múltiplas conexões de clientes utilizando multithreading.

**Requisitos:**

* **Linguagem:** C
* **Tipo de conexão:** TCP
* **Plataforma:** Linux
* **IPv4 e IPv6:** O programa deve funcionar com ambos os protocolos.

**Funcionalidades:**

* **Servidor:**
    * Aceita conexões UDP de clientes e cria uma nova thread para cada cliente conectado.
    * Contém 3 listas de frases de filmes enviadas aos clientes com base em sua seleção.
    * A cada 3 segundos, envia uma frase do filme selecionado pelo respectivo cliente para cada cliente conectado.
    * Após enviar todas as frases, a conexão é desfeita para aquele cliente.
    * A cada 4 segundos, exibe na tela a quantidade de clientes conectados.
* **Cliente:**
    * Apresenta um menu para escolha de 3 filmes.
    * Conecta-se ao servidor utilizando UDP após a seleção de um filme, informando a opção selecionada (1, 2 ou 3).
    * Exibe as frases recebidas do servidor no console.
    * Retorna ao menu inicial após receber todas as frases.
    * Encerra o programa se a opção "Sair" for selecionada.

**Entrega:**

* Código fonte em C (arquivos separados para cliente e servidor)
* Makefile para compilação
* Documentação em PDF (pelo menos 3 páginas) contendo:
    * Nome completo e matrícula do aluno
    * Explicação detalhada do código (com foco nas partes de conexão)
    * Prints do programa em funcionamento (com descrições)

**Instruções de compilação:**

Execute o comando `make` na raiz do projeto. Isso irá gerar os binários `client` e `server` dentro da pasta `bin`.

**Execução:**

* **Servidor:** `./bin/server [ipv4|ipv6] [porta]`
* **Cliente:** `./bin/client [ipv4|ipv6] [ip_servidor] [porta]`

**Observações:**

* Comente todas as linhas de código relacionadas à conexão.
* Utilize a metodologia de divisão e conquista para simplificar o desenvolvimento.

**Pontuação:**
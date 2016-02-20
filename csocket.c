/* Exemplo de uso de sockets
 *
 * Para entender o uso da struct sddrinfo, bind() e connect(), ler:
 * man bind
 * man connect
 * man getaddrinfo
 *
 * FYI:
		struct addrinfo {
               int              ai_flags;
               int              ai_family;
               int              ai_socktype;
               int              ai_protocol;
               socklen_t        ai_addrlen;
               struct sockaddr *ai_addr;
               char            *ai_canonname;
               struct addrinfo *ai_next;
		};
 *
 *
 */

/* socket */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
//#include <fcntl.h>
#include <netdb.h>

/* mostrar erros */
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Variáveis globais */
struct addrinfo *res;
char buff[100];
int sock;

/* Rodar no final do programa */
void rodar_no_fim(void){

	// Fecha socket aberto
	close(sock);

	// Libera memória utilizada na estrutura
	freeaddrinfo(res);

	puts("Fim\n");
}


int linha(int);

int main()
{
	struct addrinfo hints;

	/* Definindo as configurações do socket */
	{
		// Zera a estrutura
		bzero(&hints, sizeof (struct addrinfo) );

		// AF_UNSPEC - funciona para IPv4 e IPv6
		// AF_INET   - somente IPv4
		// AF_INET6  - somente IPv6
		hints.ai_family = AF_UNSPEC;

		// SOCK_STREAM - socket TCP
		// SOCK_DGRAM  - socket UDP
		hints.ai_socktype = SOCK_STREAM;

		// Se o primeiro parâmetro passado para getaddrinfo() for NULL
		// e a flag AI_PASSIVE for usada, o socket retornado estará
		// preparado para ser usado em bind() e accept(). O endereço
		// retornado será INADDR_ANY para IPv4.
		hints.ai_flags = AI_PASSIVE;
	}


	/* Obtendo estrutura contendo configurações para o socket */
	{
		int status;
		if(status = getaddrinfo(NULL, "80", &hints, &res) != 0)
		{
			/* Getaddrinfo tem seu próprio esquema de erros */
			fprintf(stderr, "getaddrinfo() error: %s\n", gai_strerror(status));
			exit(EXIT_FAILURE);
		}
	}

	/* Obtendo o novo socket à partir das configurações anteriores */
	if( (sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
	{
		perror("socket() error");
		exit(EXIT_FAILURE);
	}


	/* Registrando função para rodar ao fim */
	atexit(rodar_no_fim);


	/* Registrando o socket na porta configurada anteriormente */
	if(bind(sock, res->ai_addr, res->ai_addrlen) != 0)
	{
		perror("bind() error");
		exit(EXIT_FAILURE);
	}

	/* Marca o socket criado como passivo (aguardando conexões) */
	listen(sock, 10);

	int prosock, temp;

	while(1){

		/* Cria file descriptor para socket aberto */
		prosock = accept(sock, NULL, NULL);

		/* Zera buffer, lê primeira linha e printa */
		bzero(buff, 99);
		printf("%d: ", linha(prosock));
		puts(buff);

puts("Reading...");
		while(linha(prosock)!=0);
puts("done!\n");

		/* Se a requisição não for para "localhost/", avisa */
		if( strstr(buff, "GET / ")==NULL ){
			write(prosock, "HTTP/1.1 200 OK\r\n", 17);
			write(prosock, "Content-length: 41\r\n", 20);
			write(prosock, "Content-Type: text/html\r\n\r\n", 27);
			write(prosock, "<html><body>Not implemented</body></html>",41);
		}else{
			write(prosock, "HTTP/1.1 200 OK\r\n", 17);
			write(prosock, "Content-length: 46\r\n", 20);
			write(prosock, "Content-Type: text/html\r\n\r\n", 27);
			write(prosock, "<html><body><h2>Hello world</h2></body></html>",46);
		}

		/* Fecha o socket */
		close(prosock);
	}


	return EXIT_SUCCESS;
}

int linha(int sock)
{
	int keep = 0, i=0;
	char c;

	/* Roda enquanto não encontrar um fim de linha ou o fim do buffer */
	while(keep==0 && i<=98){

		/* Caso não hajam mais caracteres a serem lidos ou haja erro */
		if(recv(sock, &c, 1, 0) <= 0){
			keep=2;
		}else{

			/* Enquanto não encontrar CR/LF, copia os caracteres */
			if(c!='\n' && c!='\r'){
				buff[i++] = c;
			}else{

			/* Se encontrar CR/LF, põe fim de linha no buffer e sai */
				buff[i] = '\n';
				keep=1;
			}
		}
	}
	/* Caso tenha chegado ao final do buffer */
	if(i==99 && keep==0)
		buff[99] = '\0';

	/* Caso não hajam mais caracteres */
	if(keep==2)
		buff[i]=='\0';

	return i;
}

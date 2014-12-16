#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "hacking.h"

#define PORT 7890	// the port users will be connecting to

int main(void) {
	int sockfd, conn_sockfd;  // listen on sock_fd, new connection on conn_sockfd
	struct sockaddr_in host_addr, client_addr;	// my address information
	socklen_t sin_size;
	int recv_length=1, yes=1;
	char buffer[1024];

	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		fatal("in socket");

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		fatal("setting socket option SO_REUSEADDR");
	
	host_addr.sin_family = AF_INET;		 // host byte order
	host_addr.sin_port = htons(PORT);	 // short, network byte order
	host_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
	memset(&(host_addr.sin_zero), '\0', 8); // zero the rest of the struct

	if (bind(sockfd, (struct sockaddr *)&host_addr, sizeof(struct sockaddr)) == -1)
		fatal("binding to socket");

	if (listen(sockfd, 5) == -1)
		fatal("listening on socket");

	while(1) {    // Accept loop
		sin_size = sizeof(struct sockaddr_in);
		conn_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
		if(conn_sockfd == -1)
			fatal("accepting connection");
		printf("server: got connection from %s port %d\n",inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		send(conn_sockfd, "Hello World!\n", 13, 0);
		recv_length = recv(conn_sockfd, &buffer, 1024, 0);
		recv(conn_sockfd, buffer, 100, 0);
		while(recv_length > 0) {
			printf("RECV: %d bytes\n", recv_length);
			dump((unsigned char*) buffer, recv_length);
			recv_length = recv(conn_sockfd, &buffer, 1024, 0);
		}
	//implement message.c
		close(conn_sockfd);
	}
	return 0;
}

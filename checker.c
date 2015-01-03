/********************************************************************************************************
* file: checker.c																					 	*
* date: /1/1/15																							*
* author: Devin Ercolano, David Weinman																	*
* sources: Hacking the Art of Exploitation by Jon Erickson												*
*																										*
* Details: This program is a server recieving submissions that											*
* conform to a domain specific language for the red and blue teams of our scenario.						*
*																										*
* Simple Example Domain Specific language:																*
*																										*
*			CHALLENGE_SUBMIT: RED1																		*
*				BEGIN																					*
*			… submission here ...																		*
*				END																						*
*																										*
*																										*
* * Where RED1 is teamcolor and chall_num																*
*																										*
*																										*
*																										*
*********************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include<sys/stat.h>
#include "hacking.h"

#define ErrorMessage( Str ) fprintf( stderr, "%s\n", Str)

#define MAXFILENAMELENGTH 20
#define MAXFILESIZE 250
#define MAXBUFFERSIZE 1024

#define SUBMISSIONPATH "/home/betsythecat/submission_location/"

#define PORT 7890	// the port users will be connecting to

int chartoint(char c){
	return (int) (c-48);
}

//This function opens a new file (submission), writes the user's submission (buffer) to it, and closes the file
void submissionsaver(const char * submission, char * buffer ) {
	int fd; //file descriptor
	char *datafile;
	datafile = (char*) ec_malloc(MAXFILESIZE);
	
	snprintf(datafile, 250, "%s%s", SUBMISSIONPATH, submission);
	// Opening the file
	fd = open(datafile, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
	if(fd == -1)
		fatal("in main() while opening file");
	// Writing data
	if(write(fd, buffer, strlen(buffer)) == -1)
		fatal("in main() while writing buffer to file");
	// Closing file
	if(close(fd) == -1)
		fatal("in main() while closing file");

	printf("Submission has been saved.\n");
	free(datafile);
}

void fatal(char *); // a function for fatal errors
void *ec_malloc(unsigned int); // an errorchecked malloc() wrapper

int main(void) {
	int sockfd, conn_sockfd, i;  // listen on sock_fd, new connection on conn_sockfd, array index
	struct sockaddr_in host_addr, client_addr;	// my address information
	socklen_t sin_size;
	int recv_length=1, yes=1;
	char *buffer;
	const char * redchallenges[] = {
				"sql_version", "vandalism", "ssn_recon",
				"peresistent_access", "wrench_in_the_gears", "logout_user"
			     };
	const char * bluechallenges[] = {
				"ftp_server", "stan", "plugin",
				"forgot_my_pass", "compress_stan", "pro_tools"
			      };

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

		printf("server: got connection from %s port %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		buffer = (char *)ec_malloc(MAXBUFFERSIZE);
		memset(buffer, 0, MAXBUFFERSIZE);

		recv_length = recv(conn_sockfd, buffer, 1024, 0);
		printf("[DB] buffer: %s\n", buffer);
		char *teamcolor, *submission_num;
		while(recv_length > 0) {	
			char chall_num = '1';
			for(i = 0; i < 5; i++)
				{
				submission_num = strchr(buffer, chall_num);
				//check if strchr found a match
				//if match found then break
				if (submission_num != NULL)
					break;
				chall_num++;
			}

			//search for team color in buffer to determine which list of team challenges to use file submission (where the file is saved)
			teamcolor = strstr(buffer,"red");

			if (teamcolor == NULL)
				teamcolor = strstr(buffer, "blue");
			
			if (teamcolor == NULL) {
				ErrorMessage("team color not found");
				break;
			}
		
			if (0 == strncmp(teamcolor, "red", 3))
				submissionsaver( redchallenges[ chartoint(chall_num) - 1 ], buffer );
			else if (0 == strncmp(teamcolor, "blue", 4))
				submissionsaver( bluechallenges[ chartoint(chall_num ) - 1 ], buffer );
			
			else {	/*print error message from invlaid name*/
				ErrorMessage("Challenge name not found");
				break;
			}

			free(buffer);
			buffer = (char *)ec_malloc(MAXBUFFERSIZE);
			memset(buffer, 0, MAXBUFFERSIZE);
			recv_length = 0;
		}
		free(buffer);
		close(conn_sockfd);
	}
	return 0;
}

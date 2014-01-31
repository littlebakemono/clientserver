/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3511"  // the port users will be connecting to
#define MAXDATASIZE 100
#define BACKLOG 10	 // how many pending connections queue will hold
#define MAXBUFFERSIZE 1024

const int ERROR = -1;
const int FLAGS = 0;

char list_command[] = "list";
char dl_command[] = "download";
char dsp_command[] = "display";
char chk_command[] = "check";
char help_command[] = "help";

void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void exec_commands(char *data_received);
void error(char *s);

int main(void)
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("Server: Waiting for connections...\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("Server: Got connection from %s\n", s);

      char buf[MAXDATASIZE];
	   
   	
      if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener
			if (send(new_fd, "Connection established!", 23, 0) == -1)
				perror("send");
       
         int in[2], out[2], pid; //Two pipes in and out
         
         if(pipe(in)<0){
            error("Pipe in");
         }
         if(pipe(out)<0){
            error("Pipe out");
         }
         
          //Close stdin, stdout, stderr
         close(0);
         close(1);
         close(2);

         dup2(in[0],0);
         dup2(out[1],1);
         dup2(out[1],2);
               
         close(in[1]);
         close(out[0]);
         	
         int waiting = 1;
         int numbytes;
         while(waiting){
           numbytes = recv(new_fd, buf, MAXDATASIZE-1, FLAGS);
           if(numbytes == ERROR){
              perror("recv");
              waiting = 0;
           } else if (numbytes == 0) {
              printf("Lost connection\n");
              waiting = 0;
           } else {
              buf[numbytes] = '\0';
              printf("Data received: '%s'\n", buf);
              exec_commands(buf);
           }
         }
         close(new_fd);
			exit(0);
		} else {
      }
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}

void exec_commands(char *data_received)
{
   if(strcmp(data_received, list_command) == 0){
      exec("");
   }
}

void error(char *s)
{
   perror(s);
   exit(1);
}

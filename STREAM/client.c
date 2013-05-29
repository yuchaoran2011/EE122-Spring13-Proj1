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

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10	 // Maximum number of pending connections queue is able to accomodate



void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}


int uniform_distribution(int rangeLow, int rangeHigh) {
    double myRand = rand()/(1.0 + RAND_MAX); 
    int range = rangeHigh - rangeLow + 1;
    int myRand_scaled = (myRand * range) + rangeLow;
    return myRand_scaled;
}



// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}




int main(void)
{
	int listen_on_socket, client_socket;  // listen on sock_fd, new connection on client_socket
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1, rv, rate = 5;
	char s[INET6_ADDRSTRLEN];
	struct timespec sleepTime = {5, 0};//10000000

	char * IP = "10.10.66.90";



	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; 




	if ((rv = getaddrinfo(IP, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through the linked list servinfo returned by getaddrinfo, stop as soon as the binding succeeds.
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((listen_on_socket = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (setsockopt(listen_on_socket, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(listen_on_socket, p->ai_addr, p->ai_addrlen) == -1) {
			close(listen_on_socket);
			perror("client: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "client: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); // release servinfo when we are done using it.

	if (listen(listen_on_socket, BACKLOG) == -1) {
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


	printf("client: waiting for connections...\n");



	// Accept
	while(1) {  
		sin_size = sizeof their_addr;
		client_socket = accept(listen_on_socket, (struct sockaddr *)&their_addr, &sin_size);
		if (client_socket == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("client: got connection from %s\n", s);

		if (!fork()) { // this is the child process
			close(listen_on_socket); // child doesn't need the listener
			// if (send(client_socket, "Hello, world!", 13, 0) == -1)
			// 	perror("send");


/////////////////////////////////////////////////////////////

			FILE *input = fopen("Schedule.png", "rb");
			int a;
			while(fread(&a, sizeof(int), 1, input) && !feof(input)) {
				send(client_socket, &a, sizeof(a),0);
				sleepTime.tv_sec = rate;
				//nanosleep(&sleepTime, NULL);
				rate = uniform_distribution(0, 10);
				printf("%d\n", rate);
			}

			a = -1;
			send(client_socket, &a, sizeof(a), 0);
			fclose(input);
			char ackn[100];
			int numbytes;
			if ((numbytes = recv(client_socket, ackn, sizeof(ackn)-1, 0)) == -1) {
				perror("receiving error.");
			}
			ackn[numbytes] = '\0';
			printf("%s\n", ackn);
			

/////////////////////////////////////////////////////////////
			close(client_socket);
			exit(0);
		}
		close(client_socket);  // parent doesn't need this
	}

	return 0;
}

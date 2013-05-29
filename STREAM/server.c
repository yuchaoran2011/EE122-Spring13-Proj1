#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT "3490" // the port server will be connecting to 
#define MAXDATASIZE 100


int blink_counter = 0;



// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}



void *threadfunction1()
{
	int server_socket, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	char * IP = "10.10.66.90";

	/*
	if (argc != 2) {
	    fprintf(stderr,"usage: server hostname\n");
	    exit(1);
	}
	*/

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;


	
	if ((rv = getaddrinfo(IP, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	



	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((server_socket = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (connect(server_socket, p->ai_addr, p->ai_addrlen) == -1) {
			close(server_socket);
			perror("server: connect");
			continue;
		}

		break;
	}



	if (p == NULL) {
		fprintf(stderr, "server: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("server: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	

/////////////////////////////////////////////////////////////
	FILE *output;
	char fname[] = "Schedule_recv.png";
	output = fopen(fname, "wb");
	int a;
	printf("Start receiving...\n");
	while(recv(server_socket, &a, sizeof(a),0) && a != -1){
		blink_counter += 1;
		fwrite(&a, sizeof(int), 1, output);
	}
	printf("Done.\n");
	fclose(output);
	char msg[] = "Received.";
	if (fopen(fname, "rb"))
		send(server_socket, msg, sizeof(msg)-1, 0);
	else printf("Receive error\n");

/////////////////////////////////////////////////////////////

	close(server_socket);

	return 0;
}




void *threadfunction2(){
	int prev = 0;
	int current = -1;
	struct timespec sleepTime = {5, 0};//10000000
	

	while (1) {
		if (blink_counter!=0) 
			current = blink_counter;
		if (current-prev >= 5) {
			nanosleep(&sleepTime, NULL);
			printf("Blink!\n");
			prev = prev + 1;
		} 
	}
}





int main(int argc, char *argv[]) {

	pthread_t threadID1, threadID2;
	void *exit_status;

	pthread_create(&threadID1, NULL, threadfunction1, NULL);
	pthread_create(&threadID2, NULL, threadfunction2, NULL);
	pthread_join(threadID1, &exit_status);
	pthread_join(threadID2, &exit_status);
	
	return 0;
}
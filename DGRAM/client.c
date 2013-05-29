/*
 Datagram client
 The client sends a file to server, which sends back information about lost packets to the client and requests retransmission
 of lost packets. 
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CAPACITY 301
#define PAYLOAD_SIZE 200


int main(int argc, char *argv[]){
	struct addrinfo hints, hints2;
	struct addrinfo *serviceinfo, *serviceinfo2;
	int sockfd, sockfd2, bytes_sent;
	FILE *fp = fopen("122 Proj1 Report.txt", "r");
	char port[6] = "40010", port2[6] = "40013";
	char txt[PAYLOAD_SIZE];

   char ack[CAPACITY];

	struct timespec sleepTime = {0, 10000000};







   /////////////////////////////////////////////////////////////////////////////////////////////////////////////

   /*  Get the address info of the file receiver and creat a socket through which the client send the file packet by packet  */
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	int i = 1,j;

   struct sockaddr_in from;
   int from_len;


	if (getaddrinfo("10.10.67.155", port, &hints, &serviceinfo) == -1 || serviceinfo == NULL) {
		printf("getaddrinfo error!\n");
		exit(1);
	}


	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1){
		printf("Error: Failed to create sending socket!\n");
		exit(1);
	}

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////












   /////////////////////////////////////////////////////////////////////////////////////////////////////////////

   /* Get the address of client itself and create a socket at which the client listens for incomming packets sent back 
      by the server. Because sockfd2 is a "listening" socket, we need to bind it.  */
   memset(&hints2, 0, sizeof hints);
   hints2.ai_family = AF_INET;
   hints2.ai_socktype = SOCK_DGRAM;
   hints2.ai_flags = AI_PASSIVE;

   if (getaddrinfo("10.10.66.108", port2, &hints2, &serviceinfo2) == -1 || serviceinfo2 == NULL) {
      printf("getaddrinfo error!\n");
      exit(1);
   }

   if ((sockfd2 = socket(PF_INET, SOCK_DGRAM, 0)) == -1){
      printf("Error: Failed to create listening socket!\n");
      exit(1);
   }

   if (bind(sockfd2, serviceinfo2->ai_addr, serviceinfo2->ai_addrlen) == -1) {
      printf("Binding failed!\n");
      printf("%d\n", errno);
      exit(1);
   }

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////









   /////////////////////////////////////////////////////////////////////////////////////////////////////////////

   /*  Compute the size of file that client is going to send using standard file operations  */
	long size;
	if (fp) {
   	fseek(fp, 0, SEEK_END);
   	size = ftell(fp);
   	fclose(fp);
   }
   printf("%ld\n", size);
   close(fp);

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////







   	
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////
   struct timeval now;
   int rc;
   /*
   rc = gettimeofday(&now, NULL);
   if(rc == 0) {
      printf("gettimeofday() successful.\n");
      printf("time = %u.%06u\n", now.tv_sec, now.tv_usec);
   }
   else {
      printf("gettimeofday() failed, errno = %d\n", errno);
      return -1;
   }*/



  	fp = fopen("122 Proj1 Report.txt", "r");   // File open the file for reading
   while (i*PAYLOAD_SIZE < size) {

      for (j=0; j<PAYLOAD_SIZE; j++) 
   		txt[j] = fgetc(fp);

   	txt[PAYLOAD_SIZE] = (i-1) / 256;     // txt is our packet. Its last two elements represent the packet's unique 
   	txt[PAYLOAD_SIZE+1] = (i-1) % 256;    // ID number. 


   	bytes_sent = sendto(sockfd, txt, PAYLOAD_SIZE+2, 0, serviceinfo->ai_addr, serviceinfo->ai_addrlen);

      rc = gettimeofday(&now, NULL);
      if(rc == 0) {
         //printf("gettimeofday() successful.\n");
         printf(".%08u\n", now.tv_usec);
      }
      else {
         printf("gettimeofday() failed, errno = %d\n", errno);
         return -1;
       }
   	nanosleep(&sleepTime, NULL);

      i += 1;
   }



   nanosleep(&sleepTime, NULL);

   if ((i-1)*PAYLOAD_SIZE < size) {
      for (j=0; j<size-(i-1)*PAYLOAD_SIZE; j++)
         txt[j] = fgetc(fp); 

   	printf("%d\n", (i-1) / 256);
   	txt[PAYLOAD_SIZE] = (i-1) / 256;
   	printf("%d\n", (i-1) % 256);
   	txt[PAYLOAD_SIZE+1] = (i-1) % 256;
   	bytes_sent = sendto(sockfd,txt, PAYLOAD_SIZE+2, 0, serviceinfo->ai_addr, serviceinfo->ai_addrlen);

      printf("Done with transmission. Waiting for ACK\n");



      from_len = sizeof(struct sockaddr_in);
      if (recvfrom(sockfd2, ack, CAPACITY, 0, &from, &from_len) == -1)
         printf("Failed to receive ACK!\n");


      for (i=0; i<CAPACITY; i++)
         printf("%d\n", (unsigned char)ack[i]);

   }




   int pos, k;
   int num_of_erroneous_packets = (unsigned char)ack[0];


   if (num_of_erroneous_packets != 0) {

      for (k=0; k<50*num_of_erroneous_packets; k++) {
         for (j=1; j<=num_of_erroneous_packets; j+=1) {
         
            pos = PAYLOAD_SIZE * ((unsigned char)ack[(j-1)*2+1] * 256 + (unsigned char)ack[(j-1)*2+2]);
            fseek(fp, pos, SEEK_SET);
            for (i=0; i<4129; i++)
               txt[i] = fgetc(fp);
            txt[PAYLOAD_SIZE] = (unsigned char)ack[(j-1)*2+1];
            txt[PAYLOAD_SIZE+1] = (unsigned char)ack[(j-1)*2+2];
            bytes_sent = sendto(sockfd, txt, PAYLOAD_SIZE+2, 0, serviceinfo->ai_addr, serviceinfo->ai_addrlen);
            printf("%d\n", bytes_sent);
            if (bytes_sent == -1) {
               printf("Resending failed!\n");
               exit(1);
            }
            nanosleep(&sleepTime, NULL);
            fclose(fp);
            fp = fopen("122 Proj1 Report.txt", "r");
         }
      }
   }





   fclose(fp);
   close(sockfd);
   close(sockfd2);
}
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SIZE 6488666 
#define CAP 301
#define BACKLOG 10 // how many pending connections queue will hold
#define PAYLOADSIZE 200


int main(int argc, char *argv[]){
	FILE *fp = fopen("bigbig.txt", "w");
	char msg[3] = "ACK";



	struct addrinfo hints;
	struct addrinfo *serviceinfo = NULL;
	struct sockaddr_storage addr;
	struct sockaddr_in from;
	socklen_t addrlen;
	char buf[SIZE];
	unsigned char temp[PAYLOADSIZE+2];
	int from_len, sockfd, new_fd, length, status;
	char port[6] = "40010";
    char port2[6] = "40013";

	int TIME_OUT = 20;
    

	in_addr_t data; 
	struct hostent *hp;

	int i = 0, j = 0, k;

    
	clock_t begin, end;

	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;



	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
		printf("Socket system call failed!\n");
		exit(1);
	}


	if (getaddrinfo("10.10.67.155", port, &hints, &serviceinfo) == -1 || serviceinfo == NULL) {
		printf("getaddrinfo error!\n");
		exit(1);
	}

	// serviceinfo now points to a linked list of 1 or more struct addrinfos


	if (bind(sockfd, serviceinfo->ai_addr, serviceinfo->ai_addrlen) == -1) {
		printf("Binding failed!\n");
		printf("%d\n", errno);
		exit(1);
	}

	/*
	if (listen(sockfd, BACKLOG) == -1) {
		printf("Listening failed!\n");
		exit(1);
	}


	addrlen = sizeof(struct sockaddr_storage);
	status = accept(sockfd, (struct sockaddr *)&addr, &addrlen);
	if (status == -1) {
		printf("Accepting failed!\n");
		exit(1);
	}
	else {
		new_fd = status;
	}
	*/


	
	
	from_len = sizeof(struct sockaddr_in);
	begin = clock();
	end = clock();


	for (k=0; k<SIZE; k++)
		buf[k] = -1;



	while (i < SIZE) {
		/*
		if ((end - begin)/CLOCKS_PER_SEC > TIME_OUT) {
			printf("Time out when i is %d\n", i);
			break;
		}
		*/

		if (recvfrom(sockfd, temp, PAYLOADSIZE+2, 0, &from, &from_len) == -1) {
			printf("Receiving failed!\n");
			exit(1);
		}
		
		/*
		if (i == 0) 
			begin = clock()*100;
		end = clock()*100;

		
		printf("End: %ld\n", end);
		printf("Begin: %ld\n", begin);
		printf("%d\n", CLOCKS_PER_SEC);
		printf("Time in secs: %g\n", ((double)(end-begin))/ CLOCKS_PER_SEC);
		*/


		if ((temp[PAYLOADSIZE]*256+temp[PAYLOADSIZE+1]+1)*PAYLOADSIZE > SIZE) {
			memcpy(buf+(temp[PAYLOADSIZE]*256+temp[PAYLOADSIZE+1])*PAYLOADSIZE, temp, SIZE - (temp[PAYLOADSIZE]*256+temp[PAYLOADSIZE+1])*PAYLOADSIZE);
			i += SIZE-(temp[PAYLOADSIZE]*256+temp[PAYLOADSIZE+1])*PAYLOADSIZE;

			break;
		} 
		else {
			//for (j=0; j<4130; j++) 
			//	printf("%c", temp[j]);
			memcpy(buf+(temp[PAYLOADSIZE]*256+temp[PAYLOADSIZE+1])*PAYLOADSIZE, temp, PAYLOADSIZE);
			//printf("%d\n",temp[4130]);
			//printf("%d\n",temp[4131]);
			i += PAYLOADSIZE;
		}	
	}

	

	int arrayindex = 0;
	while (arrayindex < SIZE) {
        fputc(buf[arrayindex], fp);
        arrayindex += 1;
	}
    fputc('\0',fp); 







	////////////////////////////////////////////////////////////////////SEND BACK ACK START
    printf("Initial Transmission Finished!\n");
    struct addrinfo hints2;
	struct addrinfo *serviceinfo2;
	int sockfd2, bytes_sent, c2;
    
	char txt[CAP];

   

	struct timespec sleepTime = {0, 10000000};


	memset(&hints2, 0, sizeof hints2);
	hints2.ai_family = AF_INET;
	hints2.ai_socktype = SOCK_DGRAM;
	hints2.ai_flags = AI_PASSIVE;


	

   //struct sockaddr_in from2;
   //int from_len2;


   //clock_t start, end;

	if (getaddrinfo("10.10.66.122", port2, &hints2, &serviceinfo2) == -1 || serviceinfo2 == NULL) {
		printf("getaddrinfo error!\n");
		exit(1);
	}


	if ((sockfd2 = socket(PF_INET, SOCK_DGRAM, 0)) == -1){
		printf("Error: Could not get socket.\n");
		exit(1);
	}

	/*
	if (connect(sockfd, serviceinfo->ai_addr, serviceinfo->ai_addrlen) == -1) {
		printf("Connecting failed!\n");
		exit(1);
	}	
	*/

	int txtcounter = 0;

	for (i=0;i<CAP;i++)
		txt[i] = -1;
	
	int TIMES_OF_TRANS;
	k = 0;


	if (SIZE % PAYLOADSIZE != 0)
		TIMES_OF_TRANS = SIZE / PAYLOADSIZE + 1;
	else
		TIMES_OF_TRANS = SIZE / PAYLOADSIZE;

    int haserror = 0;
	for (;k<=TIMES_OF_TRANS-1;k++)
		if (buf[k*PAYLOADSIZE] == -1) {
            haserror = 1;
            printf("%d ", k/256);
            printf("%d Error\n", k%256);
			txt[2*txtcounter+1] =  k/256;
			txt[2*txtcounter+2] =  k%256;
			txtcounter += 1;
		}
    txt[0] = txtcounter;
    if (haserror)
        printf("There is Error!\n");
    else
        printf("perfect transmission\n");


   	bytes_sent = sendto(sockfd2, txt, CAP, 0, serviceinfo2->ai_addr, serviceinfo2->ai_addrlen);
    if (bytes_sent == -1) {
        printf("send ack failure");
    } else {
        printf("Send %d bytes ack!\n",bytes_sent);
        
    }
   	//nanosleep(&sleepTime, NULL);


   
   

////////////////////////////////////////////////////////////////////////////////  SEND BACK ACK END
    
    
////////////////////////////////////////////////////////////////////////////////  START TO REC AGAIN
    
    printf("WHILE LOOP!\n");
    if (haserror)
        printf("Starting to received correction!\n");
    
    //int receivedco[(CAP-1)/2];
    //int z;
    //for (z = 0; z < txtcounter;z++)
    //    receivedco[z]=-1;
    //int receivedcoindex = 0;
    
    
    
    //int l;
    //int hasitornot;
    // accepts 10 rounds of packets  while the sender sends 50 rounds of packets
    txtcounter = txtcounter*10;
    while (haserror && txtcounter >0) {
        
        if (recvfrom(sockfd, temp, PAYLOADSIZE+2, 0, &from, &from_len) == -1) {
            printf("Receiving failed!\n");
            exit(1);
        }
    
    
    /*
     if (i == 0)
     begin = clock()*100;
     end = clock()*100;
     
     
     printf("End: %ld\n", end);
     printf("Begin: %ld\n", begin);
     printf("%d\n", CLOCKS_PER_SEC);
     printf("Time in secs: %g\n", ((double)(end-begin))/ CLOCKS_PER_SEC);
     */
        
        
        if ((temp[PAYLOADSIZE]*256+temp[PAYLOADSIZE+1]+1)*PAYLOADSIZE > SIZE) {
            
            /*
            for (l = 0; l<txtcounter; l++) {
                printf("in the for loop");
                if (receivedco[l]==(temp[4130]*256+temp[4131])*4130) {
                    hasitornot = 1;
                
                    break;
                }
            }
            */
            
            memcpy(buf+(temp[PAYLOADSIZE]*256+temp[PAYLOADSIZE+1])*PAYLOADSIZE, temp, SIZE - (temp[PAYLOADSIZE]*256+temp[PAYLOADSIZE+1])*PAYLOADSIZE);
            txtcounter -= 1;
            printf("%d ",(unsigned char)temp[PAYLOADSIZE]);
            printf("%d Corrected\n",(unsigned char)temp[PAYLOADSIZE+1]);
                        
            
           
            
            
            
        //i += SIZE-(temp[4130]*256+temp[4131])*4130;
        
            
        }
        else {
            //for (j=0; j<4130; j++)
            //	printf("%c", temp[j]);
            /*
            for (l = 0; l<txtcounter; l++) {
                if (receivedco[l]==(temp[4130]*256+temp[4131])*4130) {
                    hasitornot = 1;
                    break;
                }
            }
            */
            
            memcpy(buf+(temp[PAYLOADSIZE]*256+temp[PAYLOADSIZE+1])*PAYLOADSIZE, temp, PAYLOADSIZE);
            txtcounter -= 1;
            printf("%d ",(unsigned char)temp[PAYLOADSIZE]);
            printf("%d Corrected\n",(unsigned char)temp[PAYLOADSIZE+1]);
            //printf("%d\n",temp[4130]);
            //printf("%d\n",temp[4131]);
            //i += 4130;
            
        }
    
    
    
    }
    
    
    
    
//////////////////////////////////////////////////////////////////////////////////////////////// END TO REC AGAIN
    
    
    
    
    
    
    
    

	close(new_fd);
	close(sockfd);
    close(sockfd2);
	fclose(fp);

	freeaddrinfo(serviceinfo);
    freeaddrinfo(serviceinfo2);
}

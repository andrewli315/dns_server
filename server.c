#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>     /* getopt, ssize_t */
#include <pthread.h>
#include "status.h"

#define SIZE_OF_BUFFER 128
typedef struct DNS_DATA
{
	char ip[16];
	char domain[100];
};
DNS_DATA dns_data[1000];
static int index;
void *handle_request(void *socketfd);
int main(void)
{
	int socketfd, clientfd, portno,addrlen;	
	int portno = 12345;
	int ret;
	int *new_sock;
	struct sockaddr_in serv_addr, client_addr;
	memset((char*)&serv_addr,0,sizeof(serv_addr));
	portno = port;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if(bind(socketfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0)
	{
		error("ERROR on binding\n");
		return 1;
	}
	//listen the client
	listen(socketfd,5);
	while(1)
	{
		 = sizeof(client_addr);
		pthread new_thread;
		/* Wait and Accept connection */
		clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &addrlen);
		*new_sock = clientfd;
		printf("connect successfully\n");
		ret = pthread_create(&new_thread,NULL,(void*)new_sock < 0);


	}
	return 0;
}
void *handle_request(void *socketfd)
{
	int sock = *(int*)socketfd;
	int i = 0,j = 0;
	int flag =0;
	size_t size_req,size_resp;
	char request[SIZE_OF_BUFFER];
	char *response = (char*)malloc(SIZE_OF_BUFFER);
	char cmd[5];
	char domain[100];
	char ip[16];


	while((read_size = recv(sock , request , SIZE_OF_BUFFER , 0)) < 0)
	{
		request[read_size] = '\0';
		while(request[i] != '\0')
		{
			if(request[i] == ' ')
			{
				flag++;
				j=0;
			}
			else if(flag == 0 && request[i] != ' ')
			{
				cmd[j++] = request[i++];
				cmd[j] = '\0';
			}
			else if(flag == 1 && request[i] != ' ')
			{
				domain[j++] = request[i++];
				domain[j] = '\0';
			}
			else if(flag == 2 && request[i] != ' ')
			{
				ip[j++] = request[i++];
				ip[j] = '\0';
			}

		}
		if(strcmp(cmd,"SET") == 1)
		{

		}
		else if(strcmp(cmd,"GET") == 1)
		{
			
		}
		if(strcmp(cmd,"INFO") == 1)
		{
			
		}

	}



}
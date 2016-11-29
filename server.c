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
	int ip[4];
	char domain[100];
};
DNS_DATA dns_data[1000];
static int index;
void *handle_request(void *socketfd);
int search_domain(char* ip);
int search_ip(char *domain);
int check_ip_invalid(char *ip);
int check_domain_invalid(char *domain);
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
	char ip[20];



	while((read_size = recv(sock , request , SIZE_OF_BUFFER , 0)) >= 0)
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
			
			if(check_domain_invalid(domain) == 0 && check_ip_invalid(ip) == 0)
			{
				int ip_int[4];
				int i;
				sprintf(ip,"%d.%d.%d.%d",ip_int[0],ip_int[1],ip_int[2],ip_int[3]);//process the response string
				for(i = 0;i<4;i++)
					dns_data[index].ip[i] = ip_int[i];
				strcpy(dns_data[index].domain,domain);

			}
			else
			{
				//bad request
			}



		}
		else if(strcmp(cmd,"GET") == 1)
		{
			if(check_domain_invalid(domain) == 0 && ip == NULL)
			{
				search_ip(domain);//process the response string
			}
			else
			{
				//bad request
			}			
		}
		if(strcmp(cmd,"INFO") == 1)
		{
			if(domain == NULL && ip == NULL)
			{
				strcpy(response,atoi(status_code[OK]));
				strcat(response,status_str[OK]);
				strcat(response,atoi(index));
				write(sock,&size_resp,sizeof(size_t));
				write(sock,response,size_resp)//send the index num to client
			}
			else
			{
				//bad request
			}			
		}
	}
}
//check the ip format if it is valid
int check_ip_invalid(char *ip)
{
	int ip_int[4];
	int i;
	sprintf(ip,"%d.%d.%d.%d",ip_int[0],ip_int[1],ip_int[2],ip_int[3]);
	for(i=0;i<4;i++)
		if(ip_int[i] > 255 || ip_int[i] < 0)
			return 1;
	return 0;

}
//check the domain name if it is valid
int check_domain_invalid(char *domain)
{
	int length = strlen(domain);
	int i;
	int count;
	for(i = 0;i<length;i++)
	{
		if(domain[i] == '.')
			count++;
	}
	if(count >= 1)
		return 0;
	else
		return 1;
}
//search the corresponding IP through the domain name
int search_ip(char *domain)
 {
	int i;
	for(i = 0; i<index;i++)
	{
		if(strcmp(domain,dns_data[i].domain) == 1)
			return i;
	}
	return 0;
}
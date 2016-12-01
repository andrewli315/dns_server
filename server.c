#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>     /* getopt, ssize_t */
#include <pthread.h>
#include "status.h"

#define SIZE_OF_BUFFER 128
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct DATA
{
	int ip[4];
	char domain[100];
}DNS_DATA;
DNS_DATA dns_data[1000];
static int data_index;
void *handle_request(void *socketfd);
int search_domain(char* ip);
int search_ip(char *domain);
int check_ip_invalid(char *ip);
int check_domain_invalid(char *domain);
int main(void)
{
	int socketfd, clientfd,addrlen;	
	int portno = 12345;
	int ret;
	int *new_sock;
	struct sockaddr_in serv_addr, client_addr;
	
	memset((char*)&serv_addr,0,sizeof(serv_addr));
	socketfd = socket(AF_INET,SOCK_STREAM,0);
	if(socketfd < 0)
	{
		printf("ERROR in create new socket\n");
		return 1;
	}
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	
	if(bind(socketfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0)
	{
		printf("ERROR on binding\n");
		return 1;
	}
	printf("breakpoint\n");
	//listen the client
	listen(socketfd,5);
	while(1)
	{
		addrlen = sizeof(client_addr);
		pthread_t new_thread;
		/* Wait and Accept connection */
		clientfd = accept(socketfd, (struct sockaddr*)&client_addr, &addrlen);
		new_sock = &clientfd;
		printf("connect successfully\n");
		ret = pthread_create(&new_thread,NULL,handle_request,(void*)new_sock);
		//handle_request(new_sock);


	}
	return 0;
}
void *handle_request(void *socketfd)
{
	int sock = *(int*)socketfd;
	int i = 0,j = 0,search_id;
	int flag =0;
	int read_size;
	size_t *size_req,size_resp;
	char request[SIZE_OF_BUFFER];
	char response[SIZE_OF_BUFFER]; 
	char cmd[5];
	char domain[100];
	char ip[20];
	memset(cmd,0,sizeof(cmd));
	memset(domain,0,sizeof(domain));
	memset(ip,0,sizeof(ip));
	printf("handle_request\n");
	while(1)
	{
		read(sock , &size_req , sizeof(size_t));
		read(sock , request , size_req);
		read_size = strlen(request);

		request[read_size] = '\0';
		printf("%s\n", request);
		while(request[i] != '\0')
		{
			if(request[i] == ' ')
			{
				flag++;
				j=0;
				i++;
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
			else
				i++;
		}
		printf("cmd: %s ip:%s domain:%s\n", cmd,ip,domain);
		//set mutex lock
		//pthread_mutex_lock(&mutex);
		printf("breakpoint:before judge\n");
		if(strcmp(cmd,"SET") == 0)
		{
			
			if(check_domain_invalid(domain) == 0 && check_ip_invalid(ip) == 0)
			{
				printf("breakpoint:SET\n");
				int ip_int[4];
				int i;
				sprintf(ip,"%d.%d.%d.%d",ip_int[0],ip_int[1],ip_int[2],ip_int[3]);//process the response string
				for(i = 0;i<4;i++)
					dns_data[data_index].ip[i] = ip_int[i];
				strcpy(dns_data[data_index].domain,domain);
				data_index++;
			}
			else
			{
				//bad request
				printf("breakpoint:set BAD_REQUEST\n");
				sprintf(response,"%d \"%s\"",status_code[BAD_REQUEST],status_str[BAD_REQUEST]);
				if(write(sock,&size_resp,sizeof(size_t)) == -1)
					break;
				if(write(sock,response,size_resp) == -1)
					break;
			}
		}
		else if(strcmp(cmd,"GET") == 0)
		{
			if(check_domain_invalid(domain) == 0 && ip == NULL)
			{
				printf("breakpoint:search\n");
				search_id = search_ip(domain);//process the response string
				if(search_id != -1)
				{
					//transmit response
					size_resp = sprintf(response,"%d \"%s\" %d.%d.%d.%d",status_code[OK],status_str[OK],
														dns_data[search_id].ip[0],dns_data[search_id].ip[1],
														dns_data[search_id].ip[2],dns_data[search_id].ip[3]);
					if(write(sock,&size_resp,sizeof(size_t)) == -1)
						break;
					if(write(sock,response,size_resp) == -1)
						break;
				}
				else
				{
					//404 NOT FOUND
					printf("breakpoint:404 NOT_FOUND\n");
					size_resp = sprintf(response,"%d \"%s\"",status_code[NOT_FOUND],status_str[NOT_FOUND]);
					if(write(sock,&size_resp,sizeof(size_t)) == -1)
						break;
					if(write(sock,response,size_resp) == -1)
						break;
				}
			}
			else
			{
				//bad request
				printf("breakpoint:BAD_REQUEST\n");
				sprintf(response,"%d \"%s\"",status_code[BAD_REQUEST],status_str[BAD_REQUEST]);
				if(write(sock,&size_resp,sizeof(size_t)) == -1)
					break;
				if(write(sock,response,size_resp) == -1)
					break;
			}			
		}
		else if(strcmp(cmd,"INFO") == 0)
		{
			if(domain == NULL && ip == NULL)
			{
				//send the index num to client
				size_resp = sprintf(response,"%d \"%s\" %d",status_code[OK],status_str[OK],index);
				write(sock,&size_resp,sizeof(size_t));
				write(sock,response,size_resp);

			}
			else
			{
				//bad request
				sprintf(response,"%d \"%s\"",status_code[BAD_REQUEST],status_str[BAD_REQUEST]);
				if(write(sock,&size_resp,sizeof(size_t)) == -1)
					break;
				if(write(sock,response,size_resp) == -1)
					break;
			}			
		}
		else
		{
			//method not allowed
			sprintf(response,"%d \"%s\"",status_code[METHOD_NOT_ALLOWED],status_str[METHOD_NOT_ALLOWED]);
			if(write(sock,&size_resp,sizeof(size_t)) == -1)
				break;
			if(write(sock,response,size_resp) == -1)
				break;
		}
		//mutex unlock
		//pthread_mutex_unlock(&mutex);
	}
	printf("breakpoint:end\n");
}
//check the ip format if it is valid
int check_ip_invalid(char *ip)
{
	int ip_int[4];
	int i;
	sscanf(ip,"%d.%d.%d.%d",ip_int[0],ip_int[1],ip_int[2],ip_int[3]);
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
	for(i = 0; i<data_index;i++)
	{
		if(strcmp(domain,dns_data[i].domain) == 1)
			return i;
	}
	return -1;
}

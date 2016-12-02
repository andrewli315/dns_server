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
	char ip[20];
	char domain[100];
}DNS_DATA;
DNS_DATA dns_data[1000];
static int data_index;
int thread_cnt=0;
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
	int *new_sock = malloc(sizeof(int));
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
	//listen the client
	listen(socketfd,5);
	addrlen = sizeof(client_addr);
	while((clientfd = accept(socketfd, (struct sockaddr*)&client_addr, (socklen_t*)&addrlen)))
	{	

		
		pthread_t new_thread[10];
		/* Wait and Accept connection */
		
		*new_sock = clientfd;
		printf("connect successfully\n");
		if(thread_cnt<=9)
		{
			ret = pthread_create(&new_thread[thread_cnt],NULL,handle_request,(void*)new_sock);
			thread_cnt++;
		}

		if(ret < 0)
		{
			printf("pthread create error\n");
			return 1;
		}
		if(clientfd < 0)
		{
			printf("client connect failed\n");
			return 1;
		}
	}
	printf("halt\n");
	return 0;
}
void *handle_request(void *socketfd)
{
	int sock = *(int*)socketfd;
	int i = 0,j = 0,search_id;
	int flag =0;
	int read_size,size_resp;
	size_t size_req;
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
		memset(request,'\0',sizeof(request));
		read(sock , &size_req , sizeof(size_t));
		read(sock , request , size_req);
		read_size = strlen(request);

		request[read_size] = '\0';
		memset(cmd,'\0',sizeof(cmd));
		memset(domain,'\0',sizeof(domain));
		memset(ip,'\0',sizeof(ip));
		printf("%s\n", request);
		sscanf(request,"%s %s %s",cmd,domain,ip);
		//printf("cmd: %s ip:%s domain:%s\n", cmd,ip,domain);
		//set mutex lock
		pthread_mutex_lock(&mutex);
		if(strcmp(cmd,"SET") == 0)
		{
			
			if(check_domain_invalid(domain) == 0 && check_ip_invalid(ip) == 0)
			{
				strcpy(dns_data[data_index].domain,domain);
				strcpy(dns_data[data_index].ip,ip);
				data_index++;
				size_resp = sprintf(response,"%d \"%s\"",status_code[OK],status_str[OK]);
			}
			else
			{
				//bad request
				size_resp = sprintf(response,"%d \"%s\"",status_code[BAD_REQUEST],status_str[BAD_REQUEST]);
				
			}
		}
		else if(strcmp(cmd,"GET") == 0)
		{
			if(check_domain_invalid(domain) == 0)
			{
				search_id = search_ip(domain);//process the response string
				if(search_id != -1)
				{
					//transmit response
					size_resp = sprintf(response,"%d \"%s\" %s",status_code[OK],status_str[OK],
														dns_data[search_id].ip);
					
				}
				else
				{
					//404 NOT FOUND
					size_resp = sprintf(response,"%d \"%s\"",status_code[NOT_FOUND],status_str[NOT_FOUND]);
					
				}
			}
			else
			{
				//bad request
				size_resp = sprintf(response,"%d \"%s\"",status_code[BAD_REQUEST],status_str[BAD_REQUEST]);
				
			}			
		}
		else if(strcmp(cmd,"INFO") == 0)
		{
			size_resp = sprintf(response,"200 \"OK\" %d",data_index);		
		}
		else if(strcmp(cmd,"\0") == 0 && strcmp(domain,"\0") == 0 && strcmp(ip,"\0") == 0)
		{
			pthread_mutex_unlock(&mutex);
			break;
		}
		else
		{
			//method not allowed
			size_resp  = sprintf(response,"%d \"%s\"",status_code[METHOD_NOT_ALLOWED],status_str[METHOD_NOT_ALLOWED]);
			
		}
		pthread_mutex_unlock(&mutex);
		printf("\n%d %s\n\n",size_resp,response);
		if(write(sock,&size_resp,sizeof(size_t)) < 0)
			break;
		if(write(sock,response,size_resp) < 0)
			break;
		//mutex unlock

		
	}
	thread_cnt--;
	pthread_exit((void*)socketfd);
	return 0;
}
//check the ip format if it is valid
int check_ip_invalid(char *ip)
{
	int ip_int[4];
	int i;
	sscanf(ip,"%d.%d.%d.%d",&ip_int[0],&ip_int[1],&ip_int[2],&ip_int[3]);
	for(i=0;i<4;i++)
	{
		if(ip_int[i] > 255 || ip_int[i] < 0)
		{
			printf("ip invalid\n");
			return 1;
		}
	}
	return 0;

}
//check the domain name if it is valid
int check_domain_invalid(char *domain)
{
	int length = strlen(domain);
	int i;
	int count=0;
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
		if(strcasecmp(domain,dns_data[i].domain) == 1)
			return i;
	}
	return -1;
}

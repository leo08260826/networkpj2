#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>

#define MAX_CLIENT 100
#define MAX_CLIENT_ONLINE 10
#define MAX_FD 1024
using namespace std;


int main(int argc,char* argv[]){
	//argv[1]=port
	if(argc!=2)
	{
		printf("Incorrect parameters!\n");
		return 0;
	}

	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	assert(sockfd!=-1);

	struct sockaddr_in server_addr;
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = PF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(atoi(argv[1]));

	int ret = bind(sockfd,(struct sockaddr*) &server_addr, sizeof(server_addr));
	assert(ret!=-1);

	listen(sockfd,MAX_CLIENT_ONLINE);
	
	fd_set readset;
	fd_set working_readset;
	FD_ZERO(&readset);
	FD_SET(sockfd,&readset);

	string clients_name[MAX_FD];
	
	while(1)
	{
		memcpy(&working_readset,&readset,sizeof(fd_set));
		int select_num = select(MAX_FD, &working_readset,NULL,NULL,NULL);

		assert(select_num >= 0);
		if(select_num == 0)
			continue;

		for(int fd=0;fd<MAX_FD;fd++)
		{
			if(!FD_ISSET(fd,&working_readset))
				continue;
			else if(fd == sockfd)
			{
				//new connect
				printf("New connect!\n");
				struct sockaddr_in client_addr;
				socklen_t addrlen = sizeof(client_addr);
				int client_fd = accept(sockfd,(struct sockaddr*)&client_addr,&addrlen);
				if(client_fd >=0)
				{
					FD_SET(client_fd,&readset);
					//TODO
				}
				else
				{
					printf("Client connecting error!\n");
				}
			}
			else
			{
				//receive things form connect-er
				//TODO
			
			}

		}//end for fd->MAXFD
	
	}//end while(1)

	return 0;
}

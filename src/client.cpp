#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

using namespace std;
int main(int argc,char* argv[]){
	//argv[1]=host //argv[2]=port
	if(argc!=3)
	{
		printf("wrong parameters!\n");
		return 0;
	}

	struct addrinfo hints;
	struct addrinfo *servinfo;

	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int ret = getaddrinfo(argv[1],argv[2],&hints,&servinfo);
	if(ret!=0)
	{
		printf("going worng when getaddrinfo!\n");
	}
	struct sockaddr_in *addr = (struct sockaddr_in*)servinfo->ai_addr;
	char host[100]={""};
	strcpy(host,inet_ntoa((struct in_addr)addr->sin_addr));
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd == -1)
	{
		printf("fails to create socket!\n");
		return 0;
	}

	struct sockaddr_in info;
	memset(&info,0,sizeof(info));
	info.sin_family = PF_INET;
	info.sin_addr.s_addr = inet_addr(host);
	info.sin_port = htons(atoi(argv[2]));

	ret = connect(sockfd,(struct sockaddr*)&info,sizeof(info));
	if(ret==-1)
	{
		printf("fails to connect to server!\n");
	}

	//TODO


	return 0;
}

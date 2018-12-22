#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <assert.h>
#include "LMLine_common.h"
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <string.h>
#include "Interface.h"

int UserStatus = LMLINE_GUESS;

int init_service(int sockfd, uint8_t op, char* username, char* password){

	// make header
	LMLine_protocol_header header;
	memset(&header, 0, sizeof(LMLine_protocol_header));
	header.op = op;
	header.status = UserStatus;

	// make packet 
	LMLine_protocol_account account;
	memset(&account, 0, sizeof(LMLine_protocol_account));
	account.magic = LMLINE_SUCCESS;
	strcpy(account.username, username);
	strcpy(account.password, password);

	// Send it
	send(sockfd, &header, sizeof(header), 0);
	send(sockfd, &account, sizeof(account), 0);

	// prepare receive packet
	LMLine_protocol_header res_header;
	LMLine_protocol_account res_account;
	memset(&res_header, 0, sizeof(res_header));
	memset(&res_account, 0, sizeof(res_account));

	recv(sockfd, &res_header, sizeof(res_header), 0);
	recv(sockfd, &res_account, sizeof(res_account), 0);

	assert(res_header.op == op && res_header.status == LMLINE_SERVER);
	if (res_account.magic == LMLINE_SUCCESS)
		return 1;
	else
		return 0;


}


void welcome_process(int sockfd){
	char cmd[CMD_LEN];
	scanf("%s", cmd);
	char username[USERNAME_MAXLEN], password[PASSWD_MAXLEN];

	if (strcmp(cmd, "/r") == 0){
		printf("\nPlease enter the username You want:\n");
		scanf("%s", username);
		printf("\nPlease enter the password You want:\n");
		scanf("%s", password);
		if (!init_service(sockfd, LMLINE_OP_REGISTER, username, password))
		{
			printf("Register Fail!\n"
				   "Reason: Same username exists!\n"
					);
		}
		Welcome_Interface();
	}
	else if (strcmp(cmd, "/login") == 0){
		printf("\nPlease enter Your username:\n");
		scanf("%s", username);
		printf("\nPlease enter Your password:\n");
		scanf("%s", password);
		if (!init_service(sockfd, LMLINE_OP_LOGIN, username, password))
		{
			printf("Login Fail!\n"
				   "Reason: WRONG password or username NOT exist!\n"
				   );
			Welcome_Interface();
		}
		else{
			UserStatus = LMLINE_ONLINE;
			Online_Interface();
		}
	}
	else{
		printf("Invalid Command\n");
		Welcome_Interface();
	}
}

void logout(){
	printf("haven't done yet XD.\n");
	//TODO
}

void user_process(int sockfd){
	string cmd;
	cin>>cmd

	if(cmd == "/logout"){
		logout();	
	}
	//TODO:

}

void handle_user_request(int sockfd){

	switch(UserStatus){
		case LMLINE_GUESS:
			welcome_process(sockfd);
			break;
		case LMLINE_ONLINE:
			user_process();
			break;
		case LMLINE_CHAT:
			//chat_process();
			break;
		default:
			break;
	}

}


using namespace std;
int main(int argc,char* argv[]){
	//argv[1]=host //argv[2]=port
	if(argc!=3)
	{
		printf("wrong parameters!\n");
		return 0;
	}

	// Init the Connect 	//
	
	struct addrinfo hints;
	struct addrinfo *servinfo;

	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int ret = getaddrinfo(argv[1],argv[2],&hints,&servinfo);
	assert(ret==0);

	struct sockaddr_in *addr = (struct sockaddr_in*)servinfo->ai_addr;
	char host[100]="";
	strcpy(host,inet_ntoa((struct in_addr)addr->sin_addr));
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	assert(sockfd!=-1);

	struct sockaddr_in info;
	memset(&info,0,sizeof(info));
	info.sin_family = PF_INET;
	info.sin_addr.s_addr = inet_addr(host);
	info.sin_port = htons(atoi(argv[2]));

	ret = connect(sockfd,(struct sockaddr*)&info,sizeof(info));
	assert(ret!=-1);

	// 
	
	
	// set up working set for Select //
	
	fd_set readset;
	fd_set working_readset;
	FD_ZERO(&readset);
	FD_SET(sockfd,&readset);
	FD_SET(STDIN_FILENO,&readset);
	//

	// Render UI //
	Welcome_Interface();
	//
	while(1){

		// copy readset to working set to keep readset clean //
		memcpy(&working_readset,&readset,sizeof(fd_set));
		int select_num = select(MAX_FD, &working_readset,NULL,NULL,NULL);
		// 
		assert(select_num >= 0);
		if (!select_num)	// continue if no any input or socket message
			continue;
		if (FD_ISSET(STDIN_FILENO,&working_readset)){
			handle_user_request(sockfd);
		}	// 






	}
	
	return 0;
}

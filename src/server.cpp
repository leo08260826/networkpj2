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

using namespace std;


int ReturnUsernameFromFile(uint8_t op, string username, string password){
	
	ifstream fin(ACCOUNT_FILE_PATH, ios::in);
	string entry;
	string pattern=",";
	int IsUsernameExist = 0, IsLoginSuccess = 0;

	while(getline(fin, entry)){
		int pos = entry.find(pattern, 0);
		string tmpusername = entry.substr(0, pos);
		string tmppassword = entry.substr(pos+1, entry.size());



		if (username == tmpusername && password == tmppassword){
			cout << tmpusername << " " << tmppassword << endl;
			IsUsernameExist = 1;
			IsLoginSuccess = 1;
			break;
		}
		if (username == tmpusername){
			IsUsernameExist = 1;
			break;
		}
	}
	fin.close();

	if (!IsUsernameExist && op == LMLINE_OP_REGISTER){
		
		fstream f(ACCOUNT_FILE_PATH, ios::out | ios::app);
		f << username << "," << password << endl ;
		f.close();

	}

	if(IsLoginSuccess)	return IsLoginSuccess;
	if(!IsLoginSuccess && IsUsernameExist) return 2; // set special flag for LOGIN FAIL
	if(!IsUsernameExist)	return IsUsernameExist;
	return -1;
}



void UserRegister(int client_fd){


	// accept Register request
	LMLine_protocol_account account;
	memset(&account, 0, sizeof(LMLine_protocol_account));
	recv(client_fd, &account, sizeof(account), 0);

	string reg_username = account.username;
	string reg_password = account.password;

	if (ReturnUsernameFromFile(LMLINE_OP_REGISTER,reg_username, reg_password)){		// same username exists, register FAIL
		
		LMLine_protocol_header header;
		memset(&header, 0, sizeof(LMLine_protocol_header));
		header.op = LMLINE_OP_REGISTER;
		header.status = LMLINE_SERVER;
		send(client_fd, &header, sizeof(header), 0);

		LMLine_protocol_account account;
		memset(&account, 0, sizeof(LMLine_protocol_account));
		account.magic = LMLINE_FAIL;
		send(client_fd, &account, sizeof(account), 0);

		printf("A client register fail\n");
	}
	else{ 		// same username exists, register FAIL

		LMLine_protocol_header header;
		memset(&header, 0, sizeof(LMLine_protocol_header));
		header.op = LMLINE_OP_REGISTER;
		header.status = LMLINE_SERVER;
		send(client_fd, &header, sizeof(header), 0);

		LMLine_protocol_account account;
		memset(&account, 0, sizeof(LMLine_protocol_account));
		account.magic = LMLINE_SUCCESS;
		send(client_fd, &account, sizeof(account), 0);

		printf("A client register success\n");
	}


}

void UserLogin(int client_fd){

	// accept Login Request
	LMLine_protocol_account account;
	memset(&account, 0, sizeof(LMLine_protocol_account));
	recv(client_fd, &account, sizeof(account), 0);

	string log_username = account.username;
	string log_password = account.password;

	if (ReturnUsernameFromFile(LMLINE_OP_LOGIN, log_username, log_password) == 1){	// Login SUCCESS 

		LMLine_protocol_header header;
		memset(&header, 0, sizeof(LMLine_protocol_header));
		header.op = LMLINE_OP_LOGIN;
		header.status = LMLINE_SERVER;
		send(client_fd, &header, sizeof(header), 0);

		LMLine_protocol_account account;
		memset(&account, 0, sizeof(LMLine_protocol_account));
		account.magic = LMLINE_SUCCESS;
		send(client_fd, &account, sizeof(account), 0);

		printf("A client login success\n");
		
	}
	else{		// Login FAIL due to username not exist or wrong password

		LMLine_protocol_header header;
		memset(&header, 0, sizeof(LMLine_protocol_header));
		header.op = LMLINE_OP_LOGIN;
		header.status = LMLINE_SERVER;
		send(client_fd, &header, sizeof(header), 0);

		LMLine_protocol_account account;
		memset(&account, 0, sizeof(LMLine_protocol_account));
		account.magic = LMLINE_FAIL;
		send(client_fd, &account, sizeof(account), 0);

		printf("A client login fail\n");

	}


}


void handle_client_request(int client_fd){

	// first accept header 
	LMLine_protocol_header header;
	memset(&header, 0, sizeof(LMLine_protocol_header));
	recv(client_fd, &header, sizeof(header), 0);

	switch(header.op){
		case LMLINE_OP_REGISTER:
		UserRegister(client_fd);
		break;
		case LMLINE_OP_LOGIN:
		UserLogin(client_fd);
		break;
		default:
		break;

	}



}



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

		for(int fd=0;fd < MAX_FD;fd++)
		{	
			int client_fd;
			if(!FD_ISSET(fd,&working_readset))
				continue;
			else if(fd == sockfd)
			{
				//new connect
				printf("New connect!\n");
				struct sockaddr_in client_addr;
				socklen_t addrlen = sizeof(client_addr);
				client_fd = accept(sockfd,(struct sockaddr*)&client_addr,&addrlen);
				assert(client_fd >= 0);
				FD_SET(client_fd, &readset);
			}
			else
				handle_client_request(client_fd);
		}
	
	}

	return 0;
}

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
#include <map>
#include <sys/stat.h> 
#include <vector>
#include <algorithm>
using namespace std;


// to look up from username -> file descriptor
map<string, int> UsernameTLB;

// to look up from file descriptor -> username
map<int, string> ReverseUsernameTLB;

vector<string> 	ExistUserList;
int ConnectionList[MAX_CLIENT] = {0}; 


void InitUserList(){

	fstream fin(ACCOUNT_FILE_PATH, ios::in);
	string entry;
	string pattern=",";
	while(getline(fin, entry)){
		int pos = entry.find(pattern, 0);
		string username = entry.substr(0, pos);
		ExistUserList.push_back(username);
	}
	return;
}



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


	uint8_t magic;
	if (ReturnUsernameFromFile(LMLINE_OP_REGISTER,reg_username, reg_password)){		// same username exists, register FAIL
		magic = LMLINE_FAIL;
		printf("A client register fail\n");
	}
	else{ 		// register success
		magic = LMLINE_SUCCESS;
		//	Need to Create a unique dir for New Register
		char* DIR_PATH = (char*)malloc(30);
		strcpy(DIR_PATH, USR_DIR_PATH);
		strcat(DIR_PATH, reg_username.c_str());
		mkdir(DIR_PATH, 0666);
		printf("A client register success\n");
		// Need to Update ExistUserList
		ExistUserList.push_back(reg_username);
	}

	LMLine_protocol_header header;
	memset(&header, 0, sizeof(LMLine_protocol_header));
	header.op = LMLINE_OP_REGISTER;
	header.status = LMLINE_SERVER;
	send(client_fd, &header, sizeof(header), 0);

	/*
	LMLine_protocol_account account;
	memset(&account, 0, sizeof(LMLine_protocol_account));
	*/
	account.magic = magic;
	send(client_fd, &account, sizeof(account), 0);
	
	// return;
}

void UserLogin(int client_fd){

	// accept Login Request
	LMLine_protocol_account account;
	memset(&account, 0, sizeof(LMLine_protocol_account));
	recv(client_fd, &account, sizeof(account), 0);

	string log_username = account.username;
	string log_password = account.password;

	uint8_t magic;
	if (ReturnUsernameFromFile(LMLINE_OP_LOGIN, log_username, log_password) == 1){	// Login SUCCESS 
		
		magic = LMLINE_SUCCESS;
		// assign connection
		UsernameTLB[log_username] = client_fd;
		ReverseUsernameTLB[client_fd] = log_username;

		printf("A client login success\n");
		
	}
	else{		// Login FAIL due to username not exist or wrong password

		magic = LMLINE_FAIL;
		printf("A client login fail\n");
	}
	LMLine_protocol_header header;
	memset(&header, 0, sizeof(LMLine_protocol_header));
	header.op = LMLINE_OP_LOGIN;
	header.status = LMLINE_SERVER;
	send(client_fd, &header, sizeof(header), 0);

	/*
	LMLine_protocol_account account;
	memset(&account, 0, sizeof(LMLine_protocol_account));
	*/
	account.magic = magic;
	send(client_fd, &account, sizeof(account), 0);

}

void UserLogout(int client_fd){

	string username = ReverseUsernameTLB[client_fd];
	ReverseUsernameTLB.erase(client_fd);
	UsernameTLB.erase(username);

	printf("A client logout or close connect\n");

	
	// return;
}

int CheckConnectValid(string username){
	vector<string>::iterator it = find(ExistUserList.begin(), ExistUserList.end(), username);
	if (it != ExistUserList.end())	return 1;
	else		return 0;
}

void ConstructConnect(int client_fd, string connectusername){
	// first get the connect user fd
	int connectuserid = UsernameTLB[connectusername];
	// then set the connection. Notice that this is one-side, not both-side
	ConnectionList[client_fd] = connectuserid;
}



void UserConnect(int client_fd){

	LMLine_protocol_communicate req_communicate;
	memset(&req_communicate, 0, sizeof(req_communicate));
	recv(client_fd, &req_communicate, sizeof(req_communicate), 0);

	uint8_t magic; 

	string connectusername = req_communicate.dstusername;
	if (CheckConnectValid(connectusername) != 0){		// Connect is valid(Username exist), construct connect
		ConstructConnect(client_fd, connectusername);
		magic = LMLINE_SUCCESS;
	}
	else{
		magic = LMLINE_FAIL;
		printf("connect fail\n");
		vector<string>::iterator it;
		for(it=ExistUserList.begin();it!=ExistUserList.end();it++)
			cout << *it << endl;
	}

	// Send Back the Response back to original client 
	LMLine_protocol_header header;
	memset(&header, 0, sizeof(LMLine_protocol_header));
	header.op = LMLINE_OP_CONNECT;
	header.status = LMLINE_SERVER;
	send(client_fd, &header, sizeof(header), 0);

	LMLine_protocol_communicate res_communicate;
	memset(&res_communicate, 0, sizeof(LMLine_protocol_communicate));
	
	res_communicate.magic = magic;
	send(client_fd, &res_communicate, sizeof(req_communicate), 0);
	// 

}

void UserChat(int client_fd){

	LMLine_protocol_communicate req_communicate;
	memset(&req_communicate, 0, sizeof(req_communicate));
	recv(client_fd, &req_communicate, sizeof(req_communicate), 0);

	uint8_t magic;

	char msg[MSG_MAXLEN];
	strcpy(msg,req_communicate.message);
	printf("msg:%s\n",msg);

	//TODO
	//deliver to another





	//Send Back to Original client, To ACK
	LMLine_protocol_header header;
	memset(&header, 0 ,sizeof(LMLine_protocol_header));
	header.op = LMLINE_OP_CHAT;
	header.status = LMLINE_SERVER;
	send(client_fd, &header, sizeof(header),0);
	
	LMLine_protocol_communicate res_communicate;
	memset(&res_communicate, 0, sizeof(LMLine_protocol_communicate));
	res_communicate.magic = LMLINE_SUCCESS;//need modify
	send(client_fd, &res_communicate,sizeof(res_communicate),0);


	// Need to write the msg to HISTORY msg, and the history msg is 2-side too. Store: A->B, B->A.



}

void UserFile(int client_fd){

	LMLine_protocol_file req_file;
	memset(&req_file,0,sizeof(req_file));
	recv(client_fd, &req_file,sizeof(req_file),0);

	uint8_t magic;

	char filename[FILENAME_MAXLEN];
	char file[FILE_MAXLEN];
	int file_len;
	strcpy(filename,req_file.filename);
	file_len=req_file.file_len;
	recv(client_fd,file,file_len,0);
	
	//TODO
	//deliver to another, ONLY SEND TO ONLINE USER, which menas the CONNECT IS 2-side
	




	//Send Back to Original client, To ACK
	LMLine_protocol_header header;
	memset(&header,0,sizeof(LMLine_protocol_header));
	header.op = LMLINE_OP_FILE_SEND;
	header.status = LMLINE_SERVER;
	send(client_fd,&header,sizeof(header),0);

	LMLine_protocol_file res_file;
	memset(&res_file,0,sizeof(LMLine_protocol_file));
	res_file.magic = LMLINE_SUCCESS;//need modify
	send(client_fd,&res_file,sizeof(res_file),0);

}

void handle_client_request(int client_fd, fd_set* readset){

	// first accept header 
	LMLine_protocol_header header;
	memset(&header, 0, sizeof(LMLine_protocol_header));
	if (recv(client_fd, &header, sizeof(header), 0) == 0){
		UserLogout(client_fd);
		FD_CLR(client_fd, readset);
	}

	switch(header.op){
		case LMLINE_OP_REGISTER:
			UserRegister(client_fd);
			break;
		case LMLINE_OP_LOGIN:
			UserLogin(client_fd);
			break;
		case LMLINE_OP_LOGOUT:
			UserLogout(client_fd);
			break;
		case LMLINE_OP_CONNECT:
			UserConnect(client_fd);
			break;
		case LMLINE_OP_CHAT:
			UserChat(client_fd);
			break;
		case LMLINE_OP_FILE_SEND:
			UserFile(client_fd);
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
	
	// Have to Init the exist userlist every time after reboot
	InitUserList();

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
			if(fd == sockfd)
			{
				//new connect
				printf("New connect!\n");
				struct sockaddr_in client_addr;
				socklen_t addrlen = sizeof(client_addr);
				client_fd = accept(sockfd,(struct sockaddr*)&client_addr,&addrlen);
				assert(client_fd >= 0);
				FD_SET(client_fd, &readset);
			}
			else{
				client_fd = fd;
				handle_client_request(client_fd, &readset);
			}
		}
	
	}

	return 0;
}

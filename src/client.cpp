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
#include <sys/stat.h> 

using namespace std;
int UserStatus = LMLINE_GUESS;
char ConnectionUsername[USERNAME_MAXLEN];

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

	/////////////////////////
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
		printf("\nInvalid Command\n");
		Welcome_Interface();
	}
}

void logout(int sockfd){
	
	LMLine_protocol_header header;
	memset(&header, 0, sizeof(LMLine_protocol_header));
	header.op = LMLINE_OP_LOGOUT;
	header.status = UserStatus;
	
	// Only need to send header Dont need to get the response
	send(sockfd, &header, sizeof(header), 0);

	UserStatus = LMLINE_GUESS;

	return;
}

int connecttouser(int sockfd){

	char connectusername[USERNAME_MAXLEN];
	scanf("%s", connectusername);

	strcpy(ConnectionUsername,connectusername);
	printf("Connecting to %s\n",ConnectionUsername);

	// Prepare packet and Send Request to Server 
	LMLine_protocol_header header;
	memset(&header, 0, sizeof(LMLine_protocol_header));
	header.op = LMLINE_OP_CONNECT;
	header.status = UserStatus;

	LMLine_protocol_communicate communitcate_packet;
	memset(&communitcate_packet, 0, sizeof(LMLine_protocol_communicate));
	strcpy(communitcate_packet.dstusername, connectusername);

	send(sockfd, &header, sizeof(header), 0);
	send(sockfd, &communitcate_packet, sizeof(communitcate_packet), 0);


	//////////////////
	// Receive packet
	LMLine_protocol_header res_header;
	memset(&header, 0, sizeof(LMLine_protocol_header));

	LMLine_protocol_communicate res_communitcate_packet;
	memset(&communitcate_packet, 0, sizeof(LMLine_protocol_communicate));

	recv(sockfd, &res_header, sizeof(res_header), 0);
	recv(sockfd, &res_communitcate_packet, sizeof(res_communitcate_packet), 0);

	if (res_header.op != LMLINE_OP_CONNECT || res_header.status != LMLINE_SERVER || res_communitcate_packet.magic != LMLINE_SUCCESS)
		return 0;
	else
		return 1;

}
void AddFriend(int sockfd){

	char mode[5], username[USERNAME_MAXLEN]; 
	scanf("%s %s",mode, username);


	uint8_t op;
	if (strcmp(mode, "-add") == 0){
		op = LMLINE_OP_FRIEND_ADD;
	}
	else if (strcmp(mode, "-del") == 0){
		op = LMLINE_OP_FRIEND_DEL;
	}

	// Send Add(or Del) friend Req
	LMLine_protocol_header header;
	memset(&header, 0, sizeof(LMLine_protocol_header));
	header.op = op;
	header.status = UserStatus;

	LMLine_protocol_communicate communicate_packet;
	memset(&communicate_packet, 0, sizeof(LMLine_protocol_communicate));
	strcpy(communicate_packet.dstusername, username);

	send(sockfd, &header, sizeof(header), 0);
	send(sockfd, &communicate_packet, sizeof(communicate_packet), 0);

	// Receive the response
	LMLine_protocol_communicate res_communicate_packet;
	memset(&res_communicate_packet, 0, sizeof(LMLine_protocol_communicate));

	recv(sockfd, &header, sizeof(header), 0);
	recv(sockfd, &res_communicate_packet, sizeof(res_communicate_packet), 0);

	if (res_communicate_packet.magic == LMLINE_SUCCESS)
		printf("\n%s User:%s SUCCESS\n", mode, username);
	else
		printf("\n%s User:%s FAIL\n", mode, username);
}

void ShowFriend(int sockfd){

	LMLine_protocol_header header;
	memset(&header, 0, sizeof(LMLine_protocol_header));

	header.op = LMLINE_OP_FRIEND_SHOW;
	header.status = UserStatus;
	send(sockfd, &header, sizeof(header), 0);


	LMLine_protocol_file friendfileheader;
	memset(&friendfileheader, 0, sizeof(LMLine_protocol_file));
	recv(sockfd, &header, sizeof(header), 0);
	recv(sockfd, &friendfileheader, sizeof(friendfileheader), 0);
	
	if (header.op != LMLINE_OP_FRIEND_SHOW || header.status != LMLINE_SERVER || friendfileheader.magic != LMLINE_SUCCESS)
		return;
	else{
		int friendfilelen = friendfileheader.file_len;
		char friendfile[FILE_MAXLEN];
		recv(sockfd, friendfile, friendfilelen, 0);
		printf("\n%s\n", friendfile);
		return;
	}


}


void user_process(int sockfd){
	char cmd[CMD_LEN];
	scanf("%s", cmd);

	if (strcmp(cmd, "/h") == 0){
		Online_Interface();
	}
	else if (strcmp(cmd, "/logout") == 0){
		logout(sockfd);
		Welcome_Interface();
	}
	else if (strcmp(cmd, "/c") == 0){
		if (connecttouser(sockfd)){
			UserStatus = LMLINE_CHAT;
			Chat_Interface();
		}
		else{
			printf("(Connect Fail Due to username not Exist)\n");
			Online_Interface();
		}
	}
	else if (strcmp(cmd, "/f") == 0){
		AddFriend(sockfd);	// Use to Add or Del friend
		//Online_Interface();
	}
	else if (strcmp(cmd, "/lf") == 0){
		ShowFriend(sockfd);
		//Online_Interface();
	}
	else{
		printf("(Invalid Command)\n");
		//Online_Interface();
	}
}

void send_msg(int sockfd){
	char msg[MSG_MAXLEN];
	scanf("%s",msg);

	//send
	LMLine_protocol_header header;
	memset(&header, 0, sizeof(LMLine_protocol_header));
	header.op = LMLINE_OP_CHAT;
	header.status = UserStatus;

	LMLine_protocol_communicate communicate_packet;
	memset(&communicate_packet, 0, sizeof(LMLine_protocol_communicate));
	
	strcpy(communicate_packet.message,msg);
	strcpy(communicate_packet.dstusername,ConnectionUsername);

	send(sockfd,&header,sizeof(header),0);
	send(sockfd,&communicate_packet,sizeof(communicate_packet),0);
	
	//recv
	LMLine_protocol_header res_header;
	memset(&header, 0, sizeof(LMLine_protocol_header));

	LMLine_protocol_communicate res_communicate_packet;
	memset(&res_communicate_packet, 0,sizeof(LMLine_protocol_communicate));

	recv(sockfd, &res_header,sizeof(res_header), 0);
	recv(sockfd, &res_communicate_packet,sizeof(res_communicate_packet),0);
/*
	if(res_header.op != LMLINE_OP_CHAT || res_header.status != LMLINE_SERVER || res_communicate_packet.magic != LMLINE_SUCCESS)	
			printf("fails to send\n");
	else
			printf("success to send\n");
*/
}

void send_file(int num,LMLine_protocol_header* header,LMLine_protocol_file* file_packet,LMLine_file* files){
	printf("(Enter the path of the %d-th file:)\n",num+1);
	char path[MSG_MAXLEN];
	scanf("%s",path);
	char filename[FILENAME_MAXLEN];
	printf("(Enter the name of the %d-th file:)\n",num+1);
	scanf("%s",filename);
	char file[FILE_MAXLEN];
	FILE *fp=fopen(path,"rb");
	if(fp==NULL)
	{
		printf("(wrong path)\n");
		return;
	}
	int file_len = fread(file,sizeof(char),sizeof(file),fp);
	strcpy(files->content,file);
	fclose(fp);
	
	//send
	memset(header,0,sizeof(LMLine_protocol_header));
	header->op = LMLINE_OP_FILE_SEND;
	header->status = UserStatus;
	memset(file_packet,0,sizeof(LMLine_protocol_file));
	strcpy(file_packet->dstusername,ConnectionUsername);
	strcpy(file_packet->filename,filename);
	file_packet->file_len = file_len;
	
}

void pre_send_file(int sockfd){
	int num;
	scanf("%d",&num);
	printf("=====start sending file=====\n");

	LMLine_protocol_header header[MAX_FILE];
	LMLine_protocol_file file_packet[MAX_FILE];
	LMLine_file file[MAX_FILE];
	for(int i=0;i<num;i++){
		send_file(i,&header[i],&file_packet[i],&file[i]);
	}
	for(int i=0;i<num;i++){
		
		send(sockfd,&header[i],sizeof(header[i]),0);
		send(sockfd,&file_packet[i],sizeof(file_packet[i]),0);
		send(sockfd,&file[i],file_packet[i].file_len,0);
		//recv
		LMLine_protocol_header res_header;
		memset(&header, 0 ,sizeof(LMLine_protocol_header));

		LMLine_protocol_file res_file_packet;
		memset(&res_file_packet,0,sizeof(LMLine_protocol_file));

		recv(sockfd,&res_header,sizeof(res_header),0);
		recv(sockfd,&res_file_packet,sizeof(res_file_packet),0);
	
		if(res_header.op != LMLINE_OP_FILE_SEND || res_header.status != LMLINE_SERVER || res_file_packet.magic != LMLINE_SUCCESS)
			printf("(fails to send %d-th file)\n",i+1);
		else	
			printf("(success to send %d-th file)\n",i+1);

	}
	printf("=====end sending file=======\n");
}

void handle_msg(int sockfd){
	char msg[MSG_MAXLEN];
	
	LMLine_protocol_header res_header;
	memset(&res_header,0,sizeof(res_header));
	recv(sockfd,&res_header,sizeof(res_header),0);

	if(res_header.op == LMLINE_OP_CHAT){
		//chat
		LMLine_protocol_communicate res_communicate;
		memset(&res_communicate,0,sizeof(res_communicate));
		recv(sockfd,&res_communicate,sizeof(res_communicate),0);

		strcpy(msg,res_communicate.message);
		printf("%s:%s\n",ConnectionUsername,msg);

		LMLine_protocol_header header;
		memset(&header,0,sizeof(header));
		header.op = LMLINE_OP_CHAT;
		header.status = LMLINE_CHAT;
		send(sockfd,&header,sizeof(header),0);

		LMLine_protocol_communicate communicate;
		memset(&communicate,0,sizeof(communicate));
		communicate.magic = LMLINE_SUCCESS;
		send(sockfd,&communicate,sizeof(communicate),0);
	}
	else if(res_header.op == LMLINE_OP_FILE_SEND){
		//file send
		LMLine_protocol_file res_file_packet;
		memset(&res_file_packet,0,sizeof(res_file_packet));
		recv(sockfd,&res_file_packet,sizeof(res_file_packet),0);

		//file
		char file[FILE_MAXLEN];
		recv(sockfd,&file,res_file_packet.file_len,0);
		//write file
		fstream file_f;
		file_f.open(res_file_packet.filename,ios::out|ios::trunc);
		file_f.write(file,res_file_packet.file_len);
		file_f.close();

		printf("%s:(send a file:%s)\n",ConnectionUsername,res_file_packet.filename);

		LMLine_protocol_header header;
		memset(&header,0,sizeof(header));
		header.op = LMLINE_OP_FILE_SEND;
		header.status = LMLINE_OP_FILE_SEND;
		send(sockfd,&header,sizeof(header),0);

		LMLine_protocol_file file_packet;
		memset(&file_packet,0,sizeof(file_packet));
		file_packet.magic = LMLINE_SUCCESS;
		send(sockfd,&file_packet,sizeof(file_packet),0);
	}
}

void leave(int sockfd){
	//leaving chatroom
	printf("(Left from chatroom with %s)\n",ConnectionUsername);
	strcat(ConnectionUsername,"");

	LMLine_protocol_header header;
	memset(&header,0,sizeof(header));
	header.op = LMLINE_OP_LEAVE;
	header.status = UserStatus;
	send(sockfd, &header,sizeof(header),0);

	LMLine_protocol_header res_header;
	memset(&res_header,0,sizeof(res_header));
	recv(sockfd,&res_header,sizeof(res_header),0);
}

void query(int sockfd){
	LMLine_protocol_header header;
	memset(&header,0,sizeof(header));
	header.op = LMLINE_OP_QUERY;
	header.status = UserStatus;
	send(sockfd,&header,sizeof(header),0);

	printf("=====start logging======\n");

	LMLine_protocol_communicate line_packet;
	memset(&line_packet,0,sizeof(line_packet));
	recv(sockfd,&line_packet,sizeof(line_packet),0);
	while(line_packet.magic!=LMLINE_FAIL){
		
		char line_msg[MSG_MAXLEN];
		strcpy(line_msg,line_packet.message);
		if(line_msg[0]=='I'){
			printf(":%s\n",&line_msg[1]);
		}
		else if(line_msg[0]=='U'){
			printf("%s:%s\n",ConnectionUsername,&line_msg[1]);
		}
		else{
			//pass
		}
		memset(&line_packet,0,sizeof(line_packet));
		recv(sockfd,&line_packet,sizeof(line_packet),0);
	}
	printf("=====end of log=========\n");

}

void chat_process(int sockfd){
	char cmd[CMD_LEN];
	scanf("%s",cmd);
	if (strcmp(cmd,"/h") == 0){
		Chat_Interface();
	}
	else if (strcmp(cmd,"/log") == 0){
		query(sockfd);
	}
	else if (strcmp(cmd,"/s") == 0){
		send_msg(sockfd);
	}
	else if (strcmp(cmd,"/file") == 0){
		pre_send_file(sockfd);
	}
	else if (strcmp(cmd,"/l") == 0){
		leave(sockfd);
		UserStatus = LMLINE_ONLINE;
		Online_Interface();
	}
	else{
		printf("(Invaild Command)\n");
	}
}

void handle_user_request(int sockfd){

	switch(UserStatus){
		case LMLINE_GUESS:
			welcome_process(sockfd);
			break;
		case LMLINE_ONLINE:
			user_process(sockfd);
			break;
		case LMLINE_CHAT:
			chat_process(sockfd);
			break;
		default:
			break;
	}

}

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
		if (FD_ISSET(sockfd,&working_readset) && UserStatus==LMLINE_CHAT ){
			//msg arrives
			handle_msg(sockfd);
		}	

	}
	
	return 0;
}

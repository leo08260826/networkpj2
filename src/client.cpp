#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <assert.h>
#include "LMLine_common.h"

void Welcome_Interface(){
	printf("#############################################\n"
	       "#				  	    #\n"			 	
	       "#				   	    #\n"
	       "#    	    Welcome to LMLine!!!    	    #\n"
	       "#					    #\n"
	       "#############################################\n"
	       "If You're new to here, Please go Register first~\n"
	       "If You already have an account, Please Login to use our Line~\n\n\n"
	       "Help:\n"
	       "/r     ->  to Register \n"
	       "/login ->  to Login \n\n\n"
	       );


}
void Online_Interface(){
	printf("#############################################\n"
	       "#				  	    #\n"			 	
	       "#				   	    #\n"
	       "#    	    Welcome Back User!!!    	    #\n"
	       "#					    #\n"
	       "#############################################\n"
	       
	       "Help:\n"
	       "/c username -> to Connect to user with username\n"	       
	       "/logout     -> to Logout our line\n"
	       "/f          -> to Add friend\n"
	       "/lf         -> to List friend\n"
	       "/h	    -> to show the Help Interface\n\n\n"
	       );
}
void Chat_Interface(){
	printf("#############################################\n"
	       "#				  	    #\n"			 	
	       "#				   	    #\n"
	       "#    	    Start To Chat 	    	    #\n"
	       "#					    #\n"
	       "#############################################\n"
	       
	       "Help:\n"
	       "/log        -> to show all your chats LOG\n"	       
	       "/s message  -> to Send the message\n"
	       "/file file  -> to send your FILE\n"
	       "/q          -> to Quit the chatroom\n"
	       "/h          -> to show the HELP Interface\n\n\n"
	       );
}





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

	Welcome_Interface();
	Online_Interface();
	Chat_Interface();

	
	
	
	

	

	//TODO


	return 0;
}

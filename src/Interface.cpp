#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

void Welcome_Interface(){
	printf("######################################\n"
		"#                                    #\n"
		"#          Welcome to LMLine         #\n"
		"#                                    #\n"
		"######################################\n");
	printf("If You're new to here, please \"Register\" first!\n"
		"If You already have an account, please \"Login\"!\n"
		"Help:\n"
		"/r		-> Register\n"
		"/login		-> Login\n");
}

void Online_Interface(){
	printf("######################################\n"
		"#                                    #\n"
		"#             Welcome Back!          #\n"
		"#                                    #\n"
		"######################################\n");
	printf("You've login\n"
		"HELP:\n"
		"/c [username]	-> Chat to [username]\n"
		"/logout		-> Logout\n"
		"/f [username]	-> Add [username] as friend\n"
		"/lf		-> List friends\n"
		"/h		-> Show the \"HELP\" interfacw\n");
}

void Chat_Interface(){
	printf("#####################################\n"
		"#                                   #\n"
		"#             Chart Room            #\n"
		"#                                   #\n"
		"#####################################\n");
	printf("HELP:\n"
		"/log		-> Show all the historical message\n"
		"/s [message]	-> Send \"[message]\"\n"
		"/file [path]	-> Send file\n"
		"/l		-> Leave the chatroom\n"
		"/h		-> Shoe the \"HELP\" interface\n");
}

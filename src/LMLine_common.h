#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>


#define USERNAME_MAXLEN 20
#define PASSWD_MAXLEN 20
#define MAX_CLIENT 100
#define MAX_CLIENT_ONLINE 10
#define MAX_FD 1024
#define MAX_FILE 10
#define CMD_LEN 10
#define MSG_MAXLEN 1024
#define FILENAME_MAXLEN 20
#define FILE_MAXLEN 10000
#define PATH_MAXLEN 1024

char ACCOUNT_FILE_PATH[30] = "./Account/account.txt";
char USR_DIR_PATH[20] = "./USER/" ;

typedef enum{
	LMLINE_OP_REGISTER=0x01,
	LMLINE_OP_LOGOUT,
	LMLINE_OP_CONNECT,
	LMLINE_OP_CHAT,
	LMLINE_OP_DELCHAT,
	LMLINE_OP_FILE_SEND,
	LMLINE_OP_FRIEND_ADD,
	LMLINE_OP_FRIEND_DEL,
	LMLINE_OP_FRIEND_SHOW,
	LMLINE_OP_QUERY,
	LMLINE_OP_LOGIN,
	LMLINE_OP_LEAVE
}LMLine_protocol_op;


typedef enum{
	LMLINE_GUESS=0x01,
	LMLINE_ONLINE,
	LMLINE_CHAT,
	LMLINE_SERVER,
}LMLine_protocol_UserStatus;

typedef enum{
	LMLINE_SUCCESS=0x01,
	LMLINE_FAIL,
}LMLine_protocol_magic;

typedef struct {
	uint8_t op;
	uint8_t status;
}LMLine_protocol_header;

typedef struct {
	// magic is to check if the register/login is success or not
	uint8_t magic;
	char username[USERNAME_MAXLEN];
	char password[PASSWD_MAXLEN];
}LMLine_protocol_account;	


//	use this packet to handle Make Connect to Others  & Chat with Others BUT excluse FILE Transfer 
typedef struct {
	// this field to specific whether connection is SUCCESS or FAIL
	uint8_t magic;
	// this two field is MAINly for server to store the history message more convenient !
	char srcusername[USERNAME_MAXLEN],dstusername[USERNAME_MAXLEN];
	// Only message Content 
	char message[MSG_MAXLEN];
}LMLine_protocol_communicate;

typedef struct {
	uint8_t magic;	
	char srcusername[USERNAME_MAXLEN],dstusername[USERNAME_MAXLEN];
	char filename[FILENAME_MAXLEN];
	int file_len;
}LMLine_protocol_file;


typedef struct {
	char content[FILE_MAXLEN];
}LMLine_file;











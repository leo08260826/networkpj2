#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>


#define USERNAME_MAXLEN 20
#define PASSWD_MAXLEN 20


typedef enum{
	LMLINE_OP_REGISTER=0x01,
	LMLINE_OP_LOGIN=0x02,
	LMLINE_OP_LOGOUT=0x03,
	LMLINE_OP_CONNECT=0x04,
	LMLINE_OP_CHAT=0x05,
	LMLINE_OP_FILE_SEND=0x06,
	LMLINE_OP_QUERY=0x07,
}LMLine_protocol;


typedef enum{
	LMLINE_GUESS=0x01,
	LMLINE_ONLINE=0x02,
	LMLINE_CHAT=0x03
	LMLINE_SERVER=0x04,
}LMLine_UserStatus;
#define  _CRT_SECURE_NO_WARNINGS 
#include "CodeC.h"
#include <iostream>
#include <string.h>
using namespace std;

CodeC::CodeC(){
	message = new AskMessage;
	memset(message, 0x00, sizeof(AskMessage));
}

CodeC::CodeC(AskMessage *m){
	message = new AskMessage;
	memset(message , 0x00 , sizeof(AskMessage));
	message->num = m->num;
	memcpy(message->Clientid, m->Clientid, strlen(m->Clientid));
	memcpy(message->Toserverid, m->Toserverid, strlen(m->Toserverid));
	memcpy(message->AuthenticationCode, m->AuthenticationCode, strlen(m->AuthenticationCode));
	memcpy(message->Randnum_Client, m->Randnum_Client, strlen(m->Randnum_Client));
}

int CodeC::msgEncode(char ** outData, int & len){
	writeHeadNode(message->num);
	writeNextNode(message->Clientid, strlen(message->Clientid));
	writeNextNode(message->Toserverid, strlen(message->Toserverid));
	writeNextNode(message->AuthenticationCode, strlen(message->AuthenticationCode));
	writeNextNode(message->Randnum_Client, strlen(message->Randnum_Client));
	packSequence(outData, len);
	return this->NoErr;
}

void* CodeC::msgDecode(char * inData, int inLen){
	if (message != NULL) { memset(message, 0x00, sizeof(AskMessage)); }
	unpackSequence(inData, inLen);
	readHeadNode(message->num);
	readNextNode(message->Clientid);
	readNextNode(message->Toserverid);
	readNextNode(message->AuthenticationCode);
	readNextNode(message->Randnum_Client);
	FreeSequence();
	return (void *)message;
}

CodeC::~CodeC(){
	if (message != NULL) {
		delete message;
		message = NULL;
	}
	return;
}

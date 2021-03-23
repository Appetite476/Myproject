#define  _CRT_SECURE_NO_WARNINGS 
#include "CodeS.h"
#include <iostream>
#include <string.h>
using namespace std;

CodeS::CodeS(){
	message = new AnswerMessage;
	memset(message, 0x00, sizeof(AnswerMessage));
}

CodeS::CodeS(AnswerMessage * m){
	message = new AnswerMessage;
	memset(message, 0x00, sizeof(AnswerMessage));
	message->num = m->num;
	memcpy(message->Toclientid, m->Toclientid, strlen(m->Toclientid));
	memcpy(message->serverid, m->serverid, strlen(m->serverid));
	memcpy(message->Randnum_Server, m->Randnum_Server, strlen(m->Randnum_Server));
}

int CodeS::msgEncode(char ** outData, int & len) {
	writeHeadNode(message->num);
	writeNextNode(message->Toclientid, strlen(message->Toclientid));
	writeNextNode(message->serverid, strlen(message->serverid));
	writeNextNode(message->Randnum_Server, strlen(message->Randnum_Server));
	packSequence(outData, len);
	return this->NoErr;
}

void* CodeS::msgDecode(char * inData, int inLen) {
	if (message != NULL) { memset(message, 0x00, sizeof(AnswerMessage)); }
	unpackSequence(inData, inLen);
	readHeadNode(message->num);
	readNextNode(message->Toclientid);
	readNextNode(message->serverid);
	readNextNode(message->Randnum_Server);
	FreeSequence();
	return (void *)message;
}

CodeS::~CodeS() {
	if (message != NULL) {
		delete message;
		message = NULL;
	}
	return;
}
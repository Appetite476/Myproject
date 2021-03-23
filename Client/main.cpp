#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "Client.h"
#include "CodeC.h"

using namespace std;

int main(){
	srand((unsigned int)time(NULL));
	int cmd;
	AskMessage ask;
	Client c;
	c.ShowMenu();
	while(1){
		memset(&ask, 0x00, sizeof(AskMessage));
		cout << "> cmd: ";
		cin >> cmd;
		if(cmd == 0){
			break;
		}
		ask.num = cmd;
		cout << ask.num <<endl;
		switch(cmd){
		case 1:
			c.Secret_key_Negotiated(&ask);
			break;
		case 2:
			c.Secret_key_Check();
			break;
		case 3:
			c.Secret_key_Cancel();
			break;
		case 4:
			c.Secret_key_Watch();
			break;
		default:
			cout << "输入错误 请根据表中参数进行输入..." << endl;
			break;
		}
	}

	return 0;
}
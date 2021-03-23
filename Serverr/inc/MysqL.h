#pragma once
#include <iostream>
#include <mysql++/mysql++.h>

using namespace std;
using namespace mysqlpp;

extern const char* SearchFormat;
extern const char* InsertFormat;
class Mysql{
public:
	Mysql();
	
	bool Connect_DB(char *DB, char *IP, char *user, char *password);
	
	void Disconnect_DB();
	
	bool Insert_SecInfo(const char *clientid, unsigned char *seckey);
	
	bool Check_Client(char *clientid);
	
	int Next_Id_Seckeyinfo();
	
	~Mysql();
private:
	Connection conn;
};
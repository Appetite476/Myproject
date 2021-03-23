#include "MysqL.h"
#include "ItcastLog.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <mysql++/mysql++.h>

using namespace std;
using namespace mysqlpp;

const char* SearchFormat = "select * from secnode";
const char* InsertFormat = "insert into seckeyinfo(clientid, serverid\
, createtime, state, seckey) values(%d, %d, curdate(), 1, \"%s\");";

Mysql::Mysql():conn(false){}

bool Mysql::Connect_DB(char *DB, char *IP, char *user, char *password){
	if(DB == NULL || IP == NULL || user == NULL || password == NULL){
		ItcastLog().Log(__FILE__, __LINE__, ItcastLog::ERROR, -1,
		"func Connect_DB err, Check char *DB, char *IP, char *user, char *password");
		return false;
	}
	
	if (!conn.connect(DB, IP, user, password)){
		ItcastLog().Log(__FILE__, __LINE__, ItcastLog::WARNING, 0,
		"connect to DB falied");
		return false;
	}
	//cout << "连接数据库成功..." << endl;
	return true;
}

bool Mysql::Insert_SecInfo(const char *clientid, unsigned char *seckey){
	if(clientid == NULL || seckey == NULL){
		ItcastLog().Log(__FILE__, __LINE__, ItcastLog::ERROR, -1,
		"func Insert_SecInfo err, Check char *clientid, char *seckey");
		return false;
	}
	char Insert[1024] = {0};
	sprintf(Insert, InsertFormat, atoi(clientid), 1, seckey);
	cout << Insert << endl;
	Query query = conn.query();
	query << Insert;
	SimpleResult res = query.execute();
	//Report successful insertion
	cout << "Inserted into seckeyinfo table, ID =" << res.insert_id() << endl;
	cout << endl;
	return true;
}

bool Mysql::Check_Client(char *clientid){
	Query query = conn.query();
	query << SearchFormat;
	StoreQueryResult res = query.store();
	cout << "res.num_rows() = " << res.num_rows() << endl;
	for (size_t i = 0; i < res.num_rows(); i++)
	{
		/*cout << "id: " << res[i]["id"] << "\t - Name: " << res[i]["name"] \
		<< "\t - nodedesc: " << res[i]["nodedesc"] << "\t - createtime" 
		<< res[i]["createtime"] << "\t - authcode: " << res[i]["authcode"]
		<< "\t - state: " << res[i]["state"] << endl;*/
		if(atoi(res[i]["id"]) == atoi(clientid) && atoi(res[i]["state"]) == 1){
			return true;
		}
	}
	return false;
}

void Mysql::Disconnect_DB(){
	conn.disconnect();
}

int Mysql::Next_Id_Seckeyinfo(){
	Query query = conn.query();
	query << "SELECT auto_increment FROM information_schema.`TABLES` WHERE TABLE_SCHEMA='wjy' AND TABLE_NAME='seckeyinfo';";
	StoreQueryResult res = query.store();
	return atoi(res[0]["auto_increment"]);
}

Mysql::~Mysql(){}
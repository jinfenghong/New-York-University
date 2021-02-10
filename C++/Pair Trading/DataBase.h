//
//  DataBase.h
//  FRE7831_MarketData_JSON_MAC
//
//  Created by Xiang Zou on 4/23/20.
//  Copyright © 2020 NYU. All rights reserved.
//

#ifndef DataBase_h
#define DataBase_h

#include <sqlite3.h>
#include <string>
#include <map>
using namespace std;
class Stock;
int DisplayBacktest(sqlite3* db, string sqlSelect);
int OpenDatabase(sqlite3*& db);
int CreateTables(sqlite3*& db,const char *sqlCreateTable);
int InsertTable(sqlite3* db, const Stock& stock,std::string TableName);
int DisplayTables(sqlite3* db, const char *sqlSelect);
int CreatPairTable(sqlite3* db);
int CreatPairPricesTable(sqlite3* db);
int PopulateTable(sqlite3* db, map<string, Stock> stockMap1, map<string, Stock> stockMap2, vector<string> pair1, vector<string> pair2);
int PopulateTable(sqlite3* db, vector<string> pair1, vector<string> pair2);
int PopulateTable(sqlite3* db);
void DropTables(sqlite3* db);
#endif /* DataBase_h */

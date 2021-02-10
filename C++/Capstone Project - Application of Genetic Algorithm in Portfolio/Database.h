#pragma once
#include <sqlite3.h> 
#include <string> 
#include <vector>
#include <map>
#include "Stock.h"
using namespace std;

int OpenDatabase(const char* name, sqlite3*& db);
void CloseDatabase(sqlite3* db);
int DropTable(const char* sql_drop_table, sqlite3* db);
int CreateTable(const char* sql_create_table, sqlite3* db);
int InsertTable(const char* sql_insert, sqlite3* db);

int DisplayTable(const char* sql_select, sqlite3* db);

int RetrieveOneColData(const char* sql_select, sqlite3* db, vector<string>& info);
int RetrieveOneColData_double(const char* sql_select, sqlite3* db, vector<double>& prices);

int CSVtoDB(sqlite3* db);

int FastestInsertion_Stocks(vector<Trade>& trades, sqlite3* db);
int FastestInsertion_FundmentalData(vector<FundamentalData>& fundInfo, sqlite3* db);


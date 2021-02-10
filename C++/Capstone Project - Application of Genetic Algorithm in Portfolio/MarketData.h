#pragma once
#include <iostream>
#include <string>
#include <stdio.h>

#include "json/json.h"
#include "curl/curl.h"
#include <sqlite3.h>
#include "Stock.h"

using namespace std;

int RetrieveMarketData(string url_request, Json::Value& root, CURL* myHandle);
int PopulateStockTable(const Json::Value& root, string symbol, sqlite3* db);
int PopulateSP500Table(sqlite3* db);

int Download_and_Parse_Stocks(vector<string>& symbols, vector<Trade>& trades);
int Download_and_Parse_FundamentalData(vector<string>& symbols, vector<FundamentalData>& fundInfo);

void MultiThreadRetrive(const vector<string>& symbols, sqlite3* db, int choice = 1);
int PopulateStockTable_thread(const Json::Value& root, string symbol, sqlite3* db);
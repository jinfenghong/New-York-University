#pragma once
#include "TradeAlgorithm.h"
#include "Stock.h"
#include "DataBase.h"
#include <map>



vector<double> STDEV(sqlite3* db);
int RetrieveDB(sqlite3* db, const vector<string>& pair1, const vector<string>& pair2, map<string, Stock>& pair1Stock, map<string, Stock>& pair2Stock);
vector<double> PL(map<string, Stock>& pair1Stock, map<string, Stock>& pair2Stock, vector <double>& total_PL, vector<vector<double>>& Pair_pl, const vector<string>& pair1, const vector<string>& pair2, const vector<double>& stdev);
int UpdateDB(sqlite3* db, const vector<double>& total_PL, const vector<string>& pair);
int UpdateDB(sqlite3* db, const vector<vector<double>>& pl, const vector<string>& pair, map<string, Stock> pair1Stock);



#pragma once
#include <iostream>
#include <string>
#include <stdio.h>

#include "json/json.h"
#include "curl/curl.h"
#include "Stock.h"

#include <sqlite3.h>

bool asc_sort(Portfolio& struct1, Portfolio& struct2);
int Roulette(double TotalFitness, vector<Portfolio>& portfolios);
int Crossover(vector<string>& offspring1, vector<string>& offspring2);
void Mutate(vector<string> offspring, const vector<string>& symbols);

Portfolio Strategies(sqlite3* db, vector<Stock>& stocks, map<string, int>& mymap, const vector<string>& valSymbols);
void BackTest(sqlite3* db, Portfolio& p);
void ProbationTest(sqlite3* db, Portfolio& p);
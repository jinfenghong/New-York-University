#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include <stdio.h>

#include "json/json.h"
#include "curl/curl.h"

#include <sqlite3.h>

#include "Database.h"
#include "MarketData.h"
#include "Strategies.h"
#include "Stock.h"
#include <thread>
#include <mutex>

const int NUM_OF_THREADS = 5;

using namespace std;


int main(void)
{
	const char* stockDB_name = "SP500.db";
	sqlite3* db = NULL;
	if (OpenDatabase(stockDB_name, db) == -1)
		return -1;

	// Vectors/maps for infomation storage from Table SP500
	vector<string> symbols;
	vector<FundamentalData> basicInfo;
	vector<Trade> trades;

	vector<Stock> stocks;
	map<string, int> mymap;
	vector<string> valSymbols;

	// Start Menu
	bool bCompleted = false;
	char selection;
	while (!bCompleted) {
		std::cout << endl;
		std::cout << "Menu" << endl;
		std::cout << "========" << endl;
		std::cout << "A|a - Create, Retreive, Populate tables SP500Table, GSPC.INDX, and US10Y.INDX" << endl;
		std::cout << "B|b - Display SP500Table/GSPC.INDX/US10Y.INDX" << endl;
		std::cout << "C|c - Read Tickers" << endl;
		std::cout << "D|d - Create, Retreive, Populate Table for All 505 Companies" << endl;
		std::cout << "E|e - Display 505 Companies" << endl;
		std::cout << "F|f - Create, Retreive, Populate Fundamental Data for All 505 Companies" << endl;
		std::cout << "G|g - Prepare for the Strategies" << endl;
		std::cout << "H|h - Create 50 Portfolios" << endl;
		std::cout << "W|w - Drop Tables" << endl;
		std::cout << "X|x - Exit" << endl << endl;

		std::cin.get(selection);
		std::cin.ignore();
		switch (selection) {
		case 'A':
		case 'a':
		{
			// -------------------- Part for SP500/GSPC/US10Y -------------------------------
			PopulateSP500Table(db);

			vector<string> indexes = { "GSPC", "US10Y" };
			MultiThreadRetrive(indexes, db, 0);
		}
		break;

		case 'B':
		case 'b':
		{
			string sp500_select_table = "SELECT * FROM SP500;";
			if (DisplayTable(sp500_select_table.c_str(), db) == -1)
				return -1;

			sp500_select_table = "SELECT * FROM GSPC;";
			if (DisplayTable(sp500_select_table.c_str(), db) == -1)
				return -1;

			sp500_select_table = "SELECT * FROM US10Y;";
			if (DisplayTable(sp500_select_table.c_str(), db) == -1)
				return -1;
		}
		break;

		case 'C':
		case 'c':
		{
			string sp500_select_symbol = "SELECT REPLACE(symbol,'.','') FROM SP500;";
			if (!symbols.empty()) {
				symbols.clear();
			}
			RetrieveOneColData(sp500_select_symbol.c_str(), db, symbols);
		}
		break;

		case 'D':
		case 'd':
		{
			// ----------------------------- Retrieve companies info from Table SP500 ------------------------------
			// create one table to contain all data
			string symbol = "DailyTrades";
			std::string stockDB_drop_table = "DROP TABLE IF EXISTS " + symbol + ";";
			if (DropTable(stockDB_drop_table.c_str(), db) == -1)
				return -1;

			string stockDB_create_table = "CREATE TABLE " + symbol
				+ "(symbol CHAR(20) NOT NULL,"
				+ "date CHAR(20) NOT NULL,"
				+ "open REAL NOT NULL,"
				+ "high REAL NOT NULL,"
				+ "low REAL NOT NULL,"
				+ "close REAL NOT NULL,"
				+ "adjusted_close REAL NOT NULL,"
				+ "volume UNSIGNED BIG INT NOT NULL,"
				+ "returns REAL NOT NULL);";

			if (CreateTable(stockDB_create_table.c_str(), db) == -1)
				return -1;

			Download_and_Parse_Stocks(symbols, trades);
			FastestInsertion_Stocks(trades, db);

			// create tables for each stock
			int slice = symbols.size() / NUM_OF_THREADS;
			vector< vector<string> > symbol_groups(NUM_OF_THREADS);
			for (int i = 0; i < NUM_OF_THREADS; i++) {
				int start_pos = i * slice;
				int end_pos = (i + 1) * slice;
				copy(symbols.begin() + start_pos, symbols.begin() + end_pos, std::back_inserter(symbol_groups[i]));
			}

			// Must initialize libcurl before any threads are started 
			curl_global_init(CURL_GLOBAL_ALL);

			std::thread threadObj0(MultiThreadRetrive, symbol_groups[0], db, 1);
			std::thread threadObj1(MultiThreadRetrive, symbol_groups[1], db, 1);
			std::thread threadObj2(MultiThreadRetrive, symbol_groups[2], db, 1);
			std::thread threadObj3(MultiThreadRetrive, symbol_groups[3], db, 1);
			std::thread threadObj4(MultiThreadRetrive, symbol_groups[4], db, 1);
			threadObj0.join();
			threadObj1.join();
			threadObj2.join();
			threadObj3.join();
			threadObj4.join();
			vector<string> temp = { "SPY" };
			MultiThreadRetrive(temp, db, 1);
		}
		break;

		case 'E':
		case 'e':
		{
			// ----------------------------- Display Tables --------------------------------------
			cout << "Give a symbol you want to display: " << endl;
			string stockDB_symbol;
			cin >> stockDB_symbol;
			std::cin.ignore();

			string stockDB_select_table = "SELECT * FROM DailyTrades WHERE symbol = \"" + stockDB_symbol + "\";";
			if (DisplayTable(stockDB_select_table.c_str(), db) == -1)
				return -1;
		}
		break;

		case 'F':
		case 'f':
		{
			// ----------------------------- Start create, retrieve, populate --------------------------------------
			string stockDB_symbol = "FundamentalData";
			std::string stockDB_drop_table = "DROP TABLE IF EXISTS " + stockDB_symbol + ";";
			if (DropTable(stockDB_drop_table.c_str(), db) == -1)
				return -1;

			string stockDB_create_table = "CREATE TABLE " + stockDB_symbol
				+ "(symbol CHAR(20) NOT NULL,"
				+ "EBITDA REAL NOT NULL,"
				+ "PERatio REAL NOT NULL,"
				+ "DividendShare REAL NOT NULL,"
				+ "DividendYield REAL NOT NULL,"
				+ "MarketCapMln REAL NOT NULL,"
				+ "Beta REAL NOT NULL,"
				+ "High52Week REAL NOT NULL,"
				+ "Low52Week REAL NOT NULL,"
				+ "MA50Day REAL NOT NULL,"
				+ "MA200Day REAL NOT NULL,"
				+ "SharesOutstanding UNSIGNED BIG INT NOT NULL);";

			if (CreateTable(stockDB_create_table.c_str(), db) == -1)
				return -1;

			//symbols = { "SPY" };
			Download_and_Parse_FundamentalData(symbols, basicInfo);

			FastestInsertion_FundmentalData(basicInfo, db);

		}
		break;

		case 'G':
		case 'g':
		{
			// ------------------------ 1. 选择有效股票 ----------------------------------------------------			
			string sp500_select_symbol = "SELECT symbol FROM DailyTrades WHERE date BETWEEN '2010-01-04' AND '2018-12-31' GROUP BY symbol HAVING COUNT(date) >= 2264*0.50 INTERSECT SELECT symbol FROM FundamentalData WHERE `PERatio` <> 0.0 AND `MarketCapMln` <> 0.0;";
			RetrieveOneColData(sp500_select_symbol.c_str(), db, valSymbols);

			int size = valSymbols.size(); // 479
			for (int i = 0; i < size; ++i) {
				mymap[valSymbols[i]] = i;
				Stock s(valSymbols[i]);
				s.setAll(db);
				stocks.push_back(s);
			}
		}
		break;

		case 'H':
		case 'h':
		{
			Portfolio p = Strategies(db, stocks, mymap, valSymbols);
			BackTest(db, p);
			ProbationTest(db, p);
		}
		break;

		case 'W':
		case 'w':
			// Drop tables
		{
			cout << "Please don't do that!!! Your database is treasure!\n";
			string drop_name;
			cout << "Type a table name you want to drop" << endl;
			cin >> drop_name;
			cin.ignore();
			std::string sp500_drop_table = "DROP TABLE IF EXISTS \`" + drop_name + "\`;";
			if (DropTable(sp500_drop_table.c_str(), db) == -1) {
				cout << "Fail to drop" << endl;
			}
			cout << "Successful dropping" << endl;
		}
		break;

		case 'X':
		case 'x':
			//Exit
			bCompleted = true;
			break;

		default:
			std::cout << "Invalid Selection. Please try Again." << endl;
		}//switch
		std::cout << endl << endl;

	}
	// Close Database
	CloseDatabase(db);

	return 0;
}

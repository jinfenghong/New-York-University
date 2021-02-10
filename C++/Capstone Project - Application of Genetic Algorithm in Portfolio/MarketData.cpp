#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include <stdio.h>

#include "json/json.h"
#include "curl/curl.h"

#include <sqlite3.h>
#include <mutex>
#include "Database.h"
#include "Stock.h"

using namespace std;

mutex curl_mutex;
//writing call back function for storing fetched values in memory
/*static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}*/
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	lock_guard<mutex> guard(curl_mutex);
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

int RetrieveMarketData(string url_request, Json::Value& root, CURL* myHandle)
{
	std::string readBuffer;

	//global initiliation of curl before calling a function
	curl_global_init(CURL_GLOBAL_ALL);

	//creating session handle
	CURLcode result;

	curl_easy_setopt(myHandle, CURLOPT_URL, url_request.c_str());
	curl_easy_setopt(myHandle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows; U; Windows NT 6.1; rv:2.2) Gecko/20110201");
	curl_easy_setopt(myHandle, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(myHandle, CURLOPT_SSL_VERIFYHOST, 0);
	//curl_easy_setopt(myHandle, CURLOPT_VERBOSE, 1); // This will print out the results

	curl_easy_setopt(myHandle, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(myHandle, CURLOPT_WRITEDATA, &readBuffer);
	result = curl_easy_perform(myHandle);

	// check for errors 
	if (result != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
			curl_easy_strerror(result));
	}
	else {
		//cout << readBuffer << endl;
		//json parsing
		Json::CharReaderBuilder builder;
		Json::CharReader* reader = builder.newCharReader();
		string errors;

		bool parsingSuccessful = reader->parse(readBuffer.c_str(), readBuffer.c_str() + readBuffer.size(), &root, &errors);
		if (not parsingSuccessful)
		{
			// Report failures and their locations in the document.
			cout << "Failed to parse JSON" << std::endl << readBuffer << errors << endl;
			return -1;
		}
		//std::cout << "\nSucess parsing json\n" << root << endl;
		//std::cout << "\nSucess parsing json\n";
	}
	return 0;
}

int PopulateStockTable(const Json::Value& root, string symbol, sqlite3* db)
{
	string date;
	float open, high, low, close, adjusted_close;
	int64_t volume;
	double dr;
	Stock myStock(symbol);
	int count = 0; // for id and counting numbers

	double temp;
	cout << "Inserting a value into a table ..." << endl;
	for (Json::Value::const_iterator itr = root.begin(); itr != root.end(); itr++)
	{
		adjusted_close = (*itr)["adjusted_close"].asFloat();
		close = (*itr)["close"].asFloat();
		date = (*itr)["date"].asString();
		high = (*itr)["hig"].asFloat();
		low = (*itr)["low"].asFloat();
		open = (*itr)["open"].asFloat();
		volume = (*itr)["volume"].asInt64();
		if (itr == root.begin()) {
			dr = 0;
			temp = adjusted_close;
		}
		else {
			dr = adjusted_close / temp - 1;
			temp = adjusted_close;
		}
		Trade aTrade(symbol, date, open, high, low, close, adjusted_close, volume, dr);
		myStock.addTrade(aTrade);

		count++;

		// Execute SQL
		char stockDB_insert_table[512];
		sprintf_s(stockDB_insert_table, "INSERT INTO `%s` (id, symbol, date, open, high, low, close, adjusted_close, volume) VALUES(%d, \"%s\", \"%s\", %f, %f, %f, %f, %f, %lu)", symbol.c_str(), count, symbol.c_str(), date.c_str(), open, high, low, close, adjusted_close, volume);
		if (InsertTable(stockDB_insert_table, db) == -1)
			return -1;
	}
	// get all data in myStock

	cout << "Inserted a value into the table." << endl;
	//cout << myStock;
	return 0;
}

int PopulateSP500Table(sqlite3* db)
{	
	std::string sp500_drop_table = "DROP TABLE IF EXISTS SP500;";
	if (DropTable(sp500_drop_table.c_str(), db) == -1)
		return -1;

	string sp500_create_table = "CREATE TABLE SP500 (id INT PRIMARY KEY NOT NULL, symbol CHAR(20) NOT NULL, name CHAR(20) NOT NULL, sector CHAR(20) NOT NULL);";

	if (CreateTable(sp500_create_table.c_str(), db) == -1)
		return -1;

	string sp500_data_request = "https://pkgstore.datahub.io/core/s-and-p-500-companies/constituents_json/data/8dcccae2a6de88b891fec045e4d6e2c8/constituents_json.json";
	Json::Value root;   // will contains the root value after parsing.

	CURL* curlHandle = curl_easy_init();
	if (RetrieveMarketData(sp500_data_request, root, curlHandle) == -1)
		return -1;
	curl_easy_cleanup(curlHandle);

	string name, symbol, sector;
	sqlite3_exec(db, "begin;", 0, 0, 0);
	sqlite3_stmt* stmt;
	string stockDB_insert_table = "INSERT INTO SP500 VALUES(?, ?, ?, ?)";

	if (sqlite3_prepare_v2(db, stockDB_insert_table.c_str(), stockDB_insert_table.length(), &stmt, 0) != SQLITE_OK)
	{
		cerr << "Could not prepare statement." << endl;
		return -1;
	}

	int count = 0;
	for (Json::Value::const_iterator itr = root.begin(); itr != root.end(); itr++)
	{
		symbol = (*itr)["Symbol"].asString();
		name = (*itr)["Name"].asString();
		sector = (*itr)["Sector"].asString();
		
		count++;

		sqlite3_reset(stmt);

		if (sqlite3_bind_int(stmt, 1, count) != SQLITE_OK) {
			cerr << "Could not bind count." << endl;
			return -1;
		}

		if (sqlite3_bind_text(stmt, 2, symbol.c_str(), symbol.length(), SQLITE_STATIC) != SQLITE_OK) {
			cerr << "Could not bind symbol." << endl;
			return -1;
		};

		if (sqlite3_bind_text(stmt, 3, name.c_str(), name.length(), SQLITE_STATIC) != SQLITE_OK) {
			cerr << "Could not bind name." << endl;
			return -1;
		}

		if (sqlite3_bind_text(stmt, 4, sector.c_str(), sector.length(), SQLITE_STATIC) != SQLITE_OK) {
			cerr << "Could not bind sector." << endl;
			return -1;
		}

		if (sqlite3_step(stmt) != SQLITE_DONE)
		{
			cerr << "Could not step (execute) stmt." << endl;
			return -1;
		}
	}
	sqlite3_finalize(stmt);
	sqlite3_exec(db, "commit;", 0, 0, 0);
	return 0;
}

int Download_and_Parse_Stocks(vector<string>& symbols, vector<Trade>& trades) {

	vector<string> errorSymbols;	// store symbols that cannot retrieve data online
	map<string, int> normalSymbols;	// store symbols that have the same length as SPY does

	string date;
	float open, high, low, close, adjusted_close;
	int64_t volume;
	double dr; // daily_return
	int no_of_stocks = 0;
	CURL* curlHandle = curl_easy_init();
	for (vector<string>::iterator itr = symbols.begin(); itr != symbols.end(); ++itr) {
		cout << ++no_of_stocks << " " << *itr << endl;
		// ------------------------------- size getting from JSON --------------------------------
		string symbol = *itr;

		string stock_url_common = "https://eodhistoricaldata.com/api/eod/";
		string stock_start_date = "2010-01-01";
		string stock_end_date = "2020-07-31";
		string api_token = "5ba84ea974ab42.45160048";
		string stockDB_data_request = stock_url_common + symbol + ".US?" +
			"from=" + stock_start_date + "&to=" + stock_end_date + "&api_token=" + api_token + "&period=d&fmt=json";

		Json::Value stockDB_root;   // will contains the root value after parsing.

		
		if (RetrieveMarketData(stockDB_data_request, stockDB_root, curlHandle) == -1) {
			errorSymbols.push_back(symbol);
			continue;
		}
		
		normalSymbols[symbol] = stockDB_root.size();

		double temp;
		for (Json::Value::const_iterator inner = stockDB_root.begin(); inner != stockDB_root.end(); inner++)
		{
			adjusted_close = (*inner)["adjusted_close"].asFloat();
			close = (*inner)["close"].asFloat();
			date = (*inner)["date"].asString();
			high = (*inner)["high"].asFloat();
			low = (*inner)["low"].asFloat();
			open = (*inner)["open"].asFloat();
			volume = (*inner)["volume"].asInt64();
			if (inner == stockDB_root.begin()) {
				dr = 0;
				temp = adjusted_close;
			} else {
				dr = adjusted_close / temp - 1;
				temp = adjusted_close;
			}

			Trade aTrade(symbol, date, open, high, low, close, adjusted_close, volume, dr);
			trades.push_back(aTrade);
		}
		

	}
	curl_easy_cleanup(curlHandle);

	fstream outFile1, outFile2;
	outFile1.open("errorSymbols.csv", ios::out);
	outFile2.open("normalSymbols.csv", ios::out);

	for (vector<string>::iterator itr = errorSymbols.begin(); itr != errorSymbols.end(); ++itr) {
		outFile1 << *itr << endl;
	}

	for (map<string, int>::iterator itr = normalSymbols.begin(); itr != normalSymbols.end(); ++itr) {
		outFile2 << itr->first << "," << itr->second << "\n";
	}

	outFile1.close();
	outFile2.close();

	return 0;
}

int Download_and_Parse_FundamentalData(vector<string>& symbols, vector<FundamentalData>& fundInfo) {
	// Highlights{}
	double ebitda;	// EBITDA
	double pe;	// PERatio
	double dividendShare; // DividendShare
	double dividendYield; // DividendYield
	double marketCapMln;	// MarketCapitalizationMln

	// Technicals{}
	double beta;	//Beta
	double high52Week; // 52WeekHigh
	double low52Week;	// 52WeekLow
	double ma50Day;	// 50DayMA
	double ma200Day;	// 200DayMA

	// SharesStats{}
	int64_t outstanding; // SharesOutstanding

	int no_of_stocks = 0;
	CURL* curlHandle = curl_easy_init();
	for (vector<string>::iterator itr = symbols.begin(); itr != symbols.end(); ++itr) {
		cout << ++no_of_stocks << " " << *itr << endl;
		// ------------------------------- size getting from JSON --------------------------------
		string symbol = *itr;

		string stockDB_data_request = "https://eodhistoricaldata.com/api/fundamentals/" + symbol + ".US?api_token=5ba84ea974ab42.45160048";

		Json::Value root;   // will contains the root value after parsing.
		
		if (RetrieveMarketData(stockDB_data_request, root, curlHandle) == -1) {
			continue;
		}

		// Highlights{}
		ebitda = root["Highlights"]["EBITDA"].asDouble();
		pe = root["Highlights"]["PERatio"].asDouble();
		dividendShare = root["Highlights"]["DividendShare"].asDouble();
		dividendYield = root["Highlights"]["DividendYield"].asDouble();
		marketCapMln = root["Highlights"]["MarketCapitalizationMln"].asDouble();

		// Technicals{}	
		beta = root["Technicals"]["Beta"].asDouble();
		high52Week = root["Technicals"]["52WeekHigh"].asDouble();
		low52Week = root["Technicals"]["52WeekLow"].asDouble();
		ma50Day = root["Technicals"]["50DayMA"].asDouble();
		ma200Day = root["Technicals"]["200DayMA"].asDouble();

		// SharesStats{}
		outstanding = root["SharesStats"]["SharesOutstanding"].asInt64();

		FundamentalData aStock(symbol, ebitda, pe, dividendShare, dividendYield, marketCapMln, beta, high52Week, low52Week, ma50Day, ma200Day, outstanding);
		fundInfo.push_back(aStock);
	}
	curl_easy_cleanup(curlHandle);

	return 0;
}

int PopulateStockTable_thread(const Json::Value& root, string symbol, sqlite3* db)
{
	/*clock_t start = clock();

	sqlite3* db = NULL;
	if (OpenDatabase(name, db) == -1)
		return -1;*/

	string stockDB_drop_table;
	string stockDB_create_table;
	string stockDB_select_table;

	stockDB_drop_table = "DROP TABLE IF EXISTS \'" + symbol + "\';";
	if (DropTable(stockDB_drop_table.c_str(), db) == -1)
		return -1;

	stockDB_create_table = "CREATE TABLE \'" + symbol + "\'"
		+ "(id INT PRIMARY KEY NOT NULL,"
		+ "symbol CHAR(20) NOT NULL,"
		+ "date CHAR(20) NOT NULL,"
		+ "open REAL NOT NULL,"
		+ "high REAL NOT NULL,"
		+ "low REAL NOT NULL,"
		+ "close REAL NOT NULL,"
		+ "adjusted_close REAL NOT NULL,"
		+ "volume INT NOT NULL,"
		+ "returns REAL NOT NULL);";

	if (CreateTable(stockDB_create_table.c_str(), db) == -1)
		return -1;

	sqlite3_exec(db, "begin;", 0, 0, 0);
	sqlite3_stmt* stmt;
	string stockDB_insert_table = "INSERT INTO \'" + symbol + "\' VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

	if (sqlite3_prepare_v2(db, stockDB_insert_table.c_str(), stockDB_insert_table.length(), &stmt, 0) != SQLITE_OK)
	{
		cerr << "Could not prepare statement." << endl;
		return -1;
	}

	string date;
	float open = 0, high = 0, low = 0, close = 0, adjusted_close = 0;
	int64_t volume = 0;
	
	int count = 0;
	double dr = 0, temp = 0;

	for (Json::Value::const_iterator itr = root.begin(); itr != root.end(); itr++)
	{
		//std::cout << *itr << endl;
		date = (*itr)["date"].asString();
		open = (*itr)["open"].asFloat();
		high = (*itr)["high"].asFloat();
		low = (*itr)["low"].asFloat();
		close = (*itr)["close"].asFloat();
		adjusted_close = (*itr)["adjusted_close"].asFloat();
		volume = (*itr)["volume"].asInt64();
		if (itr == root.begin()) {
			temp = adjusted_close;
		}
		else {
			dr = adjusted_close / temp - 1;
			temp = adjusted_close;
		}

		count++;

		sqlite3_reset(stmt);

		if (sqlite3_bind_int(stmt, 1, count) != SQLITE_OK) {
			cerr << "Could not bind count." << endl;
			return -1;
		}

		if (sqlite3_bind_text(stmt, 2, symbol.c_str(), symbol.length(), SQLITE_STATIC) != SQLITE_OK) {
			cerr << "Could not bind symbol." << endl;
			return -1;
		};

		if (sqlite3_bind_text(stmt, 3, date.c_str(), date.length(), SQLITE_STATIC) != SQLITE_OK) {
			cerr << "Could not bind date." << endl;
			return -1;
		}

		if (sqlite3_bind_double(stmt, 4, open) != SQLITE_OK) {
			cerr << "Could not bind open." << endl;
			return -1;
		}

		if (sqlite3_bind_double(stmt, 5, high) != SQLITE_OK) {
			cerr << "Could not bind high." << endl;
			return -1;
		}

		if (sqlite3_bind_double(stmt, 6, low) != SQLITE_OK) {
			cerr << "Could not bind low." << endl;
			return -1;
		}

		if (sqlite3_bind_double(stmt, 7, close) != SQLITE_OK) {
			cerr << "Could not bind close." << endl;
			return 1;
		}

		if (sqlite3_bind_double(stmt, 8, adjusted_close) != SQLITE_OK) {
			cerr << "Could not bind adjusted_close." << endl;
			return 1;
		}

		if (sqlite3_bind_int64(stmt, 9, volume) != SQLITE_OK) {
			cerr << "Could not bind volume." << endl;
			return -1;
		}

		if (sqlite3_bind_double(stmt, 10, dr) != SQLITE_OK) {
			cerr << "Could not bind volume." << endl;
			return -1;
		}

		if (sqlite3_step(stmt) != SQLITE_DONE)
		{
			cerr << "Could not step (execute) stmt." << endl;
			return -1;
		}
	}
	sqlite3_finalize(stmt);
	sqlite3_exec(db, "commit;", 0, 0, 0);

	/*clock_t end = clock();

	CloseDatabase(db);
	cout << "Closed Stock database" << endl << endl;

	std::cout << "Time for insertion " << ((double)(end - start)) / CLOCKS_PER_SEC << " seconds" << endl;*/

	return 0;
}

void MultiThreadRetrive(const vector<string>& symbols, sqlite3* db, int choice = 1) //1-stock;otherwise index
{
	string stock_url_common = "https://eodhistoricaldata.com/api/eod/";
	string stock_start_date = "2010-01-01";
	string stock_end_date = "2020-07-31";
	string api_token = "5ba84ea974ab42.45160048";
	string stockDB_data_request;
	CURL* curlHandle = curl_easy_init();

	if (choice == 1) {
		for (auto itr = symbols.begin(); itr != symbols.end(); itr++) {
			stockDB_data_request = stock_url_common + *itr + ".US?" +
				"from=" + stock_start_date + "&to=" + stock_end_date + "&api_token=" + api_token + "&period=d&fmt=json";

			Json::Value stockDB_root;
			RetrieveMarketData(stockDB_data_request, stockDB_root, curlHandle);
			lock_guard<mutex> guard(curl_mutex);
			PopulateStockTable_thread(stockDB_root, *itr, db);
		}
	}
	else {
		for (auto itr = symbols.begin(); itr != symbols.end(); itr++) {
			stockDB_data_request = stock_url_common + *itr + ".INDX?" +
				"from=" + stock_start_date + "&to=" + stock_end_date + "&api_token=" + api_token + "&period=d&fmt=json";

			Json::Value stockDB_root;
			RetrieveMarketData(stockDB_data_request, stockDB_root, curlHandle);
			lock_guard<mutex> guard(curl_mutex);
			PopulateStockTable_thread(stockDB_root, *itr, db);
		}
	}
	//End a libcurl easy handle.This function must be the last function to call for an easy session
	curl_easy_cleanup(curlHandle);
}

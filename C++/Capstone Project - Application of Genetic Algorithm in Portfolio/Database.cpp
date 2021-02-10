#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdio.h>

#include "json/json.h"
#include "curl/curl.h"

#include <sqlite3.h>
#include "Stock.h"

#define NUM_OF_STOCKS 505

using namespace std;

int OpenDatabase(const char* name, sqlite3*& db)
{

	/* //None threading
	int rc = 0;
	// Open Database
	cout << "Opening database: " << name << endl;
	rc = sqlite3_open(name, &db);
	if (rc)
	{
		cerr << "Error opening SQLite3 database: " << sqlite3_errmsg(db) << endl;
		sqlite3_close(db);
		return -1;
	}
	cout << "Opened database: " << name << endl;
	return 0;*/

	int rc = 0;
	//char *error = NULL;
	// Open Database
	cout << "Opening database: " << name << endl;
	rc = sqlite3_open_v2(name, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL);
	if (rc != SQLITE_OK)
	{
		cerr << "Error opening SQLite3 database: " << sqlite3_errmsg(db) << endl;
		sqlite3_close(db);
		//system("pause");
		return -1;
	}
	cout << "Opened database: " << name << endl;
	sqlite3_exec(db, "PRAGMA synchronous = OFF; ", 0, 0, 0);
	return 0;
}

void CloseDatabase(sqlite3* db)
{
	cout << "Closing a database ..." << endl;
	sqlite3_close(db);
	cout << "Closed a database" << endl << endl;
}

int DropTable(const char* sql_drop_table, sqlite3* db)
{
	// Drop the table if exists
	if (sqlite3_exec(db, sql_drop_table, 0, 0, 0) != SQLITE_OK) { // or == -- same effect
		std::cout << "SQLite can't drop sessions table" << std::endl;
		sqlite3_close(db);
		return -1;
	}
	return 0;
}

int CreateTable(const char* sql_create_table, sqlite3* db)
{
	int rc = 0;
	char* error = NULL;
	// Create the table
	cout << "Creating a table..." << endl;
	rc = sqlite3_exec(db, sql_create_table, NULL, NULL, &error);
	if (rc)
	{
		cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << endl << endl;
		sqlite3_free(error);
		return -1;
	}
	cout << "Created a table." << endl;
	return 0;
}

int InsertTable(const char* sql_insert, sqlite3* db)
{
	int rc = 0;
	char* error = NULL;
	// Execute SQL
	//cout << "Inserting a value into a table ..." << endl;
	rc = sqlite3_exec(db, sql_insert, NULL, NULL, &error);
	if (rc)
	{
		cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << endl << endl;
		sqlite3_free(error);
		return -1;
	}
	//cout << "Inserted a value into the table." << endl;
	return 0;
}

int DisplayTable(const char* sql_select, sqlite3* db)
{
	int rc = 0;
	char* error = NULL;

	// Display MyTable
	cout << "Retrieving values in a table ..." << endl;
	char** results = NULL;
	int rows, columns;
	// A result table is memory data structure created by the sqlite3_get_table() interface.
	// A result table records the complete query results from one or more queries.
	sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);
	if (rc)
	{
		cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << endl << endl;
		sqlite3_free(error);
		return -1;
	}

	// Display Table
	for (int rowCtr = 0; rowCtr <= rows; ++rowCtr)
	{
		for (int colCtr = 0; colCtr < columns; ++colCtr)
		{
			// Determine Cell Position
			int cellPosition = (rowCtr * columns) + colCtr;

			// Display Cell Value
			cout.width(12);
			cout.setf(ios::left);
			if (results[cellPosition] == NULL) {
				cout << "NULL" << " ";
			}
			else {
				cout << results[cellPosition] << " ";
			}
			
		}

		// End Line
		cout << endl;

		// Display Separator For Header
		if (0 == rowCtr)
		{
			for (int colCtr = 0; colCtr < columns; ++colCtr)
			{
				cout.width(12);
				cout.setf(ios::left);
				cout << "~~~~~~~~~~~~ ";
			}
			cout << endl;
		}
	}
	// This function properly releases the value array returned by sqlite3_get_table()
	sqlite3_free_table(results);
	return 0;
}

int RetrieveOneColData(const char* sql_select, sqlite3* db, vector<string>& info) {
	int rc = 0;
	char* error = NULL;

	char** results = NULL;
	int rows, columns;
	// A result table is memory data structure created by the sqlite3_get_table() interface.
	// A result table records the complete query results from one or more queries.
	sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);
	if (rc)
	{
		cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << endl << endl;
		sqlite3_free(error);
		return -1;
	}

	// Assign values into vector
	for (int rowCtr = 1; rowCtr <= rows; ++rowCtr)
	{
		info.push_back(results[rowCtr]);
	}
	// This function properly releases the value array returned by sqlite3_get_table()
	sqlite3_free_table(results);
	return 0;
}

int RetrieveOneColData_double(const char* sql_select, sqlite3* db, vector<double>& prices) {
	int rc = 0;
	char* error = NULL;

	//cout << "Retrieving values in a table ..." << endl;
	char** results = NULL;
	int rows, columns;
	// A result table is memory data structure created by the sqlite3_get_table() interface.
	// A result table records the complete query results from one or more queries.
	sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);
	if (rc)
	{
		cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << endl << endl;
		sqlite3_free(error);
		return -1;
	}

	// Assign values into vector
	for (int rowCtr = 1; rowCtr <= rows; ++rowCtr)
	{
		prices.push_back(stod(results[rowCtr]));
	}
	// This function properly releases the value array returned by sqlite3_get_table()
	sqlite3_free_table(results);
	return 0;
}

int CSVtoDB(sqlite3* db) {
	fstream fin;
	fin.open("SP500Info.csv", ios::in);
	string line, word;
	string symbol, name, sector;

	// skip the title row
	getline(fin,line);

	sqlite3_exec(db, "begin;", 0, 0, 0);
	sqlite3_stmt* stmt;
	const char* sql = "INSERT INTO SP500 values(?,?,?)";
	sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, 0);

	cout << "Inserting Start" << endl;
	getline(fin, line);
	while (!fin.eof()) {
		istringstream s(line);
		getline(s, word, ',');
		symbol = word;
		getline(s, word, ',');
		name = word;
		getline(s, word, ',');
		sector = word;
		
		sqlite3_bind_text(stmt, 1, symbol.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 3, sector.c_str(), -1, SQLITE_STATIC);

		sqlite3_step(stmt);
		sqlite3_reset(stmt);
		getline(fin, line);
	}
	sqlite3_finalize(stmt);
	sqlite3_exec(db, "commit;", 0, 0, 0);

	cout << "Inserting finished" << endl;
	fin.close();

	return 0;
}

int FastestInsertion_Stocks(vector<Trade>& trades, sqlite3* db) {
	sqlite3_exec(db, "begin;", 0, 0, 0);
	sqlite3_stmt* stmt;
	const char* sql = "INSERT INTO DailyTrades values(?,?,?,?,?,?,?,?,?)";
	sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, 0);
	int size = trades.size();

	cout << "Inserting Start" << endl;
	int counting = 0;
	string temp = " ", symbol = " ";
	string date;
	for (int i = 0; i < size; ++i) {
		symbol = trades[i].getSymbol();
		if (temp != symbol) {
			cout << symbol << ": " << ++counting << endl;
			temp = symbol;
		}

		date = trades[i].getDate();

		sqlite3_bind_text(stmt, 1, symbol.c_str(), symbol.length(), SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, date.c_str(), date.length(), SQLITE_STATIC);
		sqlite3_bind_double(stmt, 3, trades[i].getOpen());
		sqlite3_bind_double(stmt, 4, trades[i].getHigh());
		sqlite3_bind_double(stmt, 5, trades[i].getLow());
		sqlite3_bind_double(stmt, 6, trades[i].getClose());
		sqlite3_bind_double(stmt, 7, trades[i].getAdjustedClose());
		sqlite3_bind_int64(stmt, 8, trades[i].getVolume());
		sqlite3_bind_double(stmt, 9, trades[i].getDR());

		sqlite3_step(stmt);
		sqlite3_reset(stmt);
	}
	sqlite3_finalize(stmt);
	sqlite3_exec(db, "commit;", 0, 0, 0);

	cout << "Inserting finished" << endl;
	return 0;
}

int FastestInsertion_FundmentalData(vector<FundamentalData>& fundInfo, sqlite3* db) {
	sqlite3_exec(db, "begin;", 0, 0, 0);
	sqlite3_stmt* stmt;
	const char* sql = "INSERT INTO FundamentalData values(?,?,?,?,?,?,?,?,?,?,?,?)";
	sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, 0);
	int size = fundInfo.size();

	std::cout << "Inserting Start" << endl;
	int counting = 0;

	string s;
	const char* symbol;

	for (int i = 0; i < size; ++i) {
		s = fundInfo[i].getSymbol();
		symbol = s.c_str();

		std::cout << s << ": " << ++counting << endl;

		sqlite3_bind_text(stmt, 1, symbol, -1, SQLITE_STATIC);
		sqlite3_bind_double(stmt, 2, fundInfo[i].getEBITDA());
		sqlite3_bind_double(stmt, 3, fundInfo[i].getPE());
		sqlite3_bind_double(stmt, 4, fundInfo[i].getDividendShare());
		sqlite3_bind_double(stmt, 5, fundInfo[i].getDividendYield());
		sqlite3_bind_double(stmt, 6, fundInfo[i].getMarketCapMln());
		sqlite3_bind_double(stmt, 7, fundInfo[i].getBeta());
		sqlite3_bind_double(stmt, 8, fundInfo[i].getHigh52Week());
		sqlite3_bind_double(stmt, 9, fundInfo[i].getLow52Week());
		sqlite3_bind_double(stmt, 10, fundInfo[i].getMA50Day());
		sqlite3_bind_double(stmt, 11, fundInfo[i].getMA200Day());
		sqlite3_bind_int64(stmt, 12, fundInfo[i].getOutstanding());

		sqlite3_step(stmt);
		sqlite3_reset(stmt);
	}
	sqlite3_finalize(stmt);
	sqlite3_exec(db, "commit;", 0, 0, 0);

	std::cout << "Inserting finished" << endl;
	return 0;
}
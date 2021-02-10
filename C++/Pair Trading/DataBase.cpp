//
//  DataBase.cpp
//  FRE7831_MarketData_JSON_MAC
//
//  Created by Xiang Zou on 4/23/20.
//  Copyright © 2020 NYU. All rights reserved.
//

#include <stdio.h>
#include <stdio.h>
#include "json/json.h"
#include "curl/curl.h"
#include "DataBase.h"
#include <string>
#include <iostream>
#include "Stock.h"
using namespace std;

int PopulateTable(sqlite3* db, map<string, Stock> stockMap1, map<string, Stock> stockMap2, vector<string> pair1, vector<string> pair2)
{
    for (int i = 0; i < pair1.size(); ++i)
    {
        InsertTable(db, stockMap1[pair1[i]], "PairOnePrices");
        InsertTable(db, stockMap2[pair2[i]], "PairTwoPrices");
    }
    /*
    for (map<string,Stock>::iterator it = stockMap1.begin();it!=stockMap1.end();++it)
    {
        InsertTable(db, it->second, "PairOnePrices");
    }
    for (map<string,Stock>::iterator it = stockMap2.begin();it!=stockMap2.end();++it)
    {
        InsertTable(db, it->second, "PairTwoPrices");
    }
     */
    return 0;
}
int PopulateTable(sqlite3* db, vector<string> pair1, vector<string> pair2)
{
    int rc = 0;
    char* error = NULL;
    for (int idx = 0; idx < pair1.size(); ++idx)
    {
        char sqlInsert[512];
        sprintf(sqlInsert, "INSERT INTO StockPairs(id, Symbol1, Symbol2, Volatility, profit_loss) VALUES(%d, \"%s\",\"%s\", %f, %f)", idx, pair1[idx].c_str(), pair2[idx].c_str(), 0.0, 0.0);
        rc = sqlite3_exec(db, sqlInsert, NULL, NULL, &error);
        if (rc)
        {
            cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << endl << endl;
            sqlite3_free(error);
        }
        else
        {
            cout << "Inserted a value into the table StockPairs." << endl << endl;
        }

    }
    return 0;
}

int PopulateTable(sqlite3* db)
{
    int rc = 0;
    char* error = NULL;
    const char* sqlInsert = "Insert into PairPrices " \
        "Select StockPairs.symbol1 as symbol1, StockPairs.symbol2 as symbol2,"\
        "PairOnePrices.date as date,"\
        "PairOnePrices.open as open1, PairOnePrices.close as close1,"\
        "PairTwoPrices.open as open2, PairTwoPrices.close as close2,"\
        "0 as profit_loss "\
        "From StockPairs, PairOnePrices, PairTwoPrices "\
        "Where (((StockPairs.symbol1 = PairOnePrices.symbol) and "\
        "(StockPairs.symbol2 = PairTwoPrices.symbol)) and "\
        "(PairOnePrices.date = PairTwoPrices.date)) "\
        "ORDER BY symbol1, symbol2;";
    rc = sqlite3_exec(db, sqlInsert, NULL, NULL, &error);
    if (rc)
    {
        cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << endl << endl;
        sqlite3_free(error);
    }
    else
    {
        cout << "Inserted a value into the table PairPrices." << endl << endl;
    }

    return 0;
}

void DropTables(sqlite3* db) {
    int rc;
    char* error;
    char** results = NULL;
    int rows, columns;
    string sqltablename = "select name from sqlite_master where type='table' order by name";
    string sql_dropatable = "DROP TABLE IF EXISTS ";
    string sql_vacuum = "VACUUM";
    rc = sqlite3_get_table(db, sqltablename.c_str(), &results, &rows, &columns, &error);

    if (rc)
    {
        cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << endl << endl;
        sqlite3_free(error);
    }
    else
    {
        for (int rowCtr = 1; rowCtr <= rows; ++rowCtr)
        {
            for (int colCtr = 0; colCtr < columns; ++colCtr)
            {
                int cellPosition = (rowCtr * columns) + colCtr;
                string tname = results[cellPosition];
                if (sqlite3_exec(db, (sql_dropatable + tname + ";").c_str(), 0, 0, 0) != SQLITE_OK) {
                    cout << "SQLite can't drop " + tname + " table" << std::endl;
                }
            }
        }
    }
    sqlite3_free_table(results);
    rc = sqlite3_exec(db, sql_vacuum.c_str(), NULL, NULL, &error);
    if (rc)
    {
        cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << endl << endl;
        sqlite3_free(error);
    }
    else {
        cout << endl << "Vacuumed all the Space." << endl;
    }
}



int CreatPairTable(sqlite3* db)
{
    const char* sqlCreatePairs = "CREATE TABLE IF NOT EXISTS StockPairs (" \
        "id INT NOT NULL," \
        "Symbol1 CHAR(20) NOT NULL," \
        "Symbol2 CHAR(20) NOT NULL," \
        "Volatility REAL NOT NULL," \
        "profit_loss REAL NOT NULL," \
        "PRIMARY KEY(id));";
    const char* sqlCreatePairsone = "CREATE TABLE IF NOT EXISTS PairOnePrices (" \
        "Symbol CHAR(20) NOT NULL," \
        "Date CHAR(20) NOT NULL," \
        "Open REAL NOT NULL," \
        "High REAL NOT NULL," \
        "Low REAL NOT NULL," \
        "Close REAL NOT NULL," \
        "Adjusted_Close REAL NOT NULL," \
        "Volume INT NOT NULL," \
        "PRIMARY KEY(Symbol,Date));";
    const char* sqlCreatePairstwo = "CREATE TABLE IF NOT EXISTS PairTwoPrices (" \
        "Symbol CHAR(20) NOT NULL," \
        "Date CHAR(20) NOT NULL," \
        "Open REAL NOT NULL," \
        "High REAL NOT NULL," \
        "Low REAL NOT NULL," \
        "Close REAL NOT NULL," \
        "Adjusted_Close REAL NOT NULL," \
        "Volume INT NOT NULL," \
        "PRIMARY KEY(Symbol,Date));";
    CreateTables(db, sqlCreatePairs);
    CreateTables(db, sqlCreatePairsone);
    CreateTables(db, sqlCreatePairstwo);

    return 0;
}

int CreatPairPricesTable(sqlite3* db)
{
    const char* sqlCreatePairPrice = "CREATE TABLE IF NOT EXISTS PairPrices (" \
        "Symbol1 CHAR(20) NOT NULL," \
        "Symbol2 CHAR(20) NOT NULL," \
        "Date CHAR(20) NOT NULL," \
        "Open1 REAL NOT NULL," \
        "Close1 REAL NOT NULL," \
        "Open2 REAL NOT NULL," \
        "Close2 REAL NOT NULL," \
        "profit_loss REAL NOT NULL," \
        "PRIMARY KEY(Symbol1,Symbol2,Date),"\
        "Foreign KEY(Symbol1,Date) references PairOnePrices(Symbol1,Date)\nON DELETE CASCADE\n ON UPDATE CASCADE,"\
        "Foreign KEY(Symbol2,Date) references PairTwoPrices(Symbol1,Date)\nON DELETE CASCADE\n ON UPDATE CASCADE,"\
        "Foreign KEY(Symbol1,Symbol2) references StockPairs(Symbol1,Symbol2)\nON DELETE CASCADE\n ON UPDATE CASCADE);";
    CreateTables(db, sqlCreatePairPrice);
    return 0;
}


int DisplayBacktest(sqlite3* db, std::string sqlSelect) {
    // Display MyTable
    int rc;
    char* error;
    cout << "Retrieving values in table ..." << endl;
    char** results = NULL;
    int rows, columns;
    rc = sqlite3_get_table(db, sqlSelect.c_str(), &results, &rows, &columns, &error);

    if (rc)
    {
        cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << endl << endl;
        sqlite3_free(error);
    }
    else
    {
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
                cout << results[cellPosition] << " ";
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
    }
    sqlite3_free_table(results);

    return 1;
}
int OpenDatabase(sqlite3*& db)
{
    int rc = 0;
    cout << "Opening PairTrading.db ..." << endl;
    rc = sqlite3_open("PairTrading.db", &db);
    if (rc)
    {
        cerr << "Error opening SQLite3 database: " << sqlite3_errmsg(db) << endl << endl;
        sqlite3_close(db);
        return -1;
    }
    else
    {
        cout << "Opened PairTrading.db." << endl << endl;
    }
    return 0;
}

int CreateTables(sqlite3*& db,const char *sqlCreateTable)
{
    int rc = 0;
    char *error = NULL;

    rc = sqlite3_exec(db, sqlCreateTable, NULL, NULL, &error);
    if (rc)
    {
        cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << endl << endl;
        sqlite3_free(error);
    }
    else
    {
        cout << "Created wanted table." << endl << endl;
    }
    
    return 0;
}

int InsertTable(sqlite3* db, const Stock& stock,string TableName)
{
    int rc = 0;
    char *error = NULL;
    string symbol = stock.getSymbol();
    vector<TradeData> dailydata = stock.getTrades();
    cout << "Inserting value into table "+TableName << endl;
    for (vector<TradeData>::iterator it = dailydata.begin(); it!= dailydata.end(); ++it)
    {
        char sqlInsert[512];
        string date = it->getDate();
        float open = it->getOpen();
        float high = it->getHigh();
        float low = it->getLow();
        float close = it->getClose();
        float adjusted_close = it->getAdjClose();
        long volume = it->getVolume();
        sprintf(sqlInsert, "INSERT INTO %s(Symbol, Date, Open, High, Low, Close, Adjusted_Close, Volume) VALUES(\"%s\", \"%s\", %f, %f, %f, %f, %f, %d)",TableName.c_str(), symbol.c_str(), date.c_str(), open, high, low, close, adjusted_close, volume);
        rc = sqlite3_exec(db, sqlInsert, NULL, NULL, &error);
        if (rc)
        {
            cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << endl << endl;
            sqlite3_free(error);
        }
        else
        {
            cout << "Inserted a value into the table "+ TableName << endl << endl;
        }
    }
    
    
    return 0;
}

int DisplayTables(sqlite3* db, const char *sqlSelect)
{
    // Display MyTable
    char **results = NULL;
    char *error = NULL;
    int rows, columns;
    
    sqlite3_get_table(db, sqlSelect, &results, &rows, &columns, &error);
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
            cout << results[cellPosition] << " ";
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
    
    sqlite3_free_table(results);
    return 0;
}

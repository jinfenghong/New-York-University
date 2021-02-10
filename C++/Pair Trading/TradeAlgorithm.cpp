#pragma warning(disable:4267)
#pragma warning(disable:4244)
#include "TradeAlgorithm.h"
#include "Stock.h"
#include "MarketData.h"
#include "DataBase.h"
#include <map>
#include <fstream>
#include <numeric>
#include <math.h>
#include <cstdlib>
#include <thread>
using namespace std;

vector<double> STDEV(sqlite3* db) {
    string back_test_start_date = "2020-01-03"; // control the datetime range in 10 years

    // --------------------- Calculate and update StockPairs table ----------------------------
    cout << "Starting Calculating and Updating Volatility into StockPairs" << endl;
    string calculate_volatility_for_pair = string("UPDATE StockPairs SET Volatility =") +
        "(SELECT(AVG((Close1 / Close2) * (Close1 / Close2)) - AVG(Close1 / Close2) * AVG(Close1 / Close2)) as variance " +
        " FROM PairPrices WHERE StockPairs.symbol1 = PairPrices.symbol1 AND " +
        "StockPairs.symbol2 = PairPrices.symbol2 AND PairPrices.date <= '2020-01-03');";

    const char* sqlUpdate = calculate_volatility_for_pair.c_str();
    int rc = 0;
    char* error = NULL;
    rc = sqlite3_exec(db, sqlUpdate, NULL, NULL, &error);
    if (rc)
    {
        cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << endl << endl;
        sqlite3_free(error);
    }
    else
    {
        cout << "Updated volatility into the StockPairs table." << endl << endl;
    }

    // ----------------------- retrieve stdev from db -------------------------------------------
    vector<double> stdev;

    string sql = "SELECT Volatility from StockPairs;";
    const char* sqlSelect = sql.c_str();
    char** results = NULL;
    error = NULL;
    int rows, columns;

    sqlite3_get_table(db, sqlSelect, &results, &rows, &columns, &error);
    for (int rowCtr = 1; rowCtr <= rows; ++rowCtr) {
        stdev.push_back(atof(results[rowCtr]));
    }
    return stdev;
}


vector<double> PL(map<string, Stock>& pair1Stock, map<string, Stock>& pair2Stock, vector <double>& total_PL,
    vector<vector<double>>& Pair_pl, const vector<string>& pair1, const vector<string>& pair2, const vector<double>& stdev) {

    int size_ = pair1.size();
    for (int i = 0; i < size_; ++i) {
        vector<TradeData> trades1 = pair1Stock[pair1[i]].getTrades();
        vector<TradeData> trades2 = pair2Stock[pair2[i]].getTrades();
        
        // All we need is here:;
        vector<double> pl{ 0.0 };
        //pair.push_back(pair1[i]);
        double total = 0.0;

        // ---------------------------------- start -----------------------------------------------
        double k_ = 1;
        //double PL = 0.0;

        double benchmark = k_ * stdev[i];

        // N1 is traded at the price Open1d2 
        // N2 is traded at the price Open2d2
        double N2, N1 = 10000;

        int Tsize = trades1.size();
        for (int j = 0; j < Tsize - 1; ++j) {
            N2 = N1 * (trades1[j + 1].getOpen() / trades2[j + 1].getOpen());
            if (abs(trades1[j].getClose() / trades2[j].getClose() - trades1[j + 1].getOpen() / trades2[j + 1].getOpen()) > benchmark) {
                // Short the pair: 1st stock is short and 2nd is long
                pl.push_back(-N1 * (trades1[j + 1].getOpen() - trades1[j + 1].getClose()) + N2 * (trades2[j + 1].getOpen() - trades2[j + 1].getClose()));
            }
            else {
                // Long the pair: 1st stock is long and 2nd is short
                pl.push_back(N1 * (trades1[j + 1].getOpen() - trades1[j + 1].getClose()) - N2 * (trades2[j + 1].getOpen() - trades2[j + 1].getClose()));
            }
        }

        /*for (auto it : pl) {
            cout << it << endl;
        }
        cout << endl;*/

        Pair_pl.push_back(pl);
        total_PL.push_back(accumulate(pl.begin(), pl.end(), 0.0));

    }

    /*for (auto it : total_PL) {
        cout << it << endl;
    }*/

    return stdev;
}


int RetrieveDB(sqlite3* db, const vector<string>& pair1, const vector<string>& pair2, map<string, Stock>& pair1Stock, map<string, Stock>& pair2Stock) {
    cout << "Retrieving Starting" << endl;
    for (int i = 0; i < pair1.size(); ++i) {
        cout << pair1[i] << " " << pair2[i] << endl;
        Stock stock1(pair1[i]);
        Stock stock2(pair2[i]);
        string sql = string("SELECT * FROM PairPrices where Symbol1 = \"") + pair1[i] + "\";";
        const char* sqlSelect = sql.c_str();
        char** results = NULL;
        char* error = NULL;
        int rows, columns;

        sqlite3_get_table(db, sqlSelect, &results, &rows, &columns, &error);

        string symbol1, symbol2, date;
        double open1, open2, close1, close2;

        for (int rowCtr = 1; rowCtr <= rows; ++rowCtr)
        {
            int cellPosition = rowCtr * columns;    // Symbol, Date, Open, High, Low, Close, Adjusted_Close, Volume
            symbol1 = results[cellPosition + 0];
            symbol2 = results[cellPosition + 1];
            date = results[cellPosition + 2];
            open1 = atof(results[cellPosition + 3]);
            close1 = atof(results[cellPosition + 4]);
            open2 = atof(results[cellPosition + 5]);
            close2 = atof(results[cellPosition + 6]);

            TradeData aTrade1(date, open1, 0, 0, close1, 0, 0);
            stock1.addTrade(aTrade1);


            TradeData aTrade2(date, open2, 0, 0, close2, 0, 0);
            stock2.addTrade(aTrade2);

        }

        cout << stock1.getTrades().size() << " " << stock2.getTrades().size() << endl;

        pair1Stock[pair1[i]] = stock1;

        pair2Stock[pair2[i]] = stock2;

        sqlite3_free_table(results);
    }

    cout << "Retrieving finished" << endl;
    return 0;
}

int UpdateDB(sqlite3* db, const vector<double>& total_PL, const vector<string>& pair) {
    int size_ = total_PL.size();
    for (int i = 0; i < size_; ++i) {
        string sql = "update StockPairs set profit_loss = " + to_string(total_PL[i]) + " where Symbol1 = \"" + pair[i] + "\";";
        const char* sqlUpdate = sql.c_str();
        int rc = 0;
        char* error = NULL;
        rc = sqlite3_exec(db, sqlUpdate, NULL, NULL, &error);
        if (rc)
        {
            cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << endl << endl;
            sqlite3_free(error);
        }
        else
        {
            cout << "Updated a value into the table StockPairs." << endl << endl;
        }
    }
    return 0;
}

int UpdateDB(sqlite3* db, const vector<vector<double>>& pl, const vector<string>& pair, map<string, Stock> pair1Stock) {
    int size_ = pair.size();
    for (int i = 0; i < size_; ++i) {
        vector<string> dates;
        for (auto it : pair1Stock[pair[i]].getTrades()) {
            dates.push_back(it.getDate());
        }

        for (int j = 0; j < pl[i].size(); ++j) {
            string sql = "update PairPrices set profit_loss = " + to_string(pl[i][j]) + " where Date = \"" + dates[j] + "\";";
            const char* sqlUpdate = sql.c_str();
            int rc = 0;
            char* error = NULL;
            rc = sqlite3_exec(db, sqlUpdate, NULL, NULL, &error);
            if (rc)
            {
                cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << endl << endl;
                sqlite3_free(error);
            }
            else
            {
                cout << "Updated a value into the table PairPrices." << endl << endl;
            }
        }
    }
    return 0;
}

//
//  main.cpp
//  FRE7831_MarketData_JSON_MAC
//
//  Created by Xiang Zou on 4/24/20.
//  Copyright © 2020 NYU. All rights reserved.
//

#include "Stock.h"
#include "MarketData.h"
#include "DataBase.h"
#include <fstream>
#include "TradeAlgorithm.h"



int main(void)
{
    sqlite3* db = NULL;
    if (OpenDatabase(db) != 0)		return -1;
    char selection;

    vector<string> pair1;
    vector<string> pair2;
    map<string, Stock> pair1Stock;
    map<string, Stock> pair2Stock;
    string tmp;
    ifstream pairs("PairTrading.txt");
    vector<double> stdev;
    vector <double> TOTAL_PL;
    vector <vector<double>> pl;
    vector<vector<double>>temp;
    string mannulchoice;
    vector<string> mpair1;
    vector<string> mpair2;
    map<string, Stock> mpair1Stock;
    map<string, Stock> mpair2Stock;
    string sqlSelectPairPrices = "SELECT * FROM PairPrices ";
    string sqlSelectStockPairs = "SELECT * FROM StockPairs ";

    while (!pairs.eof()) {
        getline(pairs, tmp);
        string str;
        for (auto x : tmp)
        {
            if (x == ',')
            {
                pair1.push_back(str);
                str.erase();
            }
            else
            {
                if (x != '\r') str += x;
            }

        }
        pair2.push_back(str);

    }

    while (true)
    {
        std::cout << endl;
        std::cout << "Menu" << endl;
        std::cout << "========" << endl;
        std::cout << "A - Create and Populate Pair Table" << endl;
        std::cout << "B - Retrieve and Populate Historical Data for Each Stock" << endl;
        std::cout << "C - Create PairPrices Table" << endl;
        std::cout << "D - Calculate Violatility" << endl;
        std::cout << "E - Back Test" << endl;
        std::cout << "F - Calculate Profit and Loss for Each Pair" << endl;
        std::cout << "G - Mannul Testing" << endl;
        std::cout << "H - Drop All the Tables" << endl;
        std::cout << "X - Exit" << endl << endl;
        cin >> selection;
        switch (selection)
        {
        case 'A':
            CreatPairTable(db);
            break;
        case 'B':
            pair1Stock = RetrieveData(pair1);
            pair2Stock = RetrieveData(pair2);
            PopulateTable(db, pair1Stock, pair2Stock, pair1, pair2);
            PopulateTable(db, pair1, pair2);
            PopulateTable(db);
            break;
        case 'C':
            CreatPairPricesTable(db);
            break;
        case 'D':
            stdev = STDEV(db);
            break;
        case 'E':
            RetrieveDB(db, pair1, pair2, pair1Stock, pair2Stock);
            PL(pair1Stock, pair2Stock, TOTAL_PL, pl, pair1, pair2, stdev);
            temp.push_back(pl[0]);
            UpdateDB(db, pl, pair1, pair1Stock);
            DisplayBacktest(db, sqlSelectPairPrices);
            break;
        case 'F':
            UpdateDB(db, TOTAL_PL, pair1);
            DisplayBacktest(db, sqlSelectStockPairs);
            break;
        case 'G':
            cout << endl << "Please Choose one of the following pairs." << endl;
            for (int i = 0; i < pair1.size(); i++)  cout << i << ". " << pair1[i] << ", " << pair2[i] << endl;

            cin >> mannulchoice;
            
            mpair1.push_back(pair1[atoi(mannulchoice.c_str())]);
            mpair2.push_back(pair2[atoi(mannulchoice.c_str())]);
            /*mpair1Stock = RetrieveData(mpair1);
            mpair2Stock = RetrieveData(mpair2);
            PopulateTable(db, mpair1Stock, mpair2Stock, mpair1, mpair2);
            PopulateTable(db, mpair1, mpair2);
            PopulateTable(db);
            stdev = STDEV(db);
            RetrieveDB(db, mpair1, mpair2, mpair1Stock, mpair2Stock);

            PL(mpair1Stock, mpair2Stock, TOTAL_PL, pl, mpair1, mpair2, stdev);
   
            UpdateDB(db, TOTAL_PL, mpair1);
            
            UpdateDB(db, pl, mpair1, mpair1Stock);*/
            
            DisplayBacktest(db, sqlSelectPairPrices + "WHERE Symbol1 = '" + mpair1[atoi(mannulchoice.c_str())] + "' and Symbol2 = '" + mpair2[atoi(mannulchoice.c_str())] + "';");
            DisplayBacktest(db, sqlSelectStockPairs + "WHERE Symbol1 = '" + mpair1[atoi(mannulchoice.c_str())] + "' and Symbol2 = '" + mpair2[atoi(mannulchoice.c_str())] + "';");

            mpair1.clear();
            mpair2.clear();
            /*mpair2Stock.clear();
            mpair1Stock.clear();
            TOTAL_PL.clear();
            pl.clear();
            stdev.clear();*/

            break;
        case 'H':
            DropTables(db);
            break;
        
        case 'X':
            cout << "Closing Stocks.db ..." << endl;
            sqlite3_close(db);
            cout << "Closed Stocks.db" << endl << endl;
            cout << "Exit the Program" << endl;
            return 0;
        default: cout << selection << " is not a valid selection.\n";
            cout << endl;
        }
    }
}
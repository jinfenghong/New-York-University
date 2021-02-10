//
//  MarketData.h
//  FRE7831_MarketData_JSON_MAC
//
//  Created by Xiang Zou on 4/23/20.
//  Copyright © 2020 NYU. All rights reserved.
//

#ifndef MarketData_h
#define MarketData_h


#include <string>
#include <map>
using namespace std;
class Stock;
int PullMarketData(const std::string& url_request, std::string& read_buffer);
int PopulateDailyTrades(const std::string& read_buffer, Stock& stock);
int PopulateIntradayTrades(const std::string& read_buffer, Stock& stock);
map<string, Stock> RetrieveData(vector<string> symbols);
#endif /* MarketData_h */

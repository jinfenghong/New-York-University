//
//  MarketData.cpp
//  FRE7831_MarketData_JSON_MAC
//
//  Created by Xiang Zou on 4/23/20.
//  Copyright © 2020 NYU. All rights reserved.
//

#include <stdio.h>
#include "json/json.h"
#include "curl/curl.h"
#include "MarketData.h"
#include <string>
#include <iostream>
#include "Stock.h"
using namespace std;
map<string, Stock> RetrieveData(vector<string> symbols)
{
    map<string, Stock> stockMap;
    string start_date = "2010-01-01";
    string end_date = "2020-04-30";
    string api_token = "5ba84ea974ab42.45160048";
    for (vector<string>::iterator it = symbols.begin(); it != symbols.end(); ++it)
    {
        string sym = *it;
        if (stockMap.count(sym) == 0)
        {
            Stock my_stock(sym);
            string url_request_daily = "https://eodhistoricaldata.com/api/eod/" + sym + ".US?" + "from=" + start_date + "&to=" + end_date + "&api_token=" + api_token + "&period=d&fmt=json";
            string read_buffer;
            PullMarketData(url_request_daily, read_buffer);
            PopulateDailyTrades(read_buffer, my_stock);
            stockMap[sym] = my_stock;
        }
    }

    return stockMap;
}
//writing call back function for storing fetched values in memory
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
int PullMarketData(const std::string& url_request, std::string& read_buffer)
{
    //global initiliation of curl before calling a function
    curl_global_init(CURL_GLOBAL_ALL);
    
    //creating session handle
    CURL * handle;
    
    // Store the result of CURLís webpage retrieval, for simple error checking.
    CURLcode result;
    
    // notice the lack of major error-checking, for brevity
    handle = curl_easy_init();
    
    curl_easy_setopt(handle, CURLOPT_URL, url_request.c_str());
        
    //adding a user agent
    curl_easy_setopt(handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:74.0) Gecko/20100101 Firefox/74.0");
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0);
    
    // send all data to this function
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteCallback);
    
    // we pass our 'chunk' struct to the callback function
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &read_buffer);
    
    //perform a blocking file transfer
    result = curl_easy_perform(handle);
    
    return 0;
}

int PopulateDailyTrades(const std::string& readBuffer, Stock& myStock)
{
    Json::CharReaderBuilder builder;
    Json::CharReader* reader = builder.newCharReader();
    Json::Value root;   // will contains the root value after parsing.
    string errors;
    
    bool parsingSuccessful = reader->parse(readBuffer.c_str(), readBuffer.c_str() + readBuffer.size(), &root, &errors);
    if (not parsingSuccessful)
    {
        // Report failures and their locations in the document.
        cout << "Failed to parse JSON" << endl << readBuffer << errors << endl;
        return -1;
    }
    else
    {
        //cout << "\nSucess parsing json\n" << root << endl;
        string date;
        float open = 0, high = 0, low = 0, close = 0, adjusted_close = 0;
        int volume = 0;
        for (Json::Value::const_iterator itr = root.begin(); itr != root.end(); itr++)
        {
                //cout << *itr << endl;
                for (Json::Value::const_iterator inner = (*itr).begin(); inner != (*itr).end(); inner++)
                {
                    //cout << inner.key() << ": " << *inner << endl;
    
                    if (inner.key().asString() == "adjusted_close")
                    {
                        if (inner->isNull() || inner->isNumeric())
                            adjusted_close = inner->asFloat();
                        else
                            adjusted_close = (float)atof(inner->asCString());
                    }
                    else if (inner.key().asString() == "close")
                    {
                        if (inner->isNull() || inner->isNumeric())
                            close = inner->asFloat();
                        else
                            close = (float)atof(inner->asCString());
                    }
                    else if (inner.key() == "date")
                        date = inner->asString();
                    else if (inner.key().asString() == "high")
                    {
                        if (inner->isNull() || inner->isNumeric())
                            high = inner->asFloat();
                        else
                            high = (float)atof(inner->asCString());
                    }
                    else if (inner.key().asString() == "low")
                    {
                        if (inner->isNull() || inner->isNumeric())
                            low = inner->asFloat();
                        else
                            low = (float)atof(inner->asCString());
                    }
                    else if (inner.key() == "open")
                    {
                        if (inner->isNull() || inner->isNumeric())
                            open = inner->asFloat();
                        else
                            open = (float)atof(inner->asCString());
                    }
                    else if (inner.key().asString() == "volume")
                    {
                        if (inner->isNull() || inner->isNumeric())
                            volume = inner->asInt();
                        else
                            volume = atoi(inner->asCString());
                    }
                    else
                    {
                        cout << "Invalid json field" << endl;
                        return -1;
                    }
                }
                TradeData aTrade(date, open, high, low, close, adjusted_close, volume);
                myStock.addTrade(aTrade);
        }
    }

    return 0;


}


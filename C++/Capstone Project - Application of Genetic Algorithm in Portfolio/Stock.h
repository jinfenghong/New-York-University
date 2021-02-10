#pragma once
#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include <stdio.h>

#include <sqlite3.h>
#include <algorithm>

using namespace std;

class Trade
{
private:
	string symbol;
	string date;
	float open;
	float high;
	float low;
	float close;
	float adjusted_close;
	int64_t volume;
	double dr; // daily_return
public:
	Trade() {
		symbol = " ";
		date = " ";
		open = 0;
		high = 0;
		low = 0;
		close = 0;
		adjusted_close = 0;
		volume = 0;
		dr = 0;
	}
	Trade(string symbol_, string date_, float open_, float high_, float low_, float close_, float adjusted_close_, int64_t volume_, double dr_) :
		symbol(symbol_), date(date_), open(open_), high(high_), low(low_), close(close_), adjusted_close(adjusted_close_), volume(volume_), dr(dr_)
	{}
	~Trade() {}

	string getSymbol() {
		return symbol;
	}
	string getDate() {
		return date;
	}
	float getOpen() {
		return open;
	}
	float getHigh() {
		return high;
	}
	float getLow() {
		return low;
	}
	float getClose() {
		return close;
	}
	float getAdjustedClose() {
		return adjusted_close;
	}
	int64_t getVolume() {
		return volume;
	}
	double getDR() {
		return dr;
	}
	friend ostream& operator << (ostream& out, const Trade& t)
	{
		out << "symbol: " << t.symbol << "Date: " << t.date << " Open: " << t.open << " High: " << t.high << " Low: " << t.low << " Close: " << t.close << " Adjusted_Close: " << t.adjusted_close << " Volume: " << t.volume << endl;
		return out;
	}
};

class FundamentalData {
private:
	string symbol;
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

public:
	FundamentalData() {
		symbol = " ";
		ebitda = pe = dividendShare = dividendYield = marketCapMln = 0;
		beta = high52Week = low52Week = ma50Day = ma200Day = 0;
		outstanding = 0;
	}

	FundamentalData(string symbol_,
		double ebitda_, double pe_, double dividendShare_, double dividendYield_, double marketCapMln_,
		double beta_, double high52Week_, double low52Week_, double ma50Day_, double ma200Day_,
		int64_t outstanding_) :
		symbol(symbol_),
		ebitda(ebitda_), pe(pe_), dividendShare(dividendShare_), dividendYield(dividendYield_), marketCapMln(marketCapMln_),
		beta(beta_), high52Week(high52Week_), low52Week(low52Week_), ma50Day(ma50Day_), ma200Day(ma200Day_),
		outstanding(outstanding_) {}
	~FundamentalData() {}

	string getSymbol() {
		return symbol;
	}
	double getEBITDA() {
		return ebitda;
	}
	double getPE() {
		return pe;
	}
	double getDividendShare() {
		return dividendShare;
	}
	double getDividendYield() {
		return dividendYield;
	}
	double getMarketCapMln() {
		return marketCapMln;
	}
	double getBeta() {
		return beta;
	}
	double getHigh52Week() {
		return high52Week;
	}
	double getLow52Week() {
		return low52Week;
	}
	double getMA50Day() {
		return ma50Day;
	}
	double getMA200Day() {
		return ma200Day;
	}
	int64_t getOutstanding() {
		return outstanding;
	}

	void setSymbol(string symbol_) {
		symbol = symbol_;
	}
	void setEBITDA(double ebitda_) {
		ebitda = ebitda_;
	}
	void setPE(double pe_) {
		pe = pe_;
	}
	void setDividendShare(double ds) {
		dividendShare = ds;
	}
	void setDividendYield(double dy) {
		dividendYield = dy;
	}
	void setMarketCapMln(double mcmln) {
		marketCapMln = mcmln;
	}
	void setBeta(double beta_) {
		beta = beta_;
	}
	void setHigh52Week(double high52Week_) {
		high52Week = high52Week_;
	}
	void setLow52Week(double low52Week_) {
		low52Week = low52Week_;
	}
	void setMA50Day(double ma_) {
		ma50Day = ma_;
	}
	void setMA200Day(double ma_) {
		ma200Day = ma_;
	}
	void setOutstanding(int64_t outstanding_) {
		outstanding = outstanding_;
	}

	void setAll(sqlite3* db) {
		string sql_select = "SELECT * FROM FundamentalData WHERE symbol = \"" + symbol + "\";";
		int rc = 0;
		char* error = NULL;

		//cout << "Retrieving values in a table ..." << endl;
		char** results = NULL;
		int rows, columns;
		// A result table is memory data structure created by the sqlite3_get_table() interface.
		// A result table records the complete query results from one or more queries.
		sqlite3_get_table(db, sql_select.c_str(), &results, &rows, &columns, &error);
		if (rc) {
			cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << endl << endl;
			sqlite3_free(error);
		}

		symbol = results[columns];
		ebitda = stod(results[columns + 1]);
		pe = stod(results[columns + 2]);
		dividendShare = stod(results[columns + 3]);
		dividendYield = stod(results[columns + 4]);
		marketCapMln = stod(results[columns + 5]);
		beta = stod(results[columns + 6]);
		high52Week = stod(results[columns + 7]);
		low52Week = stod(results[columns + 8]);
		ma50Day = stod(results[columns + 9]);
		ma200Day = stod(results[columns + 10]);
		outstanding = stoll(results[columns + 11]);

		// This function properly releases the value array returned by sqlite3_get_table()
		sqlite3_free_table(results);
	}
	friend ostream& operator << (ostream& out, const FundamentalData& f)
	{
		out << "Symbol: " << f.symbol << endl;
		out << "EBITDA: " << f.ebitda << endl << "PE: " << f.pe << endl << "DividendShare: " << f.dividendShare << endl;
		out << "DividendYield: " << f.dividendYield << endl << "MarketCapMln: " << f.marketCapMln << endl;
		out << "Beta: " << f.beta << endl << "52WeekHigh: " << f.high52Week << endl << "52WeekLow: " << f.low52Week << endl;
		out << "50DayMA: " << f.ma50Day << endl << "200DayMA: " << f.ma200Day << endl << "OutStanding: " << f.outstanding << endl;
		return out;
	}
};

class GSPC {
private:
	GSPC() {}
	vector<double> returns;
public:
	static GSPC& GET() {
		static GSPC gspc;
		return gspc;
	}

	vector<double> setDailyReturns(vector<double> r) {
		returns = r;
		return returns;
	}

	static vector<double> setDailyReturns(sqlite3* db) {
		vector<double> dailyR;
		string sql_select = "SELECT adjusted_close FROM GSPC;";
		int rc = 0;
		char* error = NULL;
		char** results = NULL;
		int rows, columns;

		sqlite3_get_table(db, sql_select.c_str(), &results, &rows, &columns, &error);
		if (rc)
		{
			cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << endl << endl;
			sqlite3_free(error);
		}

		for (int rowCtr = 1; rowCtr <= rows; ++rowCtr)
		{
			dailyR.push_back(stod(results[rowCtr]));
		}

		sqlite3_free_table(results);

		return GSPC::GET().setDailyReturns(dailyR);
	}
};

class Stock
{
private:
	vector<Trade> trades;
	vector<double> daily_returns;

	string symbol;
	int size;	// will be set value in setDailyReturns
	FundamentalData basicData;
	double rf;	//risk free rate
	double volatility;
	double sharpeR;

public:
	Stock(string symbol_) {
		symbol = symbol_;
		rf = 0.0;
		basicData.setSymbol(symbol_);
		size = 0;
		volatility = 0;
		sharpeR = 0;
	}
	~Stock() {}

	void setAll(sqlite3* db) {
		setDailyReturns(db);
		setBasicData(db);
		setRF(db);
		setVolatility();
		setSharpeR();
	}
	void setDailyReturns(sqlite3* db, string sql_select = " ") {
		daily_returns.clear();
		if (sql_select == " ") {
			sql_select = "SELECT `returns` FROM `" + symbol + "`;";
		}
		int rc = 0;
		char* error = NULL;

		char** results = NULL;
		int rows, columns;
		// A result table is memory data structure created by the sqlite3_get_table() interface.
		// A result table records the complete query results from one or more queries.
		sqlite3_get_table(db, sql_select.c_str(), &results, &rows, &columns, &error);
		if (rc)
		{
			cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << endl << endl;
			sqlite3_free(error);
		}

		// Assign values into vector
		for (int rowCtr = 1; rowCtr <= rows; ++rowCtr)	//对零的部分要进行处理
		{
			daily_returns.push_back(stod(results[rowCtr]));
		}
		// This function properly releases the value array returned by sqlite3_get_table()
		sqlite3_free_table(results);

		size = daily_returns.size();
	}
	void setBasicData(sqlite3* db) {
		basicData.setAll(db);
	}
	void setRF(sqlite3* db) {
		string sql_select = "SELECT `adjusted_close` FROM US10Y WHERE `date` = (SELECT `date` FROM `" + symbol + "` LIMIT 1); ";
		int rc = 0;
		char* error = NULL;

		//cout << "Retrieving values in a table ..." << endl;
		char** results = NULL;
		int rows, columns;
		// A result table is memory data structure created by the sqlite3_get_table() interface.
		// A result table records the complete query results from one or more queries.
		sqlite3_get_table(db, sql_select.c_str(), &results, &rows, &columns, &error);
		if (rc)
		{
			cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << endl << endl;
			sqlite3_free(error);
		}

		rf = stod(results[1]) / 100.0 / 252.0;

		// This function properly releases the value array returned by sqlite3_get_table()
		sqlite3_free_table(results);
	}
	void setVolatility() {
		double total = 0.0;
		for (auto i : daily_returns)
			volatility += (i - rf) * (i - rf);
		volatility = sqrt(volatility / size);
	}
	void setSharpeR() {
		double meanR = 0.0;
		for (auto i : daily_returns) {
			meanR += i;
		}
		meanR = meanR / static_cast<double>(size);

		sharpeR = (meanR - rf) / volatility;
	}

	vector<double> getDailyReturns(sqlite3* db) {
		if (daily_returns.empty()) {
			setDailyReturns(db);
		}
		return daily_returns;
	}
	FundamentalData getBasicData() {
		return basicData;
	}
	int getSize() {
		return size;
	}
	double getRF() {
		return rf;
	}
	string getSymbol() {
		return symbol;
	}
	double getSharpeR() {
		return sharpeR;
	}
	double getVolatility() {
		return volatility;
	}

	void addTrade(Trade aTrade)
	{
		trades.push_back(aTrade);
	}
	friend ostream& operator << (ostream& out, const Stock& s)
	{
		out << "Symbol: " << s.symbol << endl;
		for (vector<Trade>::const_iterator itr = s.trades.begin(); itr != s.trades.end(); itr++)
			out << *itr;
		return out;
	}
};

class Portfolio {
private:
	// -------------------------- for portfolio's information ---------------------------------
	int id;
	vector<double> portReturns;
	vector<Stock> stocks;
	vector<string> symbols;
	vector<float> weights;

	//int min_;	// 最少的daily returns数量
	int size;

	// -------------------------- for portfolio measurement -------------------------------------
	double sharpeR;	// sharpe ratio
	double volatility;
	double meanR;	//geometric mean of return
	double rf; //risk-free rate, 注意：Stock也有risk-free rate, 可以不要了

	double portPE;
	double portBeta;
	double portDY; // dividend yield

	double fitness;

public:
	Portfolio() :id(-1), sharpeR(0), meanR(0), volatility(0), size(0), rf(1), portPE(0), portBeta(0), fitness(0), portDY(0) {}
	Portfolio(int id_) :id(id_), sharpeR(0), meanR(0), volatility(0), size(0), rf(1), portPE(0), portBeta(0), fitness(0), portDY(0) {}
	~Portfolio() {}

	void addStocks(Stock stock) {
		stocks.push_back(stock);
	}

	// -----------------------------------------------------------------------------
	void setAll(sqlite3* db) {
		setSymbols();
		//setSizes_and_StartPnts();	// get all sizes of stocks, and there starting point based on 短板效应
		setWeights(db);	// get all weights
		setPortReturns(db);
		setRF(db);
		setMeanR();
		setVolatility();
		setSharpeR();
		setPortPE();
		setPortBeta();
		setPortDY();
	}

	// ----------------------- following setting functions can be private ------------------
	void setSymbols() {
		for (auto i : stocks)
			symbols.push_back(i.getSymbol());
	}
	void setRF(sqlite3* db, string sql_select=" ") {
		if (sql_select == " ") {
			sql_select = "SELECT adjusted_close FROM US10Y WHERE `date` = (SELECT date From `";
			for (auto itr = symbols.begin(); itr != symbols.end() - 1; itr++) {
				sql_select += *itr + "` INTERSECT SELECT date From `";
			}
			sql_select += symbols[9] + "` LIMIT 1);";
		}

		int rc = 0;
		char* error = NULL;

		//cout << "Retrieving values in a table ..." << endl;
		char** results = NULL;
		int rows, columns;
		// A result table is memory data structure created by the sqlite3_get_table() interface.
		// A result table records the complete query results from one or more queries.
		sqlite3_get_table(db, sql_select.c_str(), &results, &rows, &columns, &error);
		if (rc)
		{
			cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << endl << endl;
			sqlite3_free(error);
		}

		rf = stod(results[1]) / 100.0 / 252.0;

		// This function properly releases the value array returned by sqlite3_get_table()
		sqlite3_free_table(results);

	}
	void setWeights(sqlite3* db) {
		string symbol_list = "(\"";
		for (auto itr = symbols.begin(); itr != symbols.end() - 1; ++itr) {
			symbol_list += *itr + "\",\"";
		}
		symbol_list += symbols[9] + "\")";

		string sql_select = "SELECT SP500.symbol, t.weight FROM SP500 INNER JOIN (SELECT s1.sector, s2.weights / s1.num weight FROM (SELECT sector, COUNT(*) AS num FROM SP500 WHERE symbol IN "
			+ symbol_list
			+ " GROUP BY sector) s1 INNER JOIN (SELECT t2.sector, SUM(MarketCapMln) / (SELECT SUM(MarketCapMln) FROM FundamentalData WHERE symbol IN "
			+ symbol_list
			+ ") as weights FROM FundamentalData t1 INNER JOIN SP500 t2 ON t1.symbol = t2.symbol WHERE t1.symbol IN "
			+ symbol_list
			+ " GROUP BY t2.sector) s2 ON s1.sector = s2.sector) t ON SP500.sector = t.sector WHERE symbol IN "
			+ symbol_list
			+ ";";

		int rc = 0;
		char* error = NULL;

		//cout << "Retrieving values in a table ..." << endl;
		char** results = NULL;
		int rows, columns;
		// A result table is memory data structure created by the sqlite3_get_table() interface.
		// A result table records the complete query results from one or more queries.
		sqlite3_get_table(db, sql_select.c_str(), &results, &rows, &columns, &error);
		if (rc) {
			cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << endl << endl;
			sqlite3_free(error);
		}

		map<string, double> symbolWeights;
		// Assign values into vector
		string symbol;
		double weight;
		for (int rowCtr = 1; rowCtr <= rows; ++rowCtr) {
			symbol = results[rowCtr * columns];
			weight = stod(results[rowCtr * columns + 1]);
			symbolWeights[symbol] = weight;
		}

		for (auto itr = symbols.begin(); itr != symbols.end(); ++itr) {
			weights.push_back(symbolWeights[*itr]);
		}

		// This function properly releases the value array returned by sqlite3_get_table()
		sqlite3_free_table(results);
		/*for (int i = 0; i < symbols.size(); i++)
			weights.push_back(0.1);*/
	}
	void setPortReturns(sqlite3* db, string start_date = "2010-01-04", string end_date = "2019-12-31" ) {
		string select1 = "SELECT returns FROM `";
		string select2 = "` WHERE date IN (SELECT date From `";
		for (auto itr = symbols.begin(); itr != symbols.end() - 1; itr++) {
			select2 += *itr + "` INTERSECT SELECT date From `";
		}
		select2 += symbols[9] + "`) AND date BETWEEN \'"+start_date+"\' AND \'"+end_date+"\';";

		string sql_select;

		vector<vector<double>> allReturns;
		for (int i = 0; i < 10; i++) {
			sql_select = select1 + symbols[i] + select2;
			stocks[i].setDailyReturns(db, sql_select);

			vector<double> temp = stocks[i].getDailyReturns(db);
			//所有的元素同乘一个数: weight * daily_returns from stock[i]
			allReturns.push_back(temp);
			std::for_each(allReturns[i].begin(), allReturns[i].end(), [&](double& el) {el *= weights[i]; });
		}

		size = stocks[0].getSize();	// all stock have the same return size

		// Calculate portfolio daily returns from 10 stocks
		for (int i = 0; i < size; i++) {
			double r = 0;
			for (int j = 0; j < 10; j++) {
				r += allReturns[j][i];
			}
			portReturns.push_back(r);
		}
	}
	void setMeanR() {
		for (auto i : portReturns) {
			meanR += i;
		}
		meanR = meanR / static_cast<double>(size);
	}
	void setVolatility() {
		for (auto i : portReturns) {
			volatility += (i - meanR) * (i - meanR);
		}
		volatility = sqrt(volatility / size);
	}
	void setSharpeR() {
		sharpeR = (meanR - rf) / volatility;
	}
	void setPortPE() {
		for (int i = 0; i < 10; i++) {
			portPE += stocks[i].getBasicData().getPE() * weights[i];
		}
	}
	void setPortBeta() {
		for (int i = 0; i < 10; i++) {
			portBeta += stocks[i].getBasicData().getBeta() * weights[i];
		}
	}
	void setPortDY() {
		for (int i = 0; i < 10; i++) {
			portDY += stocks[i].getBasicData().getDividendYield() * weights[i];
		}
	}
	void setFitness(double minBeta, double maxBeta, double minSR, double maxSR, double minPE, double maxPE, double  minDY, double maxDY, double medianVol) {
		fitness = 0.37 * (portBeta - minBeta) / (maxBeta - minBeta)
			+ 0.35 * (sharpeR - minSR) / (maxSR - minSR)
			+ 0.03 * (1 / portPE - minPE) / (maxPE - minPE)
			+ 0.1 * (portDY - minDY) / (maxDY - minDY)
			+ 0.15 * exp(-abs(volatility - medianVol));
	}
	void setID(int id_) {
		id = id_;
	}

	int getID() const {
		return id;
	}
	double getRF() const {
		return rf;
	}
	double getMeanR() const {
		return meanR;
	}
	double getVolatility() const {
		return volatility;
	}
	double getSharpeR() const {
		return sharpeR;
	}
	double getPE() const {
		return portPE;
	}
	double getBeta() const {
		return portBeta;
	}
	double getDY() const {
		return portDY;
	}
	vector<string> getSymbols() const {
		return symbols;
	}
	vector<float> getWeights() const {
		return weights;
	}
	double getFitness() const {
		return fitness;
	}
	vector<double> getPortReturns() {
		return portReturns;
	}

	friend ostream& operator << (ostream& out, const Portfolio& p) {
		out << "ID: " << p.id << endl;
		for (auto i : p.symbols)
			out << "\'" << i << "\',\'";
		out << endl;
		out << "PE: " << p.portPE << endl << "Dividend Yield: " << p.getDY() << endl << "Beta: " << p.portBeta << endl << "Sharpe Ratio: " << p.sharpeR << endl;
		out << "Risk-free: " << p.rf << endl << "Mean of return: " << p.meanR << endl << "Volatility: " << p.volatility << endl;
		return out;
	}
};
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
#include <set>
#include <ctime>

using namespace std::chrono;

using namespace std;

#define CROSSOVER_RATE            0.7
#define MUTATION_RATE             0.03
#define POP_SIZE                  50           //must be an even number
#define CHROMO_LENGTH             10
#define GENE_LENGTH               1
#define MAX_ALLOWABLE_GENERATIONS   100
#define BOUNDARY 10

//returns a float between 0 & 1
#define RANDOM_NUM      ((float)rand()/RAND_MAX)

const int NUM_OF_THREADS = 4;

bool asc_sort(Portfolio& struct1, Portfolio& struct2)
{
	return (struct1.getFitness() < struct2.getFitness());
}

int Roulette(double TotalFitness, vector<Portfolio>& portfolios)
{
	float Slice = RANDOM_NUM;

	//go through the chromosones adding up the fitness so far
	float FitnessSoFar = 0.0f;

	for (int i = 0; i < POP_SIZE; ++i) {
		FitnessSoFar += portfolios[i].getFitness() / TotalFitness;	// relative fitness

		//if the fitness so far > random number return the chromo at this point
		if (FitnessSoFar >= Slice)
			return i;
	}
}

int Crossover(vector<string>& offspring1, vector<string>& offspring2) {	
	int mark = -1;
	if (RANDOM_NUM < CROSSOVER_RATE) {
		std::random_shuffle(offspring1.begin(), offspring1.end());
		std::random_shuffle(offspring2.begin(), offspring2.end());
		//create a random crossover point
		int crossover = 5; // (rand() % 3) + 5;

		vector<string> v2(offspring2.begin() + crossover, offspring2.end());

		int pos = crossover;
		for (auto itr = v2.begin(); itr != v2.end(); ++itr) {
			if (std::find(offspring1.begin(), offspring1.begin() + pos, *itr) == offspring1.begin() + pos) {
				if (std::find(offspring2.begin(), offspring2.begin() + pos, offspring1[pos]) == offspring2.begin() + pos) {
					offspring2[pos] = offspring1[pos];
					offspring1[pos] = *itr;
					mark = 1;
				}
			}
			pos++;
		}
	}
	return mark;
}

void Mutate(vector<string> offspring, const vector<string>& symbols) {
	for (unsigned int i = 0; i < CHROMO_LENGTH; i++) {
		if (RANDOM_NUM < MUTATION_RATE) {
			int indx = rand() % symbols.size();
			if (std::find(offspring.begin(), offspring.end(), symbols[indx]) == offspring.end()) //不能重复
			{
				offspring[i] = symbols[indx];
			}
		}
	}
}

Portfolio Strategies(sqlite3* db, vector<Stock>& stocks, map<string, int>& mymap, const vector<string>& valSymbols) {
	srand((int)time(NULL));

	vector<Portfolio> portfolios;

	// ------------------------ 2. perpare all stocks ---------------------------------------------
	vector<double> betas;
	vector<double> sharpeRatios;
	vector<double> invPEs;
	vector<double> dividendYield;
	vector<double> vol;

	int size = valSymbols.size();

	for (int i = 0; i < size; ++i) {
		betas.push_back(stocks[i].getBasicData().getBeta());
		invPEs.push_back(1.0 / stocks[i].getBasicData().getPE());
		sharpeRatios.push_back(stocks[i].getSharpeR());
		dividendYield.push_back(stocks[i].getBasicData().getDividendYield());
		vol.push_back(stocks[i].getVolatility());
	}

	double minBeta = *min_element(betas.begin(), betas.end());
	double maxBeta = *max_element(betas.begin(), betas.end());
	double minSR = *min_element(sharpeRatios.begin(), sharpeRatios.end());
	double maxSR = *max_element(sharpeRatios.begin(), sharpeRatios.end());
	double minPE = *min_element(invPEs.begin(), invPEs.end());
	double maxPE = *max_element(invPEs.begin(), invPEs.end());
	double minDY = *min_element(dividendYield.begin(), dividendYield.end());
	double maxDY = *max_element(dividendYield.begin(), dividendYield.end());

	int mid = betas.size() / 2;
	double medianVol = size % 2 == 0 ? (vol[mid] + vol[mid - 1]) / 2 : vol[mid];

	// ------------------------ 3. 随机选择每个portfolio的股票 ---------------------------------------
	/*set<int> myset;
	for (int i = 0; i < POP_SIZE; i++) {
		cout << "id: " << i << endl;
		Portfolio p(i);
		int num;
		while (myset.size() < CHROMO_LENGTH) {
			num = int(RANDOM_NUM * size);
			myset.insert(num);
		}
		for (int it : myset) {
			cout << valSymbols[it] << ":" << it << " " << endl;
			p.addStocks(stocks[it]);
		}
		cout << endl;
		p.setAll(db);
		p.setFitness(minBeta, maxBeta, minSR, maxSR, minPE, maxPE, minDY, maxDY, medianVol);
		portfolios.push_back(p);

		myset.clear();
	}*/
	vector<Stock> copyStocks (stocks);
	std::random_shuffle(copyStocks.begin(), copyStocks.end());
	for (int i = 0; i < POP_SIZE; i++) {
		cout << "id: " << i << endl;
		Portfolio p(i);
		for (int i = 0; i < CHROMO_LENGTH; ++i) {
			cout << copyStocks[i].getSymbol() << " ";
			p.addStocks(copyStocks[i]);
		}
		p.setAll(db);
		p.setFitness(minBeta, maxBeta, minSR, maxSR, minPE, maxPE, minDY, maxDY, medianVol);
		cout << p.getFitness() << endl;
		portfolios.push_back(p);

		std::random_shuffle(copyStocks.begin(), copyStocks.end());
	}
	cout << "----------------------------------------------------------------------------------------" << endl;

	sort(betas.begin(), betas.end());
	sort(invPEs.begin(), invPEs.end());
	sort(sharpeRatios.begin(), sharpeRatios.end());
	sort(dividendYield.begin(), dividendYield.end());

	int pos = betas.size() * 0.96;
	double PctBeta = betas[pos];
	double PctSharpe = sharpeRatios[pos];
	double PctPE = invPEs[pos];
	double PctDY = dividendYield[pos];

	double benchmark = 0.37 * (PctBeta - minBeta) / (maxBeta - minBeta)
		+ 0.35 * (PctSharpe - minSR) / (maxSR - minSR)
		+ 0.03 * (PctPE - minPE) / (maxPE - minPE)
		+ 0.1 * (PctDY - minDY) / (maxDY - minDY)
		+ 0.15;

	cout << "benchmark: " << benchmark << endl;

	int GenerationsRequiredToFindASolution = 0;
	bool bFound = false;
	double fitness = 0.0;
	Portfolio save;
	int count = 0;
	while (!bFound) {
		// Roulette Wheel Selection:
		cout << " GenerationsRequiredToFindASolution: " << GenerationsRequiredToFindASolution << endl;
		double TotalFitness = 0;

		//test and update the fitness of every chromosome in the population
		for (int i = 0; i < POP_SIZE; i++) {
			if (portfolios[i].getFitness() >= benchmark) // 0.70 should be change to benchmark and we should choose not only one, but the best of best
			{
				cout << "\nSolution found in " << GenerationsRequiredToFindASolution << " generations!" << endl << endl;;
				cout << "Result: ";
				cout << portfolios[i] << endl;
				cout << portfolios[i].getFitness() << endl;

				if (count < 3) {
					if (fitness < portfolios[i].getFitness()) {
						fitness = portfolios[i].getFitness();
						save = portfolios[i];
					}
					count++;
				}
				else {
					cout << "\n Best Result: ";
					cout << save << endl;
					cout << save.getFitness() << endl;
					return save;
				}
					
			}
			TotalFitness += portfolios[i].getFitness();
		}

		vector<Portfolio> temp;
		int cPop = 0;

		//loop until we have created POP_SIZE new chromosomes
		while (cPop < BOUNDARY)
		{
			// create the new population by grabbing members of the old population
			// two at a time via roulette wheel selection.
			int a = Roulette(TotalFitness, portfolios);
			int b = Roulette(TotalFitness, portfolios);

			while (a == b) {
				b = Roulette(TotalFitness, portfolios);
			}

			vector<string> offspring1 = portfolios[a].getSymbols();
			vector<string> offspring2 = portfolios[b].getSymbols();

			if (Crossover(offspring1, offspring2) == 1) {
				Mutate(offspring1, valSymbols);
				Mutate(offspring2, valSymbols);
				Portfolio p1(-1);
				Portfolio p2(-1);
				int tempIndx1;
				int tempIndx2;
				for (int i = 0; i < CHROMO_LENGTH; i++) {
					tempIndx1 = mymap[offspring1[i]];
					p1.addStocks(stocks[tempIndx1]);
					tempIndx2 = mymap[offspring2[i]];
					p2.addStocks(stocks[tempIndx2]);
				}
				p1.setAll(db);
				p1.setFitness(minBeta, maxBeta, minSR, maxSR, minPE, maxPE, minDY, maxDY, medianVol);
				temp.push_back(p1);

				p2.setAll(db);
				p2.setFitness(minBeta, maxBeta, minSR, maxSR, minPE, maxPE, minDY, maxDY, medianVol);
				temp.push_back(p2);

				cPop += 2;
			}
		}

		// decide which one will be changed copy temp population into main population array
		std::sort(portfolios.begin(), portfolios.end(), asc_sort);
		cout << "current highest fitness: " << portfolios[49].getFitness() << endl;
		cout << "current lowest fitness: " << portfolios[0].getFitness() << endl;
		/*for (auto i : portfolios) {
			cout << i.getFitness() << " ";
		}
		cout << endl;*/
		for (int i = 0; i < temp.size(); ++i)
			portfolios[i] = temp[i];
		std::random_shuffle(portfolios.begin(), portfolios.end());

		++GenerationsRequiredToFindASolution;

		// exit app if no solution found within the maximum allowable number of generations
		if (GenerationsRequiredToFindASolution > MAX_ALLOWABLE_GENERATIONS)
		{
			if (fitness == 0) {
				cout << "No solutions found this run!";
				std::sort(portfolios.begin(), portfolios.end(), asc_sort);
				cout << portfolios[POP_SIZE - 1] << endl;
				cout << portfolios[POP_SIZE - 1].getFitness() << endl;
				return portfolios[POP_SIZE - 1];
			}
			cout << "Find one solution before 100th generation!" << endl;
			cout << save << endl;
			cout << save.getFitness() << endl;
			return save;
			
		}

	}
}

void BackTest(sqlite3* db, Portfolio& p) {
	vector<string> symbols = p.getSymbols();
	vector<float> weights = p.getWeights();
	string start_date = "2020-01-01", end_date = "2020-06-30";
	string sql = "SELECT `date` FROM `";
	string temp = "` WHERE `date` BETWEEN \'" + start_date + "\' AND \'" + end_date + "\' INTERSECT SELECT `date` FROM `";
	for (auto itr = symbols.begin(); itr != symbols.end() - 1; itr++) {
		sql += *itr + temp;
	}
	sql += symbols[9] + "` WHERE `date` BETWEEN \'" + start_date + "\' AND \'" + end_date + "\';";

	vector<string> dates;
	RetrieveOneColData(sql.c_str(), db, dates);

	string first_date = dates[0];
	string last_date = dates[dates.size() - 1];

	string select1 = "SELECT adjusted_close FROM `";
	string select2 = "` WHERE date IN (SELECT date From `";
	for (auto itr = symbols.begin(); itr != symbols.end() - 1; itr++) {
		select2 += *itr + "` INTERSECT SELECT date From `";
	}
	select2 += symbols[9] + "`) AND date BETWEEN \'" + first_date + "\' AND \'" + last_date + "\';";

	string sql_select;

	vector<vector<double>> allReturns;
	for (int i = 0; i < 10; i++) {
		sql_select = select1 + symbols[i] + select2;
		vector<double> temp;
		vector<double> r;
		RetrieveOneColData_double(sql_select.c_str(), db, temp);
		for (int j = 1; j < temp.size(); ++j) {
			r.push_back(temp[j] / temp[j - 1] - 1);
		}

		allReturns.push_back(r);
		std::for_each(allReturns[i].begin(), allReturns[i].end(), [&](double& el) {el *= weights[i]; });
	}

	int size = dates.size() - 1;	// all stock have the same return size
	vector<double> portReturns;
	// Calculate portfolio daily returns from 10 stocks
	for (int i = 0; i < size; i++) {
		double r = 0;
		for (int j = 0; j < 10; j++) {
			r += allReturns[j][i];
		}
		portReturns.push_back(r);
	}

	vector<double> SPYReturns;
	vector<double> SPY;
	sql = "SELECT `adjusted_close` FROM SPY WHERE `date` BETWEEN \'" + first_date + "\' AND \'" + last_date + "\';";;
	RetrieveOneColData_double(sql.c_str(), db, SPY);

	for (int i = 1; i < SPY.size(); i++) {
		SPYReturns.push_back(SPY[i] / SPY[i - 1] - 1);
	}

	fstream fout;
	fout.open("BackTest.csv", ios::out);
	fout << "SPY_Returns" << "," << "Portfolio_Returns" << "," << "CUM_SPY_Returns" << "," << "CUM_Portfolio_Returns" << endl;
	double cum_returns_SPY = 1, cum_returns_portfolio = 1;

	for (int i = 0; i < SPYReturns.size(); ++i) {
		cum_returns_SPY *= (1 + SPYReturns[i]);
		cum_returns_portfolio *= (1 + portReturns[i]);
		fout << SPYReturns[i] << "," << portReturns[i] << "," << cum_returns_SPY-1 << "," << cum_returns_portfolio-1 << endl;
	}

	cout << endl << "BackTesting Finished" << endl;
}

void ProbationTest(sqlite3* db, Portfolio& p) {
	vector<string> symbols = p.getSymbols();
	vector<float> weights = p.getWeights();
	string start_date = "2020-07-01", end_date = "2020-7-31";
	string sql = "SELECT `date` FROM `";
	string temp = "` WHERE `date` BETWEEN \'" + start_date + "\' AND \'" + end_date + "\' INTERSECT SELECT `date` FROM `";
	for (auto itr = symbols.begin(); itr != symbols.end() - 1; itr++) {
		sql += *itr + temp;
	}
	sql += symbols[9] + "` WHERE `date` BETWEEN \'" + start_date + "\' AND \'" + end_date + "\';";

	vector<string> dates;
	RetrieveOneColData(sql.c_str(), db, dates);

	string first_date = dates[0];
	string last_date = dates[dates.size() - 1];

	string select1 = "SELECT adjusted_close FROM `";
	string select2 = "` WHERE date IN (SELECT date From `";
	for (auto itr = symbols.begin(); itr != symbols.end() - 1; itr++) {
		select2 += *itr + "` INTERSECT SELECT date From `";
	}
	select2 += symbols[9] + "`) AND date BETWEEN \'" + first_date + "\' AND \'" + last_date + "\';";

	string sql_select;

	vector<vector<double>> allReturns;
	for (int i = 0; i < 10; i++) {
		sql_select = select1 + symbols[i] + select2;
		vector<double> temp;
		vector<double> r;
		RetrieveOneColData_double(sql_select.c_str(), db, temp);
		for (int j = 1; j < temp.size(); ++j) {
			r.push_back(temp[j] / temp[j - 1] - 1);
		}

		allReturns.push_back(r);
		std::for_each(allReturns[i].begin(), allReturns[i].end(), [&](double& el) {el *= weights[i]; });
	}

	int size = dates.size() - 1;	// all stock have the same return size
	vector<double> portReturns;
	// Calculate portfolio daily returns from 10 stocks
	for (int i = 0; i < size; i++) {
		double r = 0;
		for (int j = 0; j < 10; j++) {
			r += allReturns[j][i];
		}
		portReturns.push_back(r);
	}

	vector<double> SPYReturns;
	vector<double> SPY;
	sql = "SELECT `adjusted_close` FROM SPY WHERE `date` BETWEEN \'" + first_date + "\' AND \'" + last_date + "\';";;
	RetrieveOneColData_double(sql.c_str(), db, SPY);

	for (int i = 1; i < SPY.size(); i++) {
		SPYReturns.push_back(SPY[i] / SPY[i - 1] - 1);
	}

	fstream fout;
	fout.open("probationTest.csv", ios::out);
	fout << "SPY_Returns" << "," << "Portfolio_Returns" << "," << "CUM_SPY_Returns" << "," << "CUM_Portfolio_Returns" << endl;
	double cum_returns_SPY = 1, cum_returns_portfolio = 1;

	for (int i = 0; i < SPYReturns.size(); ++i) {
		cum_returns_SPY *= (1 + SPYReturns[i]);
		cum_returns_portfolio *= (1 + portReturns[i]);
		fout << SPYReturns[i] << "," << portReturns[i] << "," << cum_returns_SPY-1 << "," << cum_returns_portfolio-1 << endl;
	}

	// --------------------------------------------------------------
	sql_select = "SELECT adjusted_close FROM US10Y WHERE date = '" + start_date + "';";
	p.setRF(db, sql_select);
	double rf = p.getRF();
	double Beta = p.getBeta();
	double PE = p.getPE();
	double meanR = 0;
	double volatility = 0;
	for (auto i : portReturns) {
		meanR += i;
	}
	meanR = meanR / static_cast<double>(portReturns.size());
	for (auto i : portReturns) {
		volatility += (i - meanR) * (i - meanR);
	}
	volatility = sqrt(volatility / portReturns.size());
	double sharpeR = (meanR - rf) / volatility;

	cout << "Portfolio: " << endl;
	cout << "PE: " << PE << endl << "Dividend Yield: " << p.getDY() << endl << "Beta: " << Beta << endl << "Sharpe Ratio: " << sharpeR << endl;
	cout << "Risk-free: " << rf << endl << "Mean of return: " << meanR << endl << "Volatility: " << volatility << endl;

	// -------------------------------------------------------
	Beta = 1;
	meanR = 0;
	volatility = 0;
	for (auto i : SPYReturns) {
		meanR += i;
	}
	meanR = meanR / static_cast<double>(SPYReturns.size());
	for (auto i : SPYReturns) {
		volatility += (i - meanR) * (i - meanR);
	}
	volatility = sqrt(volatility / SPYReturns.size());
	sharpeR = (meanR - rf) / volatility;

	cout << "SPY: " << endl;
	cout << "Beta: " << Beta << endl << "Sharpe Ratio: " << sharpeR << endl;
	cout << "Risk-free: " << rf << endl << "Mean of return: " << meanR << endl << "Volatility: " << volatility << endl;

	cout << endl << "Probation Testing Finished" << endl;
}
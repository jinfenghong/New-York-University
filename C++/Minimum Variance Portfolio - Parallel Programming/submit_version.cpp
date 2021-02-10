#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>
//#include <curl/curl.h>
using namespace std;
typedef long myint;

// this part is to download the file, get all companies name and choose companies and store data
/*void printToFile(std::string filename, const std::string &text) {
	std::ofstream mfile;
	mfile.open(filename);
	for (myint i = 0; i<text.length(); ++i) {
		mfile << text[i];
	}
	mfile.close();
}

static size_t writerF(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	((std::string*)userdata)->append((char*)ptr, size * nmemb);
	return size * nmemb;
}
void stockDataToFile(const std::string &tickerName,
	const std::string &quandl_auth_token,
	const std::string &database = "WIKI",
	const std::string &folder = "./TXTquotes/") {
	std::string mainLink = "https://www.quandl.com/api/v3/datasets/";
	mainLink += database;
	mainLink += "/" + tickerName;
	mainLink += ".csv";
	mainLink += "?sort_order=asc&auth_token=";
	mainLink += quandl_auth_token;
	CURL *curl;
	std::string quandlData;
	std::string fName = folder;
	fName += tickerName;
	fName += ".txt"; //save the data into certain type of file, such as '.txt', '.ods', and '.csv'
	curl = curl_easy_init();
	if (curl) {
		const char* linkArrayChar = mainLink.c_str();
		curl_easy_setopt(curl, CURLOPT_URL, linkArrayChar);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writerF);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &quandlData);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		printToFile(fName, quandlData);
	}
}*/

void getTickersName(vector<string>& allTickers) {
	ifstream readFile("April 2018 S&P 500 Stocks.txt");
	string names;
	while (readFile) {
		readFile >> names;
		allTickers.push_back(names);
	}
}

int checkSize(const string& company, const myint& years) {
	myint counter = 0;
	string fname = "TXTquotes/" + company + ".txt";
	ifstream infile(fname);
	char ch;
	string s;
	int range = 250 * years;
	getline(infile, s);	//get rid of the first line
	while (!infile.eof()) {
		infile.get(ch);
		if (ch == '\n') {
			++counter;
			if (counter >= range + 1) {
				return 1;
			}
		}
	}
	return 0;
}

void getCompany(const vector<string>& allTickers, vector <string>& tickers, const myint& len, const myint& years) {
	vector<myint> num;
	const myint min = 0;
	const myint max = allTickers.size() - 1;// -2 or -1;	//has to check the size first!
	myint y;
	int i;
	for (i = 0; i < len; ++i) {
		srand(time(NULL));
		y = (rand() % (max - min + 1)) + min;
		if ((find(num.begin(), num.end(), y) != num.end()) || checkSize(allTickers[y], years) == 0) {
			--i;
		}

		else { num.push_back(y); }
	}

	sort(num.begin(), num.end());
	tickers.resize(len);

	for (i = 0; i < len; ++i) {
		tickers[i] = allTickers[num[i]];
	}
	cout << "the companies you choose are: ";
	for (i = 0; i < len; ++i) {
		cout << tickers[i] << " ";
	}
	cout << endl;
}

void readData(const vector<string>& allTickers, vector<vector<double>>& data,
	vector<string>& tickers, const myint& len, const myint& years,
	const myint& interval) {

	getCompany(allTickers, tickers, len, years);

	myint cSize = tickers.size();
	vector <double> record;
	string s, row;
	myint i, j, k, l;

	for (i = 0; i < cSize; ++i) {
		string fname = "TXTquotes/" + tickers[i] + ".txt";
		ifstream infile(fname);
		//if (infile.is_open())
		getline(infile, row);	//get rid of the first category row

		for (j = 0; j < years; ++j) {
			for (l = 0; l < 251; l += interval) {
				getline(infile, row);
				istringstream ss(row);
				for (k = 0; k < 12; ++k) {
					getline(ss, s, ',');
				}
				//------------convert a string to double-----------------
				stringstream convertor(s);
				double n;
				convertor >> n;
				record.push_back(n);
				//-------------------------------------------------------
				if (j < years) {
					for (k = 0; k < interval - 1; ++k) {
						getline(infile, row);
					}
				}
			}
		}
		data.push_back(record);
		record.clear();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*void defineMatrix(const myint& dimM, const myint& dimN, vector<vector<double>>& matrix) {
	matrix.resize(dimM);	// define the number of rows;
	for (myint i = 0; i < dimM; ++i) {
		matrix[i].resize(dimN);
	}
}*/
void defineMatrix(const myint& dimM, const myint& dimN, vector<vector<double>>& matrix) {
	int nths;
	matrix.resize(dimM);	// define the number of rows;
#pragma omp parallel
	{
		if (omp_get_thread_num() == 0) {
			nths = omp_get_num_threads();
		}
	}
#pragma omp parallel
	{
		int id = omp_get_thread_num();

		for (myint i = id; i < dimM; i += nths) {
			matrix[i].resize(dimN);
		}
	}
#pragma omp barrier
}

void mMultiplication(const vector<vector<double>>& left, const vector<vector<double>>right, vector<vector<double>>& matrixProduct) {
	/*be very careful here, the production is mat1*mat2, so the
	dimension of matrixProduct is the row of the mat1 with col of mat2 */
	myint row1 = left.size();
	myint col2 = right[0].size();
	myint row2 = right.size();

	defineMatrix(row1, col2, matrixProduct);
	int nths;
#pragma omp parallel
	{
		if (omp_get_thread_num() == 0) {
			nths = omp_get_num_threads();
		}
	}
#pragma omp parallel
	{
		int id = omp_get_thread_num();
		for (myint i = id; i < row1; i += nths) {
			for (myint j = 0; j < col2; ++j) {
				double sum = 0;
				for (myint k = 0; k < row2; ++k) {
					sum += left[i][k] * right[k][j];
				}
				matrixProduct[i][j] = sum;
			}
		}
	}
#pragma omp barrier
}

void mTranspose(const vector<vector<double>>& original, vector<vector<double>>& transposed) {
	myint dimM = original[0].size();
	myint dimN = original.size();
	defineMatrix(dimM, dimN, transposed);

	int nths;
#pragma omp parallel
	{
		if (omp_get_thread_num() == 0) {
			nths = omp_get_num_threads();
		}
	}
#pragma omp parallel
	{
		myint i, j;
		int id = omp_get_thread_num();
		for (myint i = id; i < dimM; i += nths) {
			for (myint j = 0; j < dimN; ++j) {
				transposed[i][j] = original[j][i];
			}
		}
	}
#pragma omp barrier
}

void mAddition(const vector<vector<double>>& m1, const vector<vector<double>>& m2, vector<vector<double>>& m3_add) {
	myint rows = m1.size();
	myint cols = m1[0].size();
	defineMatrix(rows, cols, m3_add);

	int nths;
#pragma omp parallel
	{
		if (omp_get_thread_num() == 0) {
			nths = omp_get_num_threads();
		}
	}
#pragma omp parallel
	{
		myint i, j;
		int id = omp_get_thread_num();
		for (i = id; i < rows; i += nths) {
			for (j = 0; j < cols; ++j) {
				m3_add[i][j] = m1[i][j] + m2[i][j];
			}
		}

	}
#pragma omp barrier
}

void LU(const vector<vector<double>>& myVec, vector<vector<double>>& l, vector<vector<double>> &u) {
	myint dim = myVec.size();
	defineMatrix(dim, dim, l);
	defineMatrix(dim, dim, u);

	/*---------- special attributs of L --------------*/
	for (myint i = 0; i < dim; ++i) {
		l[i][i] = 1;
	}

	double dot_U = 0, dot_L = 0;
	myint i, j, k;
	for (i = 0; i < dim; ++i) {
		/* to get the row for U*/
		for (j = i; j < dim; ++j) {
			for (k = 0; k < i; ++k) {
				dot_U += l[i][k] * u[k][j];	//keep L's ith row unchanged, change the column of U
			}
			u[i][j] = myVec[i][j] - dot_U;
			dot_U = 0;
		}
		/* after getting each row for U, we should also to get each column for L in order to get next row of U */
		for (j = i + 1; j < dim; ++j) {
			for (k = 0; k < i; ++k) {
				dot_L += l[j][k] * u[k][i];	//keep U's ith column unchanged, change the row of L
			}
			l[j][i] = (myVec[j][i] - dot_L) / u[i][i];
			dot_L = 0;
		}
	}
}

void mInverse(vector<vector<double>>& l, vector<vector<double>> &u, vector<vector<double>>& LU_Inv) {
	myint dim = l.size();
	long i = 0, j = 0, k = 0;
	double dot_U = 0, dot_L = 0;

	vector<vector<double>> l_Inv;	//inverse of L
	vector<vector<double>> u_Inv;	//inverse of U
	vector<vector<double>> I;		//Identity matrix I

	defineMatrix(dim, dim, l_Inv);
	defineMatrix(dim, dim, u_Inv);
	defineMatrix(dim, dim, I);
	for (myint i = 0; i < dim; ++i) {
		I[i][i] = 1;
	}

	/*first find the inverse of L*/
	for (j = 0; j < dim; ++j) {		//j is the index of column of the L's inverse
		for (i = 0; i < dim; ++i) {	//i is to change the row of L
			for (k = 0; k < i; ++k) {
				dot_L += l[i][k] * l_Inv[k][j];
			}
			l_Inv[i][j] = (I[i][j] - dot_L) / l[i][i];
			dot_L = 0;
		}
	}

	/*second find the inverse of U, the difference is that we get the inverse of U backward*/
	for (j = dim - 1; j > -1; --j) {//j is the index of column of the L's inverse
		for (i = dim - 1; i > -1; --i) {//i is to change the row of L
			for (k = dim - 1; k > i; --k) {
				dot_U += u[i][k] * u_Inv[k][j];
			}
			u_Inv[i][j] = (I[i][j] - dot_U) / u[i][i];
			dot_U = 0;
		}
	}
	/*third, calculate the inverse*/
	mMultiplication(u_Inv, l_Inv, LU_Inv);
}

void ReturnAndAverage(vector<vector<double>>& data, vector<vector<double>>& unitReturn, vector<vector<double>>& avgR) {
	myint rows = data.size();
	myint cols = data[0].size();
	defineMatrix(1, rows, avgR);
	defineMatrix(rows, cols - 1, unitReturn);
	int nths;
#pragma omp parallel
	{
		if (omp_get_thread_num() == 0) { nths = omp_get_num_threads(); }
	}
#pragma omp parallel num_threads(nths)
	{
		int id = omp_get_thread_num();
		vector<double> avgEach;
		long double sumForAvg = 0, r;
		for (myint i = id; i < rows; i += nths) {
			for (myint j = 0; j < cols - 1; ++j) {
				r = (data[i][j] / data[i][j + 1]) - 1;
				sumForAvg += r;
				unitReturn[i][j] = r;
			}
			avgR[0][i] = sumForAvg / (cols - 1);	//get the average return and push it to the end of the vector
			sumForAvg = 0;
		}
	}
#pragma omp barrier
	cout << "the average return of each company: ";
	for (int i = 0; i < rows; ++i) {
		cout << avgR[0][i] << " ";
	}
	cout << endl;
}

void Covariance_matrix(vector <vector <double> >& average_rate, vector< vector< double>>& unitReturn, vector< vector<double> >& C)
{
	myint rows, columns;
	rows = unitReturn.size();
	columns = unitReturn[0].size();

	vector< vector<double>> C_part1;
	defineMatrix(rows, rows, C_part1);

	int nths;
#pragma omp parallel
	{
		if (omp_get_thread_num() == 0) { nths = omp_get_num_threads(); }
	}
#pragma omp parallel num_threads(nths)
	{
		double sum, cov_i_j;
		int id = omp_get_thread_num();
		for (myint i = id; i< rows; i += nths)
		{
			for (myint j = i; j<rows; ++j)
			{
				sum = 0;
				for (myint k = 0; k< columns; ++k)
				{
					sum += (unitReturn[i][k] - average_rate[0][i])*(unitReturn[j][k] - average_rate[0][j]);
				}
				cov_i_j = sum / (columns - 1);
				C_part1[i][j] = cov_i_j;
			}
		}
	}
#pragma omp barrier

	vector< vector<double> > C_part1_T;//This will be the second part of C
									   // remember tha C is symmetric. 
	mTranspose(C_part1, C_part1_T);
	mAddition(C_part1, C_part1_T, C);

	//you need to divide the diagonal by 2 since by the algorithm above 
	// the variance will be add it twice
	for (myint i = 0; i< C.size(); ++i)
	{
		C[i][i] /= 2.0;
	}
	/*cout << "Covariance: " << endl;
	for (int i = 0; i < C.size(); ++i) {
		for (int j = 0; j < C[0].size(); ++j) {
			cout << C[i][j] << " ";
		}
		cout << endl;
	}*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MinimalVariance_ER(const vector<vector<double>>& m,
	const vector<vector<double>>& C,
	vector<vector<double>>& w,
	const double& ER) {

	myint dim = C.size();
	myint col = m[0].size();
	/*to set u*/
	vector<vector<double>> u;
	defineMatrix(1, col, u);
	for (int i = 0; i < col; ++i) {
		u[0][i] = 1;
	}

	/*to get the transpose of m and u matrix*/
	vector<vector<double>> m_T, u_T;
	mTranspose(m, m_T);
	mTranspose(u, u_T);

	/*to get the inverse matrix of C*/
	vector<vector<double>> C_Inv;
	vector<vector<double>> L1;
	vector<vector<double>> U1;
	LU(C, L1, U1);	// to get L and U first before getting the inverse of C_Inv
	mInverse(L1, U1, C_Inv);

	vector<vector<double>> X1, X2;	//X1 and X2 are the matrix that at the rightmost side in 2xn matrix
	defineMatrix(1, dim, X1);
	defineMatrix(1, dim, X2);

	/*to get X1, and X2*/
	mMultiplication(m, C_Inv, X1);
	mMultiplication(u, C_Inv, X2);

	/*to get M*/
	vector<vector<double>> M;
	defineMatrix(2, 2, M);
	vector<vector<double>> temp;
	mMultiplication(X1, m_T, temp);
	M[0][0] = temp[0][0];
	mMultiplication(X1, u_T, temp);
	M[0][1] = temp[0][0];
	mMultiplication(X2, m_T, temp);
	M[1][0] = temp[0][0];
	mMultiplication(X2, u_T, temp);
	M[1][1] = temp[0][0];

	/*to get the inverse of M that is M_T*/
	vector<vector<double>> M_Inv;
	vector<vector<double>> L2;
	vector<vector<double>> U2;
	LU(M, L2, U2);	// to get L and U first before getting the inverse of C_Inv
	mInverse(L2, U2, M_Inv);

	/*to get rightmost part (Y) and the combination of rightmost and middle part (Z)*/
	vector<vector<double>> Y, Z;
	defineMatrix(2, dim, Y);
	defineMatrix(2, dim, Z);
	/*to get Y*/
	for (int i = 0; i < 2; ++i) {
		if (i == 0) {
			for (int j = 0; j < dim; ++j) {
				Y[i][j] = X1[0][j];
			}
		}
		else {
			for (int j = 0; j < dim; ++j) {
				Y[i][j] = X2[0][j];
			}
		}
	}
	/*to get Z*/
	mMultiplication(M_Inv, Y, Z);

	/*to get u_1*/
	vector<vector<double>> u_1;
	defineMatrix(1, 2, u_1);
	u_1[0][0] = ER;
	u_1[0][1] = 1;

	/*to get w*/
	mMultiplication(u_1, Z, w);

}

void MVP_1(const vector< vector< double> >& C, vector< vector<double> >& minimum_weights)
{
	myint rows_C, columns_C;  // C should be square 
	rows_C = C.size();
	columns_C = C[0].size();

	vector< vector< double> > C_inverse, u, u_T;

	// Get the inverse of the covariance matrix 
	vector< vector<double> > C_lower, C_upper;
	LU(C, C_lower, C_upper);
	mInverse(C_lower, C_upper, C_inverse);

	//set up the array u 
	defineMatrix(1, columns_C, u);
	for (myint j = 0; j< columns_C; ++j)
	{
		u[0][j] = 1;
	}
	// defineMatrix(columns,1,u_T) ; 
	mTranspose(u, u_T);

	//Now get u.C^-1
	vector< vector< double> > u_mul_C_inverse;
	mMultiplication(u, C_inverse, u_mul_C_inverse);

	//Now get u.C^-1.u_T  
	vector< vector< double> > u_mul_C_inverse_mul_u_T;
	mMultiplication(u_mul_C_inverse, u_T, u_mul_C_inverse_mul_u_T);// should be a number       

	double temp = u_mul_C_inverse_mul_u_T[0][0];

	// Now get the minimum weights 
	defineMatrix(1, columns_C, minimum_weights);
	for (myint j = 0; j <columns_C; ++j)
	{
		minimum_weights[0][j] = u_mul_C_inverse[0][j] / temp;
	}

}

//Formula for getting the final return and print it to the screen
void final_return(double Capital, const vector< vector<double> >& weights,
	vector< vector< double> > average_return, vector< vector<double>>& finalreturn)
{
	//Know how many companies are we dealing with
	myint weights_rows, weights_columns;
	weights_rows = weights.size();
	weights_columns = weights[0].size();

	//get how much capital will be given for each company
	// some of them will be negative (short selling) 

	vector< vector<double>> Capital_times_weights;
	defineMatrix(1, weights_columns, finalreturn);
	defineMatrix(1, weights_columns, Capital_times_weights);

	for (myint j = 0; j< weights_columns; ++j)
	{
		Capital_times_weights[0][j] = Capital*weights[0][j];
		finalreturn[0][j] = average_return[0][j] * Capital_times_weights[0][j];
	}
	double sum_finalreturn = 0;

	for (myint j = 0; j< weights_columns; ++j)
	{
		sum_finalreturn += finalreturn[0][j];
	}

	//Now the printing stage
	/*cout << "The final return for each companies\n";
	for (myint j = 0; j<weights_columns; ++j)
	{
	cout << finalreturn[0][j] << "\t";
	}
	cout << "\n" << "Final return of portfolio: " << sum_finalreturn << endl;*/

}
////////////////////////////////////////////////////////////////////////////////
void Short_Long(const vector<vector<double>>& weights,
	const vector<vector<double>>& avgR,
	vector< vector<double>>& finalreturn,
	const double& Capital) {

	double Psum = 0.0, Nsum = 0.0;
	myint cols = weights[0].size();
	for (int i = 0; i < cols; ++i) {
		if (weights[0][i] > 0) { Psum += weights[0][i]; }
		else { Nsum += (-1.0*weights[0][i]); }
	}

	Nsum *= Capital;

	myint weights_rows, weights_columns;
	weights_rows = weights.size();
	weights_columns = weights[0].size();

	//get how much capital will be given for each company
	// some of them will be negative (short selling) 

	defineMatrix(1, weights_columns, finalreturn);

	for (myint i = 0; i< weights_columns; ++i) {
		if (weights[0][i] > 0) {
			finalreturn[0][i] = avgR[0][i] * (Capital + Nsum) * weights[0][i];
		}
		else { finalreturn[0][i]; }
	}

	double sum_finalreturn = 0;
	for (myint j = 0; j< weights_columns; ++j)
	{
		sum_finalreturn += finalreturn[0][j];
	}
}

void printPortfolio(const vector<string>& tickers, const vector<vector<double>>& avgR,
	const vector<vector<double>>& minimum_weights,
	const vector<vector<double>>& finalreturn,
	const myint& len, ofstream& outFile) {
	double sum = 0.0;
	for (int i = 0; i < len; ++i) {
		outFile << tickers[i] << "," << avgR[0][i] << "," << minimum_weights[0][i] << "," << finalreturn[0][i] << endl;
		sum += finalreturn[0][i];
	}

	outFile << ",,,," << sum << endl << endl;
}

///////////////////////////////////////////////////////////////////////////////

int main() {
	vector <string> allTickers = { "AAPL","ADSK" };	//if you use these two companies to test the function, you have to change the range of generating random variables in getCompanies function to -1 instead of -2
	//getTickersName(allTickers);
	//cout << "the size of allTickers: " << allTickers.size() << endl;

	//this is the part to download the file
	/*std::string myQuandlToken = "iAepFPtPy45DYv9CEVQW"; //this is API-KEY
	for (vector<string>::iterator itr = allTickers.begin(); itr != allTickers.end(); ++itr) {
	std::string stockName = *itr; // This is the ticker for Disney, in case you did not know
	stockDataToFile(stockName, myQuandlToken);
	}*/

	myint num_company, years;
	cout << "How  many company you want: ";
	cin >> num_company;
	cout << "How many years'data you want: ";
	cin >> years;
	myint interval; //years, days, months, weeks;
	myint uI;
	cout << "Here is the type of return: ";
	cout << "1. annually return; 2. monthly return; 3. weekly return; 4. daily return" << endl;
	cout << "Your choice: ";
	cin >> uI;
	while (uI != 1 && uI != 2 && uI != 3 && uI != 4) {
		cout << "please enter 1-4: ";
		cin >> uI;
	}
	switch (uI)
	{
	case 1: interval = 250;
		break;
	case 2: interval = 22;
		break;
	case 3: interval = 5;
		break;
	case 4: interval = 1;
		break;
	}

	double capital;
	cout << "your capital is: $";
	cin >> capital;

	myint num_portfolio;
	cout << "How many portfolio you want: ";
	cin >> num_portfolio;

	cout << "would you want to input an expected return (y/n): ";
	char ch;
	double ER;
	cin >> ch;
	if (ch == 'y') {
		cout << "Desired Expected Return: ";
		cin >> ER;
	}

	ofstream outFile("result.csv");
	outFile << "ticker name, annually average return, weight, expected final return" << endl;

	/*ofstream outFile2("result_SL.csv");
	outFile2 << "ticker name, annually average return, weight, expected final return" << endl;*/

	for (int i = 0; i < num_portfolio; ++i) {
		vector<vector<double>> cPrice;
		vector <string> tickers;
		readData(allTickers, cPrice, tickers, num_company, years, interval);

		vector<vector<double>> unitReturn;	// each company's return
		vector<vector<double>> avgR;		// average return

											//cout << endl << "this is (annually, monthly, weekly, or daily) return" << endl;
		ReturnAndAverage(cPrice, unitReturn, avgR);

		vector<vector<double>> C;	// matrix of covariance
		Covariance_matrix(avgR, unitReturn, C);

		///////////////////////////////////////////////////////////////////////////
		if (ch == 'y') {
			vector<vector<double>> w;	//contain the weight for each company
			MinimalVariance_ER(avgR, C, w, ER);
			vector<vector<double>> finalreturn;
			final_return(capital, w, avgR, finalreturn);
			printPortfolio(tickers, avgR, w, finalreturn, num_company, outFile);
		}
		else {
			vector<vector<double>> w2;	//contain the weight for each company 
			MVP_1(C, w2);

			vector<vector<double>> finalreturn;
			final_return(capital, w2, avgR, finalreturn);
			printPortfolio(tickers, avgR, w2, finalreturn, num_company, outFile);

			/*vector<vector<double>> finalreturn2;
			Short_Long(w2, avgR, finalreturn2, capital);
			printPortfolio(tickers, avgR, w2, finalreturn2, num_company, outFile2);*/
		}
	}
	return 0;
}

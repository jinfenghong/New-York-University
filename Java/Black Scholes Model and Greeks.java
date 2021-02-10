package javalab;

import java.util.Scanner;

public class HW3 {
	public static void main(String[] args) {
		Scanner sc = new Scanner(System.in); 
		
		System.out.println("start Q1");
		Q1(sc); 
		System.out.println("End Q1");
		System.out.println("---------------------------------------------");
		System.out.println("start Q2");
		Q2(sc); 
		System.out.println("End Q2");
		System.out.println("---------------------------------------------");
		
		System.out.println("done");
		sc.close(); 
	}
	
	public static void Q1(Scanner sc) {
		System.out.print("please enter K: ");
		double K = sc.nextDouble();
		System.out.print("please enter T: ");
		double T = sc.nextDouble();
		BSMOption ec = new BSMOption(K,T,true);
		BSMOption ep = new BSMOption(K,T,false);
		
		System.out.print("please enter S: ");
		double S = sc.nextDouble();
		System.out.print("please enter r: ");
		double r = sc.nextDouble() / 100;
		System.out.print("please enter q: ");
		double q = sc.nextDouble() / 100;
		System.out.print("please enter sigma: ");
		double sigma = sc.nextDouble();
		System.out.print("please enter start time t0: ");
		double t0 = sc.nextDouble();
		
		double call = ec.FairValue(S, r, q, sigma, t0);
		double put = ep.FairValue(S, r, q, sigma, t0);
		
		double deltaC = ec.Delta(S, r, q, sigma, t0);
		double deltaP = ep.Delta(S, r, q, sigma, t0);
		
		double gammaC = ec.Gamma(S, r, q, sigma, t0);
		double gammaP = ep.Gamma(S, r, q, sigma, t0);
		
		double vegaC = ec.Vega(S, r, q, sigma, t0);
		double vegaP = ep.Vega(S, r, q, sigma, t0);
		
		double thetaC = ec.Theta(S, r, q, sigma, t0);
		double thetaP = ep.Theta(S, r, q, sigma, t0);
		
		double rhoC = ec.Rho(S, r, q, sigma, t0);
		double rhoP = ep.Rho(S, r, q, sigma, t0);
		
		if(call!= -1) System.out.printf("call: %.4f\t", call);
		else System.out.println("Invalid Inputs! No call price here!");
		if(put!= -1) System.out.printf("put: %.4f\n", put);
		else System.out.println("Invalid Inputs! No put price here!");
		
		if(deltaC != 0) System.out.printf("delta of call: %.4f\t", deltaC);
		else System.out.println("Invalid Inputs! No call delta here!");
		if(deltaP != 0) System.out.printf("delta of put: %.4f\n", deltaP);
		else System.out.println("Invalid Inputs! No call delta here!");
		
		if(gammaC != 0) System.out.printf("gamma of call: %.4f\t", gammaC);
		else System.out.println("Invalid Inputs! No call gamma here!");
		if(gammaP != 0) System.out.printf("gamma of put: %.4f\n", gammaP);
		else System.out.println("Invalid Inputs! No put gamma here!");
		
		if(vegaC != 0) System.out.printf("vega of call: %.4f\t", vegaC);
		else System.out.println("Invalid Inputs! No call vega here!");
		if(vegaP != 0) System.out.printf("vega of put: %.4f\n", vegaP);
		else System.out.println("Invalid Inputs! No put vega here!");
		
		if(thetaC != 0) System.out.printf("theta of call: %.4f\t", thetaC);
		else System.out.println("Invalid Inputs! No call theta here!");
		if(thetaP != 0) System.out.printf("theta of put: %.4f\n", thetaP);
		else System.out.println("Invalid Inputs! No put theta here!");
		
		if(rhoC != 0) System.out.printf("rho of call: %.4f\t", rhoC);
		else System.out.println("Invalid Inputs! No call rho here!");
		if(rhoP != 0) System.out.printf("rho of put: %.4f\n", rhoP);
		else System.out.println("Invalid Inputs! No put rho here!");
		
		double f_tol = 0.001, x_tol = 0.0001;
		int max_iter = 100;
		
		System.out.print("please enter a target price: ");
		double target = sc.nextDouble();
		
		System.out.print("please enter a initial sigma: ");
		double sigma0 = sc.nextDouble();
		
		double ivol_c = ec.Impvol(S, r, q, sigma0, t0, target, f_tol, x_tol, max_iter);
		double ivol_p = ep.Impvol(S, r, q, sigma0, t0, target, f_tol, x_tol, max_iter);
		
		if (ivol_c != -1) System.out.printf("implicit volatility of call: %.2f%%\n",ivol_c*100);
		else System.out.println("Cannot get implicit volatility for Call, it is a bad volatility!\n");
		if (ivol_p != -1) System.out.printf("implicit volatility of put: %.2f%%\n", ivol_p*100);
		else System.out.println("Cannot get implicit volatility for Put, it is a bad volatility!\n");
		
	}
	
	public static void Q2(Scanner sc) {
		System.out.print("please enter a Future value, F: ");
		double F = sc.nextDouble();
		System.out.print("please enter a regular payment, c: ");
		double c = sc.nextDouble();
		System.out.print("please enter time periodm, n: ");
		int n = sc.nextInt();
		
		Bond bond = new Bond(F,c,n);
		
		double[] y = new double[]{0,0.01,0.02,0.03,0.04,0.05,0.06,0.07,0.08,0.09,0.1};
		
		for(int i = 0; i < y.length; i++){
			double fv = bond.FairValue(y[i]);
			System.out.printf("fair value with yield %f: %.2f\n", y[i], fv);
		}
		
		System.out.print("please enter a target price: ");
		double target = sc.nextDouble();
		
		double f_tol = 0.001, x_tol = 0.0001;
		int max_iter = 100;
		double yield = bond.Yield(target, f_tol, x_tol, max_iter);
		
		if (yield != -1){
			System.out.printf("With target price, the yield should be: %.2f%%\n", yield*100);
		}
		else System.out.println("Cannot get yield given target price, it is a bad yield!");
	}

}

class BSMOption{
	private double K;
	private double T;
	private boolean isCall;
	
	public BSMOption(double K_, double T_, boolean isCall_) {
		K = K_;
		T = T_;
		isCall = isCall_;
	}
	
	// "get" Method
	double getK(){
		return K;	
	}
	
	double getT(){
		return T;
	}
	
	boolean getIsCall(){
		return isCall;
	}
	
	public double FairValue(double S, double r, double q, double sigma, double t0) {
		if (S<=0 || K<=0 || sigma<=0 || T-t0<=0){
			return -1;
		}
		
		double sqrt_T = Math.sqrt(T-t0);
		double d1 = (Math.log(S/K)+(r-q)*(T-t0))/(sigma*sqrt_T)+sigma*sqrt_T/2;
		double d2 = d1 - sigma*sqrt_T;
		
		double fair;
		if (isCall){
			fair = S*Math.exp(-q*(T-t0))*Funcs.NormalCDF(d1)-K*Math.exp(-r*(T-t0))*Funcs.NormalCDF(d2);
		}
		else{
			fair = -S*Math.exp(-q*(T-t0))*Funcs.NormalCDF(-d1)+K*Math.exp(-r*(T-t0))*Funcs.NormalCDF(-d2);
		}
		
		return fair;
	}
	
	public double Vega(double S, double r, double q, double sigma, double t0) {
		if (S<=0 || K<=0 || sigma<=0 || T-t0<=0){
			return 0;
		}
		double sqrt_T = Math.sqrt(T-t0);
		double d1 = (Math.log(S/K)+(r-q)*(T-t0))/(sigma*sqrt_T)+sigma*sqrt_T/2;
		
		return Math.exp(-q*(T-t0))*S*sqrt_T/Math.sqrt(2*Math.PI)*Math.exp(-d1*d1/2);
	}
	
	public double Delta(double S, double r, double q, double sigma, double t0) // true = call, false = put
	{
		if (S<=0 || K<=0 || sigma<=0 || T-t0<=0){
			return 0;
		}
		
		double d1 = (Math.log(S/K)+(r-q)*(T-t0))/(sigma*Math.sqrt(T-t0))+sigma*Math.sqrt(T-t0)/2;
		
		if (isCall){
			return Math.exp(-q*(T-t0))*Funcs.NormalCDF(d1);
		}
		else{
			return -Math.exp(-q*(T-t0))*Funcs.NormalCDF(-d1);
		}
	}
	
	public double Gamma(double S, double r, double q, double sigma, double t0) // true = call, false = put
	{
		if (S<=0 || K<=0 || sigma<=0 || T-t0<=0){
			return 0;
		}
		double d1 = (Math.log(S/K)+(r-q)*(T-t0))/(sigma*Math.sqrt(T-t0))+sigma*Math.sqrt(T-t0)/2;
		double gamma = Math.exp(-q*(T-t0))/(S*sigma*Math.sqrt(2*Math.PI*(T-t0)))*Math.exp(-d1*d1/2);
		return gamma;
	}	
	
	public double Theta(double S, double r, double q, double sigma, double t0) // true = call, false = put
	{
		if (S<=0 || K<=0 || sigma<=0 || T-t0<=0){
			return 0;
		}
		
		double d1 = (Math.log(S/K)+(r-q)*(T-t0))/(sigma*Math.sqrt(T-t0))+sigma*Math.sqrt(T-t0)/2;
		double d2 = d1 - sigma*Math.sqrt(T-t0);
		
		double part1 = -(Math.exp(-q*(T-t0))*S*sigma)/(2*Math.sqrt(2*Math.PI*(T-t0)))*Math.exp(-d1*d1/2);
		
		if (isCall){
			return (part1 - r*K*Math.exp(-r*(T-t0))*Funcs.NormalCDF(d2) + q*S*Math.exp(-q*(T-t0))*Funcs.NormalCDF(d1));
		}
		else{
			return (part1 + r*K*Math.exp(-r*(T-t0))*Funcs.NormalCDF(-d2) - q*S*Math.exp(-q*(T-t0))*Funcs.NormalCDF(-d1));
		}
	}
	
	public double Rho(double S, double r, double q, double sigma, double t0) // true = call, false = put
	{
		if (S<=0 || K<=0 || sigma<=0 || T-t0<=0){
			return 0;
		}
		
		double d1 = (Math.log(S/K)+(r-q)*(T-t0))/(sigma*Math.sqrt(T-t0))+sigma*Math.sqrt(T-t0)/2;
		double d2 = d1 - sigma*Math.sqrt(T-t0);
		
		if (isCall){
			return (T-t0)*Math.exp(-r*(T-t0))*(K*Funcs.NormalCDF(d2));
		}
		else{
			return -(T-t0)*Math.exp(-r*(T-t0))*(K*Funcs.NormalCDF(-d2));
		}
	}
	
	public double Impvol(double S, double r, double q, double sigma0, double t0,
			double target, double f_tol, double x_tol, int max_iter) {
		
		final double bad_vol = -1;
		if (S <= 0 || K <= 0 || sigma0 <= 0 || T-t0 <= 0 ){
			return bad_vol;
		}
		if (target <= 0 || f_tol <= 0 || x_tol <= 0 || max_iter <= 0){
			return bad_vol;
		}
		
		double[] ivol = new double[max_iter+1];
		ivol[0] = sigma0;
		
		for (int i = 0; i < max_iter; i++){
			double f = FairValue(S, r, q, ivol[i], t0) - target;
			if (Math.abs(f) <= f_tol) return ivol[i];
			
			double fprime = Vega(S, r, q, ivol[i], t0);
			if (fprime == 0) return bad_vol;
			
			ivol[i+1] = ivol[i] - f/fprime;	
			if (Math.abs(ivol[i+1]-ivol[i]) <= x_tol) return ivol[i+1];
		}
	
		return bad_vol;
	}
}

class Bond{
	private double F;
	private double c;
	private int n;
	
	public Bond(double F_, double c_, int n_) {
		F = F_;
		c = c_;
		n = n_;
	}

	double getF(){
		return F;
	}
	
	double getC(){
		return c;
	}
	
	int getN(){
		return n;
	}
	
	public double FairValue(double y) {
		if (F <= 0 || n <= 0 || c <= 0 || n < 0){
			return -1;
		}
		double sum = 0;
		double temp = 1+y/2;
		double df = 1;
		for(int i = 0; i < n; i++){
			df *= temp;
			sum += c/2/df;
		}
		sum += F/df;
		
		return sum;
	}
	
	public double Yield(double target, double f_tol, double x_tol, int max_iter) {
		final double bad_yield = -1;
		if (F <= 0 || n <= 0 || c <= 0){
			return bad_yield;
		}
		if (target <= 0 || f_tol <= 0 || x_tol <= 0 || max_iter <= 0){
			return bad_yield;
		}
		
		double y_low = 0.0, y_high = 0.01;
		double f_low = FairValue(y_low) - target;
		if (f_low < 0) return bad_yield;
		if (Math.abs(f_low) <= f_tol) return y_low;
		
		double f_high = FairValue(y_high) - target;
		if (Math.abs(f_high) <= f_tol) return y_high;
		while (f_high> 0){
			y_high *= 2;
			f_high = FairValue(y_high) - target;
			if (Math.abs(f_high) <= f_tol) return y_high;
		}
		
		double y_mid = 0.0;
		double f_mid = 0.0;
		for (int i = 0; i < max_iter; ++i){
			y_mid = (y_low + y_high) / 2.0;
			f_mid = FairValue(y_mid) - target;
			if (Math.abs(f_mid) < f_tol) return y_mid;
			
			if (f_low * f_mid > 0) {y_low = y_mid;}
			else {y_high = y_mid;}
			
			if (Math.abs(y_high - y_low) <= x_tol) return (y_low+y_high) / 2.0;
			
		}
		
		return bad_yield;
	}
}

final class Funcs{
	private Funcs() {} // private constructor
	
	public static double NormalPDF(double x) {
		return Math.exp(-0.5*x*x) / Math.sqrt(2.0*Math.PI);
	}
	
	public static double NormalCDF(double x) {
		if (x == 0.0) return 0.5;
		if (x < 0.0) return 1.0 - NormalCDF(-x);
		double[] b = { 0.2316419, 0.319381530, -0.356563782, 1.781477937, -1.821255978, 1.330274429 };
		double t = 1.0 / (1.0 + b[0] * x);
		double sum = t * (b[1] + t*(b[2] + t*(b[3] + t*(b[4] + t*(b[5])))));
		double result = 1.0 - NormalPDF(x) * sum;
		return result;
	}
}

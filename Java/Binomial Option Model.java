package javalab;

import java.util.Scanner;
import java.lang.Math;

public class HW4 {
	public static void main(String[] args){
		Scanner sc = new Scanner(System.in); 
		System.out.println("start Q1");
		Q1(sc); 
		System.out.println("End Q1");
		System.out.println("---------------------------------------------");
		
		System.out.println("done");
		sc.close(); 
	}
	
	public static void Q1(Scanner sc){		
		System.out.print("please enter K: ");
		double K = sc.nextDouble();
		System.out.print("please enter T: ");
		double T = sc.nextDouble();
		System.out.print("please enter N: ");
		int n = sc.nextInt();
		BinomialOption ec = new BinomialOption(K,T,true,false,n);
		BinomialOption ep = new BinomialOption(K,T,false,false,n);
		BinomialOption ac = new BinomialOption(K,T,true,true,n);
		BinomialOption ap = new BinomialOption(K,T,false,true,n);
		
		System.out.print("please enter S: ");
		double S0 = sc.nextDouble();
		System.out.print("please enter start time t0: ");
		double t0 = sc.nextDouble();
		System.out.print("please enter r: ");
		double r = sc.nextDouble() / 100;
		System.out.print("please enter q: ");
		double q = sc.nextDouble() / 100;
		System.out.print("please enter sigma: ");
		double sigma = sc.nextDouble();
		
		double ec_fv = ec.FairValue(S0, t0, r, q, sigma);
		double ep_fv = ep.FairValue(S0, t0, r, q, sigma);
		double ac_fv = ac.FairValue(S0, t0, r, q, sigma);
		double ap_fv = ap.FairValue(S0, t0, r, q, sigma);
		
		if(ec_fv != 0.0) System.out.printf("European Call: %.4f\n", ec_fv);
		else System.out.println("Bad inputs for European Call!\n");
		if(ep_fv != 0.0) System.out.printf("European Put: %.4f\n", ep_fv);
		else System.out.println("Bad inputs for European Put!\n");
		if(ac_fv != 0.0) System.out.printf("American Call: %.4f\n", ac_fv);
		else System.out.println("Bad inputs for American Call!\n");
		if(ap_fv != 0.0) System.out.printf("American Put: %.4f\n", ap_fv);
		else System.out.println("Bad inputs for American Put!\n");
		
		// --------------------------------------------------------------------------
		System.out.print("please enter a target price: ");
		double target = sc.nextDouble();
		
		double f_tol = 0.001, x_tol = 0.0001;
		int max_iter = 100;
		
		double ivol_ec = ec.impliedVolatility(target,S0,t0,r,q, f_tol, x_tol, max_iter);
		double ivol_ep = ep.impliedVolatility(target,S0,t0,r,q, f_tol, x_tol, max_iter);
		
		if (ivol_ec != -1) System.out.printf("implicit volatility of European call: %.2f%%\n",ivol_ec*100);
		else System.out.println("Cannot get implicit volatility for European Call, it is a bad volatility!\n");
		if (ivol_ep != -1) System.out.printf("implicit volatility of European put: %.2f%%\n", ivol_ep*100);
		else System.out.println("Cannot get implicit volatility for European Put, it is a bad volatility!\n");
		
		double ivol_ac = ac.impliedVolatility(target,S0,t0,r,q, f_tol, x_tol, max_iter);
		double ivol_ap = ap.impliedVolatility(target,S0,t0,r,q, f_tol, x_tol, max_iter);
		if (ivol_ac != -1) System.out.printf("implicit volatility of American call: %.2f%%\n",ivol_ac*100);
		else System.out.println("Cannot get implicit volatility for American Call, it is a bad volatility!\n");
		if (ivol_ap != -1) System.out.printf("implicit volatility of American put: %.2f%%\n", ivol_ap*100);
		else System.out.println("Cannot get implicit volatility for American Put, it is a bad volatility!\n");
	}
}

final class BinomialOption
{
	private double K;
	private double T;
	private boolean isCall;
	private boolean isAm;
	private int n;
	private double[][] S_tree;
	private double[][] V_tree;
	
	public BinomialOption(double K_, double T_, boolean isCall_, boolean isAm_, int n_) {
		K = K_;
		T = T_;
		isCall = isCall_;
		isAm = isAm_;
		
		n = n_;
		if (n < 1){
			n = 1;
		}
		
		S_tree = new double[n+1][n+1];
		V_tree = new double[n+1][n+1];
	}
	
	public double FairValue(double S0, double t0, double r, double q, double sigma) {
		double FV = 0;
		if ((S0 <= 0) || (K <= 0) || (sigma <= 0) || (t0 >= T)) return FV;
		
		double delta = (T-t0)/n;
		double u = Math.exp(sigma*Math.sqrt(delta));
		double d = 1/u;
		double p_up = (Math.exp((r-q)*delta)-d)/(u-d);
		double p_down = (u-Math.exp((r-q)*delta))/(u-d);
		double df = Math.exp(-1*r*delta);
		
		if (p_up <=0.0 || p_up >= 1.0) return 0.0;
		
		// Set the values of the stock prices in the stock tree (write some loops)
		S_tree[0][0]=S0;
		for(int i=1;i<n+1;i++){
			S_tree[i][0] = S_tree[i-1][0]*d;
			for(int j=1; j<i+1;j++){
				S_tree[i][j] = S_tree[i][j-1]*u*u;
			}
		}
		
		// Set the terminal payoff values in the valuation tree
		if(isCall){
			for (int j = 0; j <= n; j++) {
				V_tree[n][j] = Math.max(0,S_tree[n][j]-K);
			}
		}
		else{
			for (int j = 0; j <= n; j++) {
				V_tree[n][j] = Math.max(0,K-S_tree[n][j]);
			}
		}
		
		// Loop backwards through tree.
		if(isAm){
			for (int i = n-1; i >= 0; i--) {
				for (int j = 0; j <= i; j++) {
					double Vdisc = df * (p_down*V_tree[i+1][j] + p_up*V_tree[i+1][j+1]);
					double Vintr = 0.0;
					if(isCall) Vintr = Math.max(0, S_tree[i][j]-K);
					else Vintr = Math.max(0, K-S_tree[i][j]);
					V_tree[i][j] = Math.max(Vdisc, Vintr);
				}
			}
		}
		else{
			for (int i = n-1; i >= 0; i--) {
				for (int j = 0; j <= i; j++) {
					double Vdisc = df * (p_down*V_tree[i+1][j] + p_up*V_tree[i+1][j+1]);
					V_tree[i][j] = Vdisc;
				}
			}
		}
			
		FV = V_tree[0][0];
		return FV;
	}
	
	public double impliedVolatility(double target, double S0, double t0, double r, 
			double q, double f_tol, double v_tol, int max_iter) {
		
		final double bad_vol = -1;
		if ((target <= 0) || (S0 <= 0) || (t0 >= T)) return bad_vol;
		if ((f_tol <= 0) || (v_tol <= 0) || (max_iter <= 0)) return bad_vol;
		
		double sigma1 = 0.4;
		double FV1 = FairValue(S0, t0, r, q, sigma1);
		double diff_FV1 = FV1 - target;
		if (Math.abs(diff_FV1) <= f_tol) return sigma1;
		
		double scale = 1.0;
		if (diff_FV1 < 0.0) scale = 2.0;
		else if(diff_FV1 > 0.0) scale = 0.5;
		
		
		double sigma2 = sigma1;
		double FV2 = FV1;
		double diff_FV2 = FV2 - target;
		int i = 0;
		
		for (i = 0; i < max_iter; i++) {
			sigma2 *= scale; // scale the value of sigma2
			FV2 = FairValue(S0, t0, r, q, sigma2);
			diff_FV2 = FV2 - target;
			if (Math.abs(diff_FV2) <= f_tol) return sigma2; // passed tolerance test
			if (diff_FV1*diff_FV2 < 0.0) break; // found a bracket
		}
		if (i == max_iter) return bad_vol; // no bracket, return "fail"
		
		for (int iter= 0; iter < max_iter; iter++) {
			double sigma = (sigma1+sigma2)/2.0;
			double FV = FairValue(S0, t0, r, q, sigma);
			double diff_FV = FV - target;
			
			if(Math.abs(diff_FV) <= f_tol) return sigma;
			
			else if(diff_FV * diff_FV1 > 0) sigma1 = sigma;
			else sigma2 = sigma;
			
			if(Math.abs(sigma2-sigma1) <= v_tol){
				sigma = (sigma1+sigma2)/2.0;
				return sigma;
			}
		}
		
		return bad_vol;
	}
	
}

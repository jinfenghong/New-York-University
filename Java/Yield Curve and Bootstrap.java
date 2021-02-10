package javalab;

import java.util.Scanner;
import java.lang.Math;
import java.util.ArrayList;

public class FinalExam {
	public static void main(String[] args) {
		Scanner sc = new Scanner(System.in); 
		System.out.println("start Q1");
		Q1(sc); 
		System.out.println("End Q1");
		System.out.println("---------------------------------------------");
		
		System.out.println("done");
		sc.close(); 
	}
	public static void Q1(Scanner sc){
		System.out.println("Please enter n (n > 0): ");
		int n = sc.nextInt();
		while(n <= 0){
			System.out.println("Please enter n (n > 0): ");
			n = sc.nextInt();
		}
		
		double[] y = new double[n];
		
		for(int i = 0; i < n; i++){
			System.out.println("Please enter a par yield for "+i+": ");
			double x = sc.nextDouble()/100;
			y[i] = x;
		}
		
		YieldCurve yc = new YieldCurve(y);
		int size = yc.lenSpotCurve();
		for (int i = 0; i < size; i++) {
			// print tenor, spot rate, discount factor
			System.out.printf("Spot tenor: %f,\tSpot rate: %.2f%%,\tDiscount factor: %.6f%%\n",yc.SpotTenor(i),yc.SpotRate(i),yc.DiscountFactor(i));
		}
		
		System.out.println("Nunmber of times to caculte the interpolation: ");
	    int counts = sc.nextInt();
	    
		for(int i = 0; i < counts; i++){
			System.out.println("Please enter an interpolation time: ");
			double t = sc.nextDouble();
			
			System.out.printf("interpolation time: %f,\tlinear interpolation: %.2f%%,\tCFR interpolation: %.2f%%\n",t,yc.lin_interp(t),yc.cfr_interp(t));
		}		
	}
}

final class YieldCurve
{
	private ArrayList<Double> par_yields = new ArrayList<Double>();
	private ArrayList<Double> par_tenors = new ArrayList<Double>();
	private ArrayList<Double> discount_factors = new ArrayList<Double>();
	private ArrayList<Double> spot_rates = new ArrayList<Double>();
	private ArrayList<Double> spot_tenors = new ArrayList<Double>();
	
	public YieldCurve(double y[]) {
		for (int i = 0; i < y.length; i++) {
			par_yields.add(y[i]);
			par_tenors.add(0.5*(i+1));
		}
		bootstrap();
	}
	
	// public accessor methods
	public int lenParCurve() { return par_tenors.size(); }
	public int lenSpotCurve() { return spot_tenors.size(); }
	
	public Double DiscountFactor(int i){
		return discount_factors.get(i);
	}
	public Double SpotRate(int i){
		return spot_rates.get(i);
	}
	public Double SpotTenor(int i){
		return spot_tenors.get(i);
	}
	public Double ParYield(int i){
		return par_yields.get(i);
	}
	public Double ParTenor(int i){
		return par_tenors.get(i);
	}
	
	
	public double lin_interp(double t) {  // linear interpolation
		if(t <= SpotTenor(0)){
			return SpotRate(0);
		}
		if(t >= spot_tenors.get(lenSpotCurve()-1)){
			return spot_rates.get(lenSpotCurve()-1);
		}
		
		// to find the range that covers t
		int ind = 0;
		for(int i = 0; i < lenSpotCurve(); ++i){
			if(spot_tenors.get(i) > t){
				break;
			}
			ind = i;
		}
		
		double lambda = (t-spot_tenors.get(ind))/(spot_tenors.get(ind+1)-spot_tenors.get(ind));
		double r_int = (1-lambda)*spot_rates.get(ind)+lambda*spot_rates.get(ind+1);		
		
		return r_int;
	}
	public double cfr_interp(double t) { // CFR interpolation
		if(t <= spot_tenors.get(0)){
			return SpotRate(0);
		}
		if(t > spot_tenors.get(spot_tenors.size()-1)){
			return spot_rates.get(lenSpotCurve()-1);
		}
		
		// to find the range that covers t
		int ind = 0;
		for(int i = 0; i < spot_tenors.size(); ++i){
			if(spot_tenors.get(i) > t){
				break;
			}
			ind = i;
		}
		
		double lambda = (t-spot_tenors.get(ind))/(spot_tenors.get(ind+1)-spot_tenors.get(ind));
		double r_cfr = ((1-lambda)*spot_rates.get(ind)*spot_tenors.get(ind)+lambda*spot_rates.get(ind+1)*spot_tenors.get(ind+1))/t;		
		
		return r_cfr;
		
	}
	
	private void bootstrap() {
		double sum = 0;
		double df;
		for(int i = 0; i < par_tenors.size(); i++){
			df = (1 - 0.5 * ParYield(i) * sum)/ (1 + 0.5*ParYield(i));
			if(df <= 1.0 && df > 0.0){
				sum+=df;
				discount_factors.add(df);
				spot_rates.add(-1 * Math.log(DiscountFactor(i))/(0.5*(i+1)) * 100);
				spot_tenors.add(0.5*(i+1));
			}
			else break;
		}
	}

}

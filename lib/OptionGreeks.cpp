#include "OptionGreeks.h"
#include <cmath>

using namespace delta_exec::greeks;

OptionGreeks::OptionGreeks()
{

}
// Standard normal probability density function
double OptionGreeks::norm_pdf(const double x)
{
    return (1.0 / pow(2*M_PI, 0.5))*exp(-0.5 * x * x);
}

// An approximation to the cumulative distribution function
// for the standard normal distribution
double OptionGreeks::norm_cdf(const double x)
{
    return (0.5 * erfc(-x * M_SQRT1_2));
}


double OptionGreeks::d_j(const int j, const double S, const double K, const double r, const double v, const double T)
{
  return (log(S/K) + (r + (pow(-1,j-1))*0.5*v*v)*T)/(v*(pow(T,0.5)));
}


double OptionGreeks::call_price(const double S, const double K, const double r, const double v, const double T)
{
  return S * ( norm_cdf(d_j(1, S, K, r, v, T)) )- ( K*exp(-r*T) ) * ( norm_cdf(d_j(2, S, K, r, v, T)) );
}


double OptionGreeks::call_delta(const double S, const double K, const double r, const double v, const double T)
{
  return norm_cdf(d_j(1, S, K, r, v, T));
}


double OptionGreeks::call_gamma(const double S, const double K, const double r, const double v, const double T)
{
  return norm_pdf(d_j(1, S, K, r, v, T))/(S*v*sqrt(T));
}


double OptionGreeks::call_vega(const double S, const double K, const double r, const double v, const double T)
{
  double vega = S*norm_pdf(d_j(1, S, K, r, v, T))*sqrt(T);
  // in website it is mentioned as % change of underlying 
  return 100*vega/S;
}


double OptionGreeks::call_theta(const double S, const double K, const double r, const double v, const double T)
{
  double t = -(S*norm_pdf(d_j(1, S, K, r, v, T))*v)/(2*sqrt(T)) - r*K*exp(-r*T)*norm_cdf(d_j(2, S, K, r, v, T));
   // our formulae given in yearly basis i.e. per unit change in year to divide by 365 to get per unit day hange
  return t/365;
}

// Calculate the European vanilla call Rho
double OptionGreeks::call_rho(const double S, const double K, const double r, const double v, const double T)
{
  double rho =  K*T*exp(-r*T)*norm_cdf(d_j(2, S, K, r, v, T));
  //in website it is mentioned as per unit % change in Risk free return
  return rho/100;
}

// Calculate the European vanilla put price based on
// underlying S, strike K, risk-free rate r, volatility of
// underlying sigma and time to maturity T
double OptionGreeks::put_price(const double S, const double K, const double r, const double v, const double T)
{
  return -S*norm_cdf(-d_j(1, S, K, r, v, T))+K*exp(-r*T) * norm_cdf(-d_j(2, S, K, r, v, T));
}

// Calculate the European vanilla put Delta
double OptionGreeks::put_delta(const double S, const double K, const double r, const double v, const double T)
{
  return norm_cdf(d_j(1, S, K, r, v, T)) - 1;
}

// Calculate the European vanilla put Gamma
double OptionGreeks::put_gamma(const double S, const double K, const double r, const double v, const double T)
{
  return call_gamma(S, K, r, v, T); // Identical to call by put-call parity
}

// Calculate the European vanilla put Vega
double OptionGreeks::put_vega(const double S, const double K, const double r, const double v, const double T)
{
  return call_vega(S, K, r, v, T); // Identical to call by put-call parity
}

// Calculate the European vanilla put Theta
double OptionGreeks::put_theta(const double S, const double K, const double r, const double v, const double T) 
{
  double t = -(S*norm_pdf(d_j(1, S, K, r, v, T))*v)/(2*sqrt(T)) + r*K*exp(-r*T)*norm_cdf(-d_j(2, S, K, r, v, T));
   // our formulae given in yearly basis i.e. per unit change in year to divide by 365 to get per unit day hange
  return t/365;
}

// Calculate the European vanilla put Rho
double OptionGreeks::put_rho(const double S, const double K, const double r, const double v, const double T)
{
  double rho = -T*K*exp(-r*T)*norm_cdf(-d_j(2, S, K, r, v, T));
  //in website it is mentioned as per unit % change in Risk free return
  return rho/100;
}

double OptionGreeks::impVol(double S, double K, double T, double marketprice, bool isCallOption, double r)
{
    double impVol = 0.25;
    double ivLow = 0.01;
    double ivHigh = 5.00;

    double maxError = 0.00001;
    int maxITer = 100;
    int iterCount = 0;

    double price = optionPrice(S, K, r, impVol, T, isCallOption);
    double diff = marketprice - price;

    while (abs(diff) > maxError)
    {
        if (diff < 0)
        {
            ivHigh = impVol;
            impVol = (impVol + ivLow)/2.0;
        }
        else
        {
            ivLow = impVol;
            impVol = (impVol + ivHigh)/2.0;
        }
        price = optionPrice(S, K, r, impVol, T, isCallOption );
        diff = marketprice - price;
        iterCount += 1;

        if (iterCount == maxITer)
            return impVol;
    }
    return impVol;
}
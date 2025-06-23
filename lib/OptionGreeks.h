#pragma once
#include <string>

namespace delta_exec
{
    namespace greeks
    {
        class OptionGreeks
        {
            public:
                OptionGreeks();
                double norm_pdf(const double x);
                double norm_cdf(const double x);
                double d_j(const int j, const double S, const double K, const double r, const double v, const double T);
                double call_price(const double S, const double K, const double r, const double v, const double T);
                double call_delta(const double S, const double K, const double r, const double v, const double T);
                double call_gamma(const double S, const double K, const double r, const double v, const double T);
                double call_vega(const double S, const double K, const double r, const double v, const double T);
                double call_theta(const double S, const double K, const double r, const double v, const double T);
                double call_rho(const double S, const double K, const double r, const double v, const double T);
                double put_price(const double S, const double K, const double r, const double v, const double T);
                double put_delta(const double S, const double K, const double r, const double v, const double T);
                double put_gamma(const double S, const double K, const double r, const double v, const double T);
                double put_vega(const double S, const double K, const double r, const double v, const double T);
                double put_theta(const double S, const double K, const double r, const double v, const double T);
                double put_rho(const double S, const double K, const double r, const double v, const double T);





                double impVol(double stkPrx, double strike, double tte, double optPx, bool isCallOption, double interestRate);

                double optionPrice(const double S, const double K, const double r, const double v, const double T, bool isCallOption)
                {
                    if(isCallOption)
                    {
                        return call_price( S,  K,  r,  v,  T);
                    }
                    return put_price( S,  K,  r,  v,  T);
                }


        };
    }
}
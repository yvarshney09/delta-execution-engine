#pragma once
#include <string>
#include <cmath>
const double OneOverRootTwoPi = 0.398942280401433;
namespace delta_exec
{
    namespace greeks
    {
        class OptionGreeks
        {
            public:
                OptionGreeks();
                //double norm_pdf(const double x);
                //double norm_cdf(const double x);
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

                inline double norm_pdf(const double x)
                {
                    return (1.0 / pow(2*M_PI, 0.5))*exp(-0.5 * x * x);
                }

                inline double norm_cdf(const double x)
                {
                    return (0.5 * erfc(-x * M_SQRT1_2));
                }


                inline double CumulativeNormal(double x) {
                    static double a[5] = {0.319381530, -0.356563782, 1.781477937, -1.821255978, 1.330274429};

                    double result;
                    if (x < -7.0)
                        result = NormalDensity(x) / sqrt(1. + x * x);
                    else {
                        if (x > 7.0) {
                            result = 1.0 - CumulativeNormal(-x);
                        } else {
                            const double tmp = 1.0 / (1.0 + 0.2316419 * fabs(x));
                            result = 1 - NormalDensity(x) *
                                (tmp * (a[0] + tmp * (a[1] + tmp * (a[2] + tmp * (a[3] + tmp * a[4])))));

                            if (x <= 0.0) result = 1.0 - result;
                        }
                    }

                    return result;
                }

                inline double NormalDensity(double x) { return OneOverRootTwoPi * exp(-x * x / 2); }



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
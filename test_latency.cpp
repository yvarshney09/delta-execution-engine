#include <iostream>
#include <chrono>
#include "StrategyExecution.h"
#include "OptionGreeks.h"
using namespace delta_exec::greeks;




void test_greeks_latency() {
    OptionGreeks greeks;
    double S = 100, K = 100, r = 0.07, v = 0.2, T = 15.0/365.0;
    auto start = std::chrono::high_resolution_clock::now();
    double call_delta = greeks.call_delta(S, K, r, v, T);
    double put_delta = greeks.put_delta(S, K, r, v, T);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::micro> duration = end - start;
    std::cout << "OptionGreeks delta calculation latency: " << duration.count() << " us" << std::endl;
    std::cout << "Call delta: " << call_delta << ", Put delta: " << put_delta << std::endl;

    // IV (implied volatility) latency test
    double market_price = greeks.call_price(S, K, r, v, T); // Use model price as market price for test
    start = std::chrono::high_resolution_clock::now();
    double iv = greeks.impVol(S, K, T, market_price, true, r);
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "OptionGreeks implied volatility (IV) calculation latency: " << duration.count() << " us" << std::endl;
    std::cout << "Implied Volatility: " << iv << std::endl;
}

int main() {
    StrategyExecution strat;
    test_greeks_latency();
    return 0;
}

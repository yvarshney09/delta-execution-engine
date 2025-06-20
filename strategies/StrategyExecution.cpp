#include "StrategyExecution.h"
#include <iostream>

StrategyExecution::StrategyExecution() {
    std::cout << "StrategyExecution constructed\n";
}

void StrategyExecution::run() {
    std::cout << "Running strategy...\n";
    OnTimerCallback();
}

void StrategyExecution::OnMarketData() {
    std::cout << "Market data received\n";
}

void StrategyExecution::OnOrderResponse() {
    std::cout << "Order response received\n";
}

void StrategyExecution::OnTimerCallback() {
    std::cout << "Timer callback triggered\n";
}

#pragma once
#include <iostream>
class StrategyExecution {
public:
    StrategyExecution();
    void run();

    // Callbacks
    void OnMarketData();       // Placeholder
    void OnOrderResponse();    // Placeholder
    void OnTimerCallback();    // Placeholder
};

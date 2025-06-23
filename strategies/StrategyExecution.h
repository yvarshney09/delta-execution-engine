#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <chrono>
#include <utility>
#include <algorithm>
#include <random>
#include "OptionGreeks.h"
using namespace delta_exec::greeks;
constexpr int BOOK_DEPTH = 10;

struct MarketData {
    uint32_t _instrumentId;
    
    double _bidPrices[BOOK_DEPTH];
    int _bidQtys[BOOK_DEPTH];
    double _askPrices[BOOK_DEPTH];
    int _askQtys[BOOK_DEPTH];

    int _lastTradedQty;
    double _lastTradedPrice;
    uint64_t _lastTradedTime; // in nanoseconds since epoch
};
enum class OptionType {
    Call = 0,
    Put = 1
};
struct Instrument{

    int _instrumentId; // Unique identifier for the instrument
    int _indexInOptionChain; // Index in the option chain
    OptionType _optionType;
    uint64_t _expiry; // Expiry time in nanoseconds since epoch
    double _strikePrice; // Strike price of the option
};//it has all info including market data, instrument type, strike, expiry, etc.

class InstrumentManager {
public:
    const Instrument* GetInstrumentInfo(const uint32_t& instrumentId) const{
        // This function would typically look up the instrument information from a database or in-memory structure
        // For now, it's a placeholder that returns nullptr.
        return nullptr; // Placeholder for actual implementation
    };
};
struct OrderResponse{
    int _instrumentId; // Unique identifier for the instrument
    int _qty;
    int _price;
};
struct OrderRequest {
    int _instrumentId; // Unique identifier for the instrument
    double _price; // Price at which the order is placed
    int _quantity; // Quantity of the order i lots
    bool _isBuy; // True if the order is a buy order, false if it's

};

struct SyntheticCandidate {
    double _syntheticForwardPrice;
    int _index;
};
class EWMA {
private:
    double _alpha = 0.1; // Smoothing factor for EWMA, default is 0.1
    double _value = 0.0; // Current value of the EWMA
    bool _initialized = false; // Flag to check if the EWMA has been initialized

public:
    
    void setAlpha(double alpha) {
        _alpha = alpha;
    }
    void update(double x) {
        if (!_initialized) {
            _value = x;
            _initialized = true;
        } else {
            _value = _alpha * x + (1.0 - _alpha) * _value;
        }
    }

    double value() const {
        return _value;
    }

    bool isInitialized() const {
        return _initialized;
    }
};

class StrategyExecution {
    
    private:

    void updateImpliedForwardAndSyntheticCandidatePrices();
    void updateATMStrike() {

        for (int i = 0; i < _strikePrices.size(); ++i) {
            if (_strikePrices[i] <= _previousImpliedForward) {
               
                 _indexForAtmStrike = i;
            } else {
                break; 
            }
        }

        
    }
    double computeVWAP(const double* prices, const int* qtys, int depth, double targetQty) {
        int remaining = targetQty;
        double totalPriceQty = 0;
        int totalQty = 0;

        for (int i = 0; i < depth; ++i) {
            int takeQty = std::min(remaining, qtys[i]);
            totalPriceQty += prices[i] * takeQty;
            totalQty += takeQty;
            remaining -= takeQty;

            if (remaining <= 0)
                break;
        }

        if (totalQty == 0)
            return -1.0;

        return totalPriceQty / totalQty;
    }
    void updateMarketDataSubscription(){
        // This function would typically interact with the platform's API to update the subscription list
        // for market data based on the current option chain and market data.
        // For now, it's a placeholder.
        if (_previousIndexForAtmStrike != _indexForAtmStrike) {
            // If the ATM strike has changed, we need to update the market data subscription
            // to include the new ATM strike and its surrounding strikes.
            _previousIndexForAtmStrike = _indexForAtmStrike;
            // Update the subscription list based on the current option chain
            // This is a placeholder for actual implementation
            // updateSubscriptionList();
        }
    };// Placeholder for actual implementation where using some api of the platform the subscription list will be updated.
    void updateAverageDeltaAcquisitionRate() {
        // This function calculates the average delta acquisition rate based on the total delta to acquire
        // and the time to acquire it. It updates the _averageDeltaAcquisitionRate variable.
        auto now = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        if (_endTime > now && _startTime < _endTime) {
            _averageDeltaAcquisitionRate = std::max(1, static_cast<int>(((_deltaToAcquire - _delataAcquired) * _orderFiringInterval) / (_endTime - now)));
        } 
    }
private:
    InstrumentManager* _instrumentManager = nullptr; // Pointer to the instrument manager
    EWMA _ewmaManagerTradeVolume; // Exponentially Weighted Moving Average for trade volume
    OptionGreeks _optionGreek; // Option Greeks calculator
    double _deltaToAcquire = 0;// Total delta to acquire in the strategy
    double _delataAcquired = 0.0; // Total delta acquired so far
    double _intrestRate = 0.07; // Interest rate for option pricing, default is 7%
    double _previousImpliedForward = 0.0; // Previous implied forward price
    double _tradeVolumeMultiplier = 1.0; // Trade volume multiplier for the strategy, default is 1.0
    double _shortTermTradeVolume = 0;
    double _ewmaTradeVolume = 0;
    double _ewmaTradeVolumeAlpha = 0.1; // EWMA alpha for trade volume, default is 0.1


    int _averageDeltaAcquisitionRate = 1; // Average delta acquisition rate in lots per order firing interval
    int _currentDeltaAcquisitionRate = 1; // Current delta acquisition rate in lots per order firing interval
    int _numberOfStrikesToCalcualteForward = 4; // Number of strikes to consider for forward calculation
    int _previousIndexForAtmStrike = 0; // Previous index of the ATM strike in the strike prices vector
    int _indexForAtmStrike = 0; // Index of the ATM strike(max strike less than impled forward ) in the strike prices vector
    int _numberOfStrikesToAcquireDelta = 10; // Number of strikes to acquire delta, default is 10
    
    uint32_t  _timeToAcquireDelta;//in minutes
    uint64_t _lastDeltaOrderTriggerTime = 0; // Timestamp of the last delta order trigger in nanoseconds
    uint64_t _orderFiringInterval = 1000000; // Interval for firing orders, in nanoseconds, keeping it 1 second fixed for now
    uint64_t _startTime = 0; // Start time of the strategy execution in nanoseconds
    uint64_t _endTime = 0; // End time of the strategy execution in nanoseconds
    uint64_t _lastTimerCallbackTime = 0; // Last time the timer callback was executed in nanoseconds

    
    std::vector<Instrument*> _callOptionChain = {};// sorted by strike ascending
    std::vector<Instrument*> _putOptionChain = {};// sorted by strike ascending
    std::vector<MarketData*> _marketDataCalls = {}; // Market data for intrested calls in ascending order with strike prices
    std::vector<MarketData*> _marketDataPuts = {}; // Market data for intrested puts in ascending order with strike prices
    std::vector<double> _strikePrices = {}; // Sorted list of strike prices to quickly search for index
    std::vector<SyntheticCandidate> _forwardPricesForStrikes = {}; // Forward prices for the strikes for the intrested strikes with index
public:
    StrategyExecution();
    void run();

    // Callbacks

    void OnMarketData(MarketData* bookUpdate);       // Placeholder
    void OnOrderResponse(OrderResponse orderResponse);   // Placeholder
    void OnTimerCallback(uint64_t time);    // Placeholder
    void SetDeltaToAcquire(double delta, uint32_t timeToAcquireDelta) {
         _deltaToAcquire = delta;
         _timeToAcquireDelta = timeToAcquireDelta;
        _startTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        _endTime = _startTime + timeToAcquireDelta * 60 * 1e9; 
        _averageDeltaAcquisitionRate = std::max(1, static_cast<int>((delta * _orderFiringInterval) / (_endTime - _startTime) )); 
    }
    void submitCallBuyOrder(uint32_t instrumentId, int qtyInLots, double price) {}
    void submitPutSellOrder(uint32_t instrumetId, int qtyInLots, double price) {}
    
};

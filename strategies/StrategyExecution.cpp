#include "StrategyExecution.h"
#include <iostream>

StrategyExecution::StrategyExecution() {
    //initalize option chains
    //configure timer callbacks, etc.
    //subscribe to market data
    
    _marketDataCalls.resize(_numberOfStrikesToAcquireDelta);
    _marketDataPuts.resize(_numberOfStrikesToAcquireDelta);

    _ewmaManagerTradeVolume.setAlpha(_ewmaTradeVolumeAlpha);
}

void StrategyExecution::run() {
    
}

void StrategyExecution::OnMarketData(MarketData* bookUpdate) {
   int tradedQty = bookUpdate->_lastTradedQty;
   const uint32_t instrumentId = bookUpdate->_instrumentId; // Assuming single instrument ID in the array
   int optionIndex = _instrumentManager->GetInstrumentInfo(instrumentId)->_indexInOptionChain;
   OptionType  optionType = _instrumentManager->GetInstrumentInfo(instrumentId)->_optionType;
   if (optionType == OptionType::Call) {
       _marketDataCalls[optionIndex - (_indexForAtmStrike - 4)] = bookUpdate;
   } else if (optionType == OptionType::Put) {
       _marketDataPuts[optionIndex - (_indexForAtmStrike - 4)] = bookUpdate;
   }

   _shortTermTradeVolume += tradedQty;
}

void StrategyExecution::OnOrderResponse(OrderResponse orderResponse) {
    // This function will process the order response then calculate the delta acquired
    // and update the total delta acquired so far. Then update the average delta acquisition rate.
    if (orderResponse._responseType != ResponseType::TRADE) {
        return; // Only process trade responses for now
    }
    int qty = orderResponse._qty;// assuming qty is in lots
    auto instrumentInfo = _instrumentManager->GetInstrumentInfo(orderResponse._instrumentId);
    double strikePrice = instrumentInfo->_strikePrice;
    OptionType optionType = instrumentInfo->_optionType;
    double delta = 0.0;
    uint64_t timeToExpiry = instrumentInfo->_expiry - _lastTimerCallbackTime; // Time to expiry in nanoseconds
    double timeToExpiryInYears = static_cast<double>(timeToExpiry) / (365.0 * 24 * 60 * 60 * 1e9); // Convert to years
    if (optionType == OptionType::Call) {
        double volatility = _optionGreek.impVol(strikePrice, strikePrice, timeToExpiryInYears, orderResponse._price, true, _intrestRate);
        delta = _optionGreek.call_delta(_previousImpliedForward, strikePrice, _intrestRate, volatility, timeToExpiryInYears); 
    } else if (optionType == OptionType::Put) {
        double volatility = _optionGreek.impVol(_previousImpliedForward, strikePrice, timeToExpiryInYears, orderResponse._price, false, _intrestRate);
        delta = _optionGreek.put_delta(_previousImpliedForward, strikePrice, _intrestRate, volatility, timeToExpiryInYears); 
    }
    _delataAcquired += delta * qty; // Update total delta acquired
    updateAverageDeltaAcquisitionRate();
}

void StrategyExecution::OnTimerCallback(uint64_t time) {//called every second
    _lastTimerCallbackTime = time;
    _tradeVolumeMultiplier = 1.0; // reset trade volume multiplier
    _currentDeltaAcquisitionRate = _averageDeltaAcquisitionRate; // reset current delta acquisition rate
    if (_shortTermTradeVolume > 0 && _ewmaTradeVolume > 0) { 
        _tradeVolumeMultiplier = _shortTermTradeVolume / _ewmaTradeVolume; // calculate trade volume multiplier
    }
    _currentDeltaAcquisitionRate = std::max(1, static_cast<int>(_currentDeltaAcquisitionRate * _tradeVolumeMultiplier)); // adjust current delta acquisition rate based on trade volume multiplier
    _ewmaManagerTradeVolume.update(_shortTermTradeVolume);
    _ewmaTradeVolume = _ewmaManagerTradeVolume.value();
    _shortTermTradeVolume = 0; // reset short term trade volume after processing
    updateImpliedForwardAndSyntheticCandidatePrices();

    int numTargets = std::min<int>(5, _forwardPricesForStrikes.size());
    int deltaPerTarget = (_currentDeltaAcquisitionRate + numTargets - 1) / numTargets;
    
    int allocatedDelta = 0;
    for (int i = 0; i < numTargets; ++i) {
        int remainingDelta = _currentDeltaAcquisitionRate - allocatedDelta;
        int thisDelta = std::min(deltaPerTarget, remainingDelta);

        if (thisDelta <= 0) break;
        int strikeIndex = _forwardPricesForStrikes[i]._index;

        MarketData* callMD = _marketDataCalls[strikeIndex - _indexForAtmStrike + 4];
        MarketData* putMD  = _marketDataPuts[strikeIndex - _indexForAtmStrike + 4];

        if (!callMD || !putMD) continue;
        double callPrice = callMD->_askPrices[10];// fire deep ioc order
        double putPrice  = putMD->_bidPrices[10];// fire deep ioc order
        // Submit buy on call ask side, sell on put bid side
        submitCallBuyOrder(callMD->_instrumentId, thisDelta, callPrice);
        submitPutSellOrder(putMD->_instrumentId, thisDelta, putPrice);
        allocatedDelta += thisDelta;
    }
    
    updateATMStrike(); 
}
void StrategyExecution::updateImpliedForwardAndSyntheticCandidatePrices() {
    _forwardPricesForStrikes.clear();  // Clear previous values
    double sum = 0.0;
    int count = 0;
    for (int i = 0; i < _marketDataCalls.size(); ++i) {
        MarketData* callMD = _marketDataCalls[i];
        MarketData* putMD  = _marketDataPuts[i];

        if (!callMD || !putMD || !_instrumentManager)
            continue;

        const Instrument* instr = _instrumentManager->GetInstrumentInfo(callMD->_instrumentId);
        if (!instr) continue;

        double strike = instr->_strikePrice;

        double callVWAP = computeVWAP(callMD->_askPrices, callMD->_askQtys, BOOK_DEPTH, _currentDeltaAcquisitionRate);
        double putVWAP  = computeVWAP(putMD->_bidPrices, putMD->_bidQtys, BOOK_DEPTH, _currentDeltaAcquisitionRate);

        if (callVWAP < 0 || putVWAP < 0)
            continue;

        double syntheticPrice = callVWAP - putVWAP + strike;

        _forwardPricesForStrikes.push_back({syntheticPrice, instr->_indexInOptionChain});
        count++;
        sum += syntheticPrice;
    }
    std::sort(_forwardPricesForStrikes.begin(), _forwardPricesForStrikes.end(),
        [](const SyntheticCandidate& a, const SyntheticCandidate& b) {
            return a._syntheticForwardPrice < b._syntheticForwardPrice;
        });
    _previousImpliedForward = (count > 0) ? (sum / count) : _previousImpliedForward; // Calculate the average implied forward price
}

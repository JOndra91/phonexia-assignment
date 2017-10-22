#pragma once

#include <set>
#include <string>

namespace billing {

  struct Tariff {
    float callInNetworkFee;
    float callOutNetworkFee;
    float smsInNetworkFee;
    float smsOutNetworkFee;
    float monthlyFee;

    /** Minimal duration of first charged unit. */
    unsigned firstUnitDuration;
    /** Minimal duration of following charged units. */
    unsigned otherUnitsDuration;

    unsigned freeCallSeconds;
    unsigned freeSms;

    std::set<std::string> freeNumbers;
  };
}

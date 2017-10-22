#pragma once

namespace billing {

  struct Bill {
    unsigned callSecondsInNetwork = 0;
    unsigned callSecondsOutNetwork = 0;
    unsigned smsInNetwork = 0;
    unsigned smsOutNetwork = 0;

    unsigned callSecondsFree = 0;
    unsigned smsFree = 0;

    float callInNetworkCharged = 0.0f;
    float callOutNetworkCharged = 0.0f;
    float smsInNetworkCharged = 0.0f;
    float smsOutNetworkCharged = 0.0f;

    float totalFee = 0.0f;
  };
}

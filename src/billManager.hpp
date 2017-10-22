#pragma once

#include <map>
#include <string>

#include "bill.hpp"
#include "record.hpp"
#include "tariff.hpp"

namespace billing {

  class BillManager {
    private:
      Bill bill;
      Tariff tariff;

    public:
      BillManager() {};
      BillManager(Tariff t) : tariff(t) {};

      void processRecord(Record record);

      inline Bill getBill() {
        bill.totalFee = tariff.monthlyFee
            + bill.callInNetworkCharged
            + bill.callOutNetworkCharged
            + bill.smsInNetworkCharged
            + bill.smsOutNetworkCharged;

        return bill;
      }
  };
}

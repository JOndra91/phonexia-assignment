#include <tuple>

#include "billManager.hpp"

using namespace std;

namespace billing {

  inline void processSms(Record record, Tariff tariff, Bill &bill) {
    float fee, *charged;
    if (record.inNetwork) {
      bill.smsInNetwork++;
      fee = tariff.smsInNetworkFee;
      charged = &bill.smsInNetworkCharged;
    }
    else {
      bill.smsOutNetwork++;
      fee = tariff.smsOutNetworkFee;
      charged = &bill.smsOutNetworkCharged;
    }

    unsigned total = bill.smsInNetwork + bill.smsOutNetwork - bill.smsFree;

    if (tariff.freeNumbers.find(record.recipient) != tariff.freeNumbers.end()) {
      bill.smsFree++;
    }
    else if (total > tariff.freeSms) {
      *charged += fee;
    }
  }

  inline void processVoice(Record record, Tariff tariff, Bill &bill) {
    float fee, *charged;

    unsigned units = tariff.firstUnitDuration;
    if (record.duration > tariff.firstUnitDuration) {
      unsigned restDuration = record.duration - tariff.firstUnitDuration;
      unsigned otherUnits = (restDuration / tariff.otherUnitsDuration);
      unsigned restUnit = (restDuration % tariff.otherUnitsDuration != 0);

      units = (otherUnits + restUnit) * tariff.otherUnitsDuration;
      units += tariff.firstUnitDuration;
    }

    if (record.inNetwork) {
      bill.callSecondsInNetwork += units;
      fee = tariff.callInNetworkFee;
      charged = &bill.callInNetworkCharged;
    }
    else {
      bill.callSecondsOutNetwork += units;
      fee = tariff.callOutNetworkFee;
      charged = &bill.callOutNetworkCharged;
    }

    unsigned total = bill.callSecondsInNetwork + bill.callSecondsOutNetwork - bill.callSecondsFree;

    if (tariff.freeNumbers.find(record.recipient) != tariff.freeNumbers.end()) {
      bill.callSecondsFree += units;
    }
    else if (total > tariff.freeCallSeconds) {
      if (total - units <= tariff.freeCallSeconds) {
        units = total - tariff.freeCallSeconds;
      }
      *charged += fee * units;
    }
  }

  void BillManager::processRecord(Record record) {
    switch (record.type) {
      case RecordType::Voice:
        processVoice(record, tariff, bill);
        break;

      case RecordType::Sms:
        processSms(record, tariff, bill);
        break;
    }
  }
}

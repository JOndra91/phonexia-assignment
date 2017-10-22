#include <random>
#include <vector>

#include <catch.hpp>

#include <bill.hpp>
#include <billManager.hpp>
#include <record.hpp>
#include <tariff.hpp>

using namespace billing;

const Tariff basicTariff = {
  .callInNetworkFee = 1,
  .callOutNetworkFee = 2,
  .smsInNetworkFee = 1.0,
  .smsOutNetworkFee = 2.0,
  .monthlyFee = 0,
  .firstUnitDuration = 1,
  .otherUnitsDuration = 1,

  .freeCallSeconds = 0,
  .freeSms = 0,

  .freeNumbers = {"+420999888777", "+420666555444", "+420333222111"},
};

const Record basicVoiceRecord = {
  .recipient = "+420123456789",
  .duration = 1,
  .type = RecordType::Voice,
  .inNetwork = true,
};

TEST_CASE("Monthly fee") {
  Bill bill;
  BillManager manager;
  Tariff tariff = basicTariff;

  std::default_random_engine generator;
  std::uniform_real_distribution<float> distribution(0.0f, 9999.9f);

  // QuickCheck like test
  for(int i = 0; i < 1000; ++i) {
    tariff.monthlyFee = distribution(generator);
    manager = BillManager(tariff);
    bill = manager.getBill();

    INFO(bill.totalFee)
    REQUIRE(bill.totalFee == tariff.monthlyFee);
  }
}

TEST_CASE("Basic voice billing") {
  Bill bill;
  BillManager manager(basicTariff);
  Record record = basicVoiceRecord;

  SECTION("Voice within network") {
    record.inNetwork = true;

    manager.processRecord(record);
    bill = manager.getBill();

    REQUIRE(bill.callSecondsInNetwork == record.duration);
    REQUIRE(bill.callInNetworkCharged == basicTariff.callInNetworkFee);
    REQUIRE(bill.callSecondsOutNetwork == 0);
    REQUIRE(bill.callOutNetworkCharged == 0.0f);
    REQUIRE(bill.totalFee == basicTariff.callInNetworkFee);
  }

  SECTION("Voice outside network") {
    record.inNetwork = false;

    manager.processRecord(record);
    bill = manager.getBill();

    REQUIRE(bill.callSecondsInNetwork == 0);
    REQUIRE(bill.callInNetworkCharged == 0.0f);
    REQUIRE(bill.callSecondsOutNetwork == record.duration);
    REQUIRE(bill.callOutNetworkCharged == basicTariff.callOutNetworkFee);
    REQUIRE(bill.totalFee == basicTariff.callOutNetworkFee);
  }
}

TEST_CASE("Basic SMS billing") {
  Bill bill;
  BillManager manager(basicTariff);
  Record record = basicVoiceRecord;
  record.type = RecordType::Sms;

  SECTION("SMS within network") {
    record.inNetwork = true;

    manager.processRecord(record);
    bill = manager.getBill();

    REQUIRE(bill.smsInNetwork == 1);
    REQUIRE(bill.smsInNetworkCharged == basicTariff.smsInNetworkFee);
    REQUIRE(bill.smsOutNetwork == 0);
    REQUIRE(bill.smsOutNetworkCharged == 0.0f);
    REQUIRE(bill.totalFee == basicTariff.smsInNetworkFee);
  }

  SECTION("SMS outside network") {
    record.inNetwork = false;

    manager.processRecord(record);
    bill = manager.getBill();

    REQUIRE(bill.smsInNetwork == 0);
    REQUIRE(bill.smsInNetworkCharged == 0.0f);
    REQUIRE(bill.smsOutNetwork == 1);
    REQUIRE(bill.smsOutNetworkCharged == basicTariff.smsOutNetworkFee);
    REQUIRE(bill.totalFee == basicTariff.smsOutNetworkFee);
  }
}

TEST_CASE("First voice unit billing") {
  Bill bill;
  Tariff tariff = basicTariff;
  Record record = basicVoiceRecord;

  tariff.firstUnitDuration = 30;
  tariff.otherUnitsDuration = 15;

  BillManager manager(tariff);

  SECTION("Duration is shorter than first unit") {
    record.duration = 7;

    manager.processRecord(record);
    bill = manager.getBill();

    REQUIRE(bill.callSecondsInNetwork == tariff.firstUnitDuration);
  }

  SECTION("Duration is same as first unit") {
    record.duration = tariff.firstUnitDuration;

    manager.processRecord(record);
    bill = manager.getBill();

    REQUIRE(bill.callSecondsInNetwork == 30);
  }

  SECTION("Duration is just larger than first unit") {
    record.duration = 31;

    manager.processRecord(record);
    bill = manager.getBill();

    REQUIRE(bill.callSecondsInNetwork == 45);
  }

  SECTION("Duration is larger than first unit and just smaller than other units") {
    record.duration = 44;

    manager.processRecord(record);
    bill = manager.getBill();

    REQUIRE(bill.callSecondsInNetwork == 45);
  }

  SECTION("Duration is first unit plus multiple of other units") {
    record.duration = 60;

    manager.processRecord(record);
    bill = manager.getBill();

    REQUIRE(bill.callSecondsInNetwork == 60);
  }

  SECTION("Duration is much larger than first unit") {
    record.duration = 327;

    manager.processRecord(record);
    bill = manager.getBill();

    REQUIRE(bill.callSecondsInNetwork == 330);
  }
}

TEST_CASE("Free numbers") {
  Bill bill;
  BillManager manager(basicTariff);
  Record record = basicVoiceRecord;

  for(auto freeNumber : basicTariff.freeNumbers) {
    record.recipient = freeNumber;
    manager.processRecord(record);
  }

  bill = manager.getBill();

  REQUIRE(bill.callSecondsInNetwork == basicTariff.freeNumbers.size());
  REQUIRE(bill.callInNetworkCharged == 0.0f);
}

TEST_CASE("Free voice units") {
  Bill bill;
  Tariff tariff = basicTariff;
  Record record = basicVoiceRecord;

  tariff.freeCallSeconds = 30;

  BillManager manager(tariff);

  SECTION("Still have free voice units") {
    record.duration = 20;

    manager.processRecord(record);
    bill = manager.getBill();

    REQUIRE(bill.callSecondsInNetwork == 20);
    REQUIRE(bill.callInNetworkCharged == 0.0f);
  }

  SECTION("Exceeded free voice units") {
    record.duration = 50;

    manager.processRecord(record);
    bill = manager.getBill();

    REQUIRE(bill.callSecondsInNetwork == 50);
    REQUIRE(bill.callInNetworkCharged == 20 * basicTariff.callInNetworkFee);
  }
}

TEST_CASE("Free SMS units") {
  Bill bill;
  Tariff tariff = basicTariff;
  Record record = basicVoiceRecord;

  record.type = RecordType::Sms;
  tariff.freeSms = 5;

  BillManager manager(tariff);

  SECTION("Still have free SMS units") {
    for (int i = 0; i < 5; ++i) {
      manager.processRecord(record);
    }
    bill = manager.getBill();

    REQUIRE(bill.smsInNetwork == 5);
    REQUIRE(bill.smsInNetworkCharged == 0.0f);
  }

  SECTION("Exceeded free SMS units") {
    for (int i = 0; i < 50; ++i) {
      manager.processRecord(record);
    }
    bill = manager.getBill();

    REQUIRE(bill.smsInNetwork == 50);
    REQUIRE(bill.smsInNetworkCharged == 45 * basicTariff.smsInNetworkFee);
  }
}

TEST_CASE("Cummulative billing") {
  Tariff tariff = {
    .callInNetworkFee = 1.5 / 60.0,
    .callOutNetworkFee = 2.5 / 60.0,
    .smsInNetworkFee = 1.25,
    .smsOutNetworkFee = 2.25,
    .monthlyFee = 314.15,
    .firstUnitDuration = 30,
    .otherUnitsDuration = 1,

    .freeCallSeconds = 300,
    .freeSms = 3,

    .freeNumbers = {"+420999888777", "+420666555444", "+420333222111"},
  };

  std::vector<Record> records = {
    {"+420123456789", 26, RecordType::Voice, true},
    {"+420123789789", 0, RecordType::Sms, false},
    {"+420321456789", 0, RecordType::Sms, true},
    {"+420987123654", 126, RecordType::Voice, false},
    {"+420999888777", 214, RecordType::Voice, false}, // Free
    {"+420321456789", 0, RecordType::Sms, true},
    {"+420123456789", 54, RecordType::Voice, true},
    {"+420666555444", 0, RecordType::Sms, true}, // Free
    {"+420321456789", 0, RecordType::Sms, false},
    {"+420987123654", 97, RecordType::Voice, false},
    {"+420123456789", 26, RecordType::Voice, true},
    {"+420123789789", 0, RecordType::Sms, false},
    {"+420321456789", 0, RecordType::Sms, true},
    {"+420987123654", 64, RecordType::Voice, false},
    {"+420999888777", 45, RecordType::Voice, true}, // Free
    {"+420321456789", 0, RecordType::Sms, true},
    {"+420123456789", 34, RecordType::Voice, true},
    {"+420666555444", 0, RecordType::Sms, true}, // Free
    {"+420321456789", 0, RecordType::Sms, false},
    {"+420987123654", 97, RecordType::Voice, false},
    {"+420987123654", 10, RecordType::Voice, true},
  };

  BillManager manager(tariff);
  for(auto record : records) {
    manager.processRecord(record);
  }

  Bill bill = manager.getBill();

  REQUIRE(bill.callSecondsInNetwork == 223);
  REQUIRE(bill.callInNetworkCharged == Approx(2.35f));
  REQUIRE(bill.callSecondsOutNetwork == 598);
  REQUIRE(bill.callOutNetworkCharged == Approx(7.00f));

  REQUIRE(bill.smsInNetwork == 6);
  REQUIRE(bill.smsInNetworkCharged == Approx(2.50f));
  REQUIRE(bill.smsOutNetwork == 4);
  REQUIRE(bill.smsOutNetworkCharged == Approx(6.75f));
  REQUIRE(bill.totalFee == Approx(314.15 + 2.35 + 7.00 + 2.50 + 6.75));
}

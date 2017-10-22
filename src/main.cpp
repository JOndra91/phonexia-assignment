#include <set>
#include <string>
#include <iostream>

#ifndef CSV_IO_NO_THREAD
#error Threaded csv readed enabled
#endif

#include <args/args.hxx>
#include <fast-cpp-csv-parser/csv.h>

#include "bill.hpp"
#include "billManager.hpp"
#include "record.hpp"
#include "tariff.hpp"

using namespace std;
using namespace billing;

inline RecordType parseType(string &type) {
  switch (type[0]) {
    case 'V':
      return RecordType::Voice;
      break;
    case 'S':
      return RecordType::Sms;
      break;
    default:
      throw type + ": Unknown record type";
  }
}

inline bool isInNetwork(string &type) {
  switch (type[1]) {
    case 'S':
      return true;
      break;
    case 'M':
      return false;
      break;
    default:
      throw type + ": Unknown record type";
  }
}

int main(int argc, char **argv) {
  Tariff tariff = {
    .callInNetworkFee = 1.5 / 60.0,
    .callOutNetworkFee = 3.5 / 60.0,
    .smsInNetworkFee = 1.0,
    .smsOutNetworkFee = 2.0,
    .monthlyFee = 900,
    .firstUnitDuration = 60,
    .otherUnitsDuration = 1,

    .freeCallSeconds = 100 * 60,
    .freeSms = 10,

    .freeNumbers = {"+420732563345", "+420707325673"},
  };
  BillManager manager(tariff);

  args::ArgumentParser argParser("Billing - Phonexia hiring assignment");
  args::HelpFlag help(argParser, "help", "Show this help", {'h', "help"});
  args::Positional<string> pathArg(argParser, "FILE", "Input file", args::Options::Required);

  try {
    argParser.ParseCLI(argc, argv);
  }
  catch(args::Help) {
    cout << argParser;
    return 0;
  }
  catch (args::ParseError e) {
      std::cerr << e.what() << std::endl;
      std::cerr << argParser;
      return 1;
  }
  catch (args::ValidationError e) {
      std::cerr << e.what() << std::endl;
      std::cerr << argParser;
      return 1;
  }

  int row = 1;
  string path = args::get(pathArg);
  try {
    io::CSVReader<3, io::trim_chars<' ', '\t'>, io::no_quote_escape<';'>> in(path);
    in.read_header(io::ignore_extra_column, "number", "duration", "type");

    string number, duration, type;
    while(in.read_row(number, duration, type)){
      row++;
      // Record parsing is simple enough to not bother with separate module
      // for parsing.
      Record record = {
        .recipient = number,
        .duration = (unsigned)(duration.empty() ? 0 : stoul(duration)),
        .type = parseType(type),
        .inNetwork = isInNetwork(type),
      };

      manager.processRecord(record);
    }
  }
  catch(string &s) {
    cerr << s << " on line " << row << " in file \"" << path << "\"" << endl;
    return 1;
  }
  catch(invalid_argument) {
    cerr << "Duration is not a number on line " << row << " in file \"" << path << "\"" << endl;
    return 1;
  }
  catch(io::error::base &e) {
    cerr << e.what() << endl;
    return 1;
  }

  Bill bill = manager.getBill();

  const char* billFormat =
    "Voice withing network: %7.2f minutes, %7.2f $\n"
    "Voice out of network:  %7.2f minutes, %7.2f $\n"
    "SMS withing network:  %5d.00,         %7.2f $\n"
    "SMS out of network:   %5d.00,         %7.2f $\n"
    "Total:                                  %7.2f $\n"
  ;

  printf(billFormat,
    bill.callSecondsInNetwork / 60.0f, bill.callInNetworkCharged,
    bill.callSecondsOutNetwork / 60.0f, bill.callOutNetworkCharged,
    bill.smsInNetwork, bill.smsInNetworkCharged,
    bill.smsOutNetwork, bill.smsOutNetworkCharged,
    bill.totalFee
  );
}

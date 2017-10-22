#pragma once

#include <string>

namespace billing {

  enum class RecordType {
    Voice, Sms,
  };

  struct Record {
    // Not really needed.
    // std::string datetime;

    std::string recipient;
    unsigned duration;
    RecordType type;
    bool inNetwork;
  };
}

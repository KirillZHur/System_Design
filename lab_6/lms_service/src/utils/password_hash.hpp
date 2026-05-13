#pragma once

#include <openssl/sha.h>

#include <iomanip>
#include <sstream>
#include <string>

namespace lms_service::utils {

inline std::string HashPassword(const std::string& password) {
  unsigned char hash[SHA256_DIGEST_LENGTH];

  SHA256(reinterpret_cast<const unsigned char*>(password.c_str()),
         password.size(),
         hash);

  std::stringstream ss;
  for (unsigned char i : hash) {
    ss << std::hex << std::setw(2) << std::setfill('0') << (int)i;
  }

  return ss.str();
}

}  // namespace lms_service::utils
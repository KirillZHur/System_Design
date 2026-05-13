#pragma once

#include <cstdint>
#include <string>

namespace lms_service::models {

struct User {
  std::int64_t id{0};
  std::string login;
  std::string password;
  std::string first_name;
  std::string last_name;
  std::string role;
};

}  // namespace lms_service::models
#pragma once

#include <cstdint>
#include <string>

namespace lms_service::models {

struct Course {
  std::int64_t id{0};
  std::string title;
  std::string description;
  std::int64_t teacher_id{0};
};

}  // namespace lms_service::models
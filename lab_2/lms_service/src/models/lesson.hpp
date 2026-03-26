#pragma once

#include <cstdint>
#include <string>

namespace lms_service::models {

struct Lesson {
  std::int64_t id{0};
  std::int64_t course_id{0};
  std::string title;
  std::string content;
};

}  // namespace lms_service::models
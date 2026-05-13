#pragma once

#include <cstdint>

namespace lms_service::models {

struct Enrollment {
  std::int64_t user_id{0};
  std::int64_t course_id{0};
};

}  // namespace lms_service::models
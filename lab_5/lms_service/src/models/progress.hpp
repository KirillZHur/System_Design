#pragma once

#include <cstdint>

namespace lms_service::models {

struct Progress {
  std::int64_t user_id{0};
  std::int64_t lesson_id{0};
  bool completed{false};
};

}  // namespace lms_service::models
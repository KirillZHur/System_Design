#pragma once

#include <cstdint>
#include <vector>

#include <userver/storages/mongo/pool.hpp>

#include "../models/course.hpp"
#include "../models/lesson.hpp"

namespace lms_service::storage {

class MongoStorage {
 public:
  explicit MongoStorage(userver::storages::mongo::PoolPtr pool);

  models::Course CreateCourse(const models::Course& course) const;
  std::vector<models::Course> GetAllCourses() const;

  models::Lesson AddLesson(const models::Lesson& lesson) const;
  std::vector<models::Lesson> GetLessonsByCourseId(std::int64_t course_id) const;

 private:
  std::int64_t GetNextCourseId() const;
  std::int64_t GetNextLessonId(std::int64_t course_id) const;

 private:
  userver::storages::mongo::PoolPtr pool_;
};

}  // namespace lms_service::storage
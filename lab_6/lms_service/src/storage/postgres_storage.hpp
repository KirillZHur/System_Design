#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <userver/storages/postgres/cluster.hpp>

#include "../models/course.hpp"
#include "../models/lesson.hpp"
#include "../models/user.hpp"
#include "../models/enrollment.hpp"
#include "../models/progress.hpp"

namespace lms_service::storage {

class PostgresStorage {
 public:
  explicit PostgresStorage(userver::storages::postgres::ClusterPtr pg_cluster);

  models::User CreateUser(const models::User& user) const;
  std::optional<models::User> GetUserByLogin(const std::string& login) const;
  std::vector<models::User> SearchUsers(const std::string& first_name_mask,
                                        const std::string& last_name_mask) const;

  models::Course CreateCourse(const models::Course& course) const;
  std::vector<models::Course> GetAllCourses() const;

  models::Lesson AddLesson(const models::Lesson& lesson) const;
  std::vector<models::Lesson> GetLessonsByCourseId(std::int64_t course_id) const;

  void EnrollUserToCourse(const models::Enrollment& enrollment) const;
  std::vector<models::Course> GetCoursesByUserId(std::int64_t user_id) const;

  void MarkLessonCompleted(const models::Progress& progress) const;

 private:
  userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace lms_service::storage
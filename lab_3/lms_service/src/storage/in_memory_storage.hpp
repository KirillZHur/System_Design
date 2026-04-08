#pragma once

#include <atomic>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <userver/components/component.hpp>
#include <userver/components/component_base.hpp>
#include <userver/yaml_config/merge_schemas.hpp>
#include <userver/yaml_config/schema.hpp>

#include "../models/course.hpp"
#include "../models/enrollment.hpp"
#include "../models/lesson.hpp"
#include "../models/progress.hpp"
#include "../models/user.hpp"

namespace lms_service::storage {

class InMemoryStorage final {
 public:
  std::optional<models::User> CreateUser(const models::User& user);

  std::optional<models::User> GetUserByLogin(const std::string& login) const;

  std::optional<models::User> GetUserById(std::int64_t id) const;

  std::vector<models::User> SearchUsers(
      const std::optional<std::string>& first_name,
      const std::optional<std::string>& last_name) const;

  bool ValidateCredentials(const std::string& login,
                           const std::string& password) const;

  std::optional<models::User> Authenticate(const std::string& login,
                                           const std::string& password) const;

  std::optional<models::Course> CreateCourse(const models::Course& course);
  std::vector<models::Course> GetAllCourses() const;
  std::optional<models::Course> GetCourseById(std::int64_t id) const;

  std::optional<models::Lesson> AddLesson(const models::Lesson& lesson);
  std::vector<models::Lesson> GetLessonsByCourseId(std::int64_t course_id) const;
  std::optional<models::Lesson> GetLessonById(std::int64_t lesson_id) const;

  bool EnrollUser(std::int64_t user_id, std::int64_t course_id);
  std::vector<models::Course> GetCoursesByUserId(std::int64_t user_id) const;
  bool IsUserEnrolled(std::int64_t user_id, std::int64_t course_id) const;

  bool MarkLessonCompleted(std::int64_t user_id, std::int64_t lesson_id);
  std::optional<models::Progress> GetLessonProgress(std::int64_t user_id,
                                                    std::int64_t lesson_id) const;

 private:
  static std::string MakeEnrollmentKey(std::int64_t user_id, std::int64_t course_id);
  static std::string MakeProgressKey(std::int64_t user_id, std::int64_t lesson_id);

 private:
  mutable std::mutex mutex_;

  std::unordered_map<std::int64_t, models::User> users_;
  std::unordered_map<std::string, std::int64_t> login_index_;

  std::unordered_map<std::int64_t, models::Course> courses_;
  std::unordered_map<std::int64_t, std::vector<std::int64_t>> user_courses_;

  std::unordered_map<std::int64_t, std::vector<std::int64_t>> course_lessons_;
  std::unordered_map<std::int64_t, models::Lesson> lessons_;

  std::unordered_set<std::string> enrollments_;
  std::unordered_map<std::string, models::Progress> progress_;

  std::atomic<std::int64_t> next_user_id_{1};
  std::atomic<std::int64_t> next_course_id_{1};
  std::atomic<std::int64_t> next_lesson_id_{1};
};

class InMemoryStorageComponent final : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "in-memory-storage";

  InMemoryStorageComponent(const userver::components::ComponentConfig& config,
                           const userver::components::ComponentContext& context);

  InMemoryStorage& GetStorage();
  const InMemoryStorage& GetStorage() const;

  const std::string& GetJwtSecret() const;

  static userver::yaml_config::Schema GetStaticConfigSchema();

 private:
  InMemoryStorage storage_;
  std::string jwt_secret_;
};

}  // namespace lms_service::storage

template <>
inline constexpr bool userver::components::kHasValidate<
    lms_service::storage::InMemoryStorageComponent> = true;
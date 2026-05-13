#include "in_memory_storage.hpp"

namespace lms_service::storage {

std::string InMemoryStorage::MakeEnrollmentKey(std::int64_t user_id,
                                               std::int64_t course_id) {
  return std::to_string(user_id) + ":" + std::to_string(course_id);
}

std::string InMemoryStorage::MakeProgressKey(std::int64_t user_id,
                                             std::int64_t lesson_id) {
  return std::to_string(user_id) + ":" + std::to_string(lesson_id);
}

std::optional<models::User> InMemoryStorage::CreateUser(const models::User& user) {
  std::lock_guard lock(mutex_);

  if (login_index_.count(user.login)) {
    return std::nullopt;
  }

  models::User new_user = user;
  new_user.id = next_user_id_++;

  login_index_[new_user.login] = new_user.id;
  users_[new_user.id] = new_user;

  return new_user;
}

std::optional<models::User> InMemoryStorage::GetUserByLogin(
    const std::string& login) const {
  std::lock_guard lock(mutex_);

  const auto it = login_index_.find(login);
  if (it == login_index_.end()) {
    return std::nullopt;
  }

  const auto user_it = users_.find(it->second);
  if (user_it == users_.end()) {
    return std::nullopt;
  }

  return user_it->second;
}

std::optional<models::User> InMemoryStorage::GetUserById(std::int64_t id) const {
  std::lock_guard lock(mutex_);

  const auto it = users_.find(id);
  if (it == users_.end()) {
    return std::nullopt;
  }

  return it->second;
}

std::vector<models::User> InMemoryStorage::SearchUsers(
    const std::optional<std::string>& first_name,
    const std::optional<std::string>& last_name) const {
  std::lock_guard lock(mutex_);

  std::vector<models::User> result;

  for (const auto& [id, user] : users_) {
    bool matches = true;

    if (first_name && user.first_name != *first_name) {
      matches = false;
    }

    if (last_name && user.last_name != *last_name) {
      matches = false;
    }

    if (matches) {
      result.push_back(user);
    }
  }

  return result;
}

bool InMemoryStorage::ValidateCredentials(const std::string& login,
                                          const std::string& password) const {
  return Authenticate(login, password).has_value();
}

std::optional<models::User> InMemoryStorage::Authenticate(
    const std::string& login, const std::string& password) const {
  std::lock_guard lock(mutex_);

  const auto it = login_index_.find(login);
  if (it == login_index_.end()) {
    return std::nullopt;
  }

  const auto user_it = users_.find(it->second);
  if (user_it == users_.end()) {
    return std::nullopt;
  }

  if (user_it->second.password != password) {
    return std::nullopt;
  }

  return user_it->second;
}

std::optional<models::Course> InMemoryStorage::CreateCourse(
    const models::Course& course) {
  std::lock_guard lock(mutex_);

  models::Course new_course = course;
  new_course.id = next_course_id_++;
  courses_[new_course.id] = new_course;

  return new_course;
}

std::vector<models::Course> InMemoryStorage::GetAllCourses() const {
  std::lock_guard lock(mutex_);

  std::vector<models::Course> result;
  result.reserve(courses_.size());

  for (const auto& [id, course] : courses_) {
    result.push_back(course);
  }

  return result;
}

std::optional<models::Course> InMemoryStorage::GetCourseById(std::int64_t id) const {
  std::lock_guard lock(mutex_);

  const auto it = courses_.find(id);
  if (it == courses_.end()) {
    return std::nullopt;
  }

  return it->second;
}

std::optional<models::Lesson> InMemoryStorage::AddLesson(
    const models::Lesson& lesson) {
  std::lock_guard lock(mutex_);

  if (!courses_.count(lesson.course_id)) {
    return std::nullopt;
  }

  models::Lesson new_lesson = lesson;
  new_lesson.id = next_lesson_id_++;

  lessons_[new_lesson.id] = new_lesson;
  course_lessons_[new_lesson.course_id].push_back(new_lesson.id);

  return new_lesson;
}

std::vector<models::Lesson> InMemoryStorage::GetLessonsByCourseId(
    std::int64_t course_id) const {
  std::lock_guard lock(mutex_);

  std::vector<models::Lesson> result;

  const auto ids_it = course_lessons_.find(course_id);
  if (ids_it == course_lessons_.end()) {
    return result;
  }

  for (const auto lesson_id : ids_it->second) {
    const auto lesson_it = lessons_.find(lesson_id);
    if (lesson_it != lessons_.end()) {
      result.push_back(lesson_it->second);
    }
  }

  return result;
}

std::optional<models::Lesson> InMemoryStorage::GetLessonById(
    std::int64_t lesson_id) const {
  std::lock_guard lock(mutex_);

  const auto it = lessons_.find(lesson_id);
  if (it == lessons_.end()) {
    return std::nullopt;
  }

  return it->second;
}

bool InMemoryStorage::EnrollUser(std::int64_t user_id, std::int64_t course_id) {
  std::lock_guard lock(mutex_);

  if (!users_.count(user_id) || !courses_.count(course_id)) {
    return false;
  }

  const auto key = MakeEnrollmentKey(user_id, course_id);
  if (enrollments_.count(key)) {
    return false;
  }

  enrollments_.insert(key);
  user_courses_[user_id].push_back(course_id);

  return true;
}

std::vector<models::Course> InMemoryStorage::GetCoursesByUserId(
    std::int64_t user_id) const {
  std::lock_guard lock(mutex_);

  std::vector<models::Course> result;

  const auto ids_it = user_courses_.find(user_id);
  if (ids_it == user_courses_.end()) {
    return result;
  }

  for (const auto course_id : ids_it->second) {
    const auto course_it = courses_.find(course_id);
    if (course_it != courses_.end()) {
      result.push_back(course_it->second);
    }
  }

  return result;
}

bool InMemoryStorage::IsUserEnrolled(std::int64_t user_id,
                                     std::int64_t course_id) const {
  std::lock_guard lock(mutex_);
  return enrollments_.count(MakeEnrollmentKey(user_id, course_id)) > 0;
}

bool InMemoryStorage::MarkLessonCompleted(std::int64_t user_id,
                                          std::int64_t lesson_id) {
  std::lock_guard lock(mutex_);

  const auto lesson_it = lessons_.find(lesson_id);
  if (lesson_it == lessons_.end()) {
    return false;
  }

  const auto course_id = lesson_it->second.course_id;
  if (!users_.count(user_id)) {
    return false;
  }

  if (!enrollments_.count(MakeEnrollmentKey(user_id, course_id))) {
    return false;
  }

  const auto key = MakeProgressKey(user_id, lesson_id);
  progress_[key] = models::Progress{
      .user_id = user_id,
      .lesson_id = lesson_id,
      .completed = true,
  };

  return true;
}

std::optional<models::Progress> InMemoryStorage::GetLessonProgress(
    std::int64_t user_id, std::int64_t lesson_id) const {
  std::lock_guard lock(mutex_);

  const auto key = MakeProgressKey(user_id, lesson_id);
  const auto it = progress_.find(key);
  if (it == progress_.end()) {
    return std::nullopt;
  }

  return it->second;
}

InMemoryStorageComponent::InMemoryStorageComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::components::ComponentBase(config, context),
      jwt_secret_(config["jwt_secret"].As<std::string>("lms-service-secret")) {}

InMemoryStorage& InMemoryStorageComponent::GetStorage() {
  return storage_;
}

const InMemoryStorage& InMemoryStorageComponent::GetStorage() const {
  return storage_;
}

const std::string& InMemoryStorageComponent::GetJwtSecret() const {
  return jwt_secret_;
}

userver::yaml_config::Schema InMemoryStorageComponent::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<userver::components::ComponentBase>(R"(
type: object
description: In-memory storage component
additionalProperties: false
properties:
  jwt_secret:
    type: string
    description: Secret key for JWT signing and verification
)");
}

}  // namespace lms_service::storage
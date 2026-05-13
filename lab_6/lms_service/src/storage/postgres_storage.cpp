#include "postgres_storage.hpp"

#include <userver/storages/postgres/cluster_types.hpp>

namespace lms_service::storage {

PostgresStorage::PostgresStorage(userver::storages::postgres::ClusterPtr pg_cluster)
    : pg_cluster_(std::move(pg_cluster)) {}

std::optional<models::User> PostgresStorage::GetUserByLogin(
    const std::string& login) const {
  const auto result = pg_cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      R"(
        SELECT id, login, password_hash, first_name, last_name, role
        FROM users
        WHERE login = $1
      )",
      login);

  if (result.IsEmpty()) {
    return std::nullopt;
  }

  const auto row = result.Front();

  return models::User{
      .id = row["id"].As<std::int64_t>(),
      .login = row["login"].As<std::string>(),
      .password = row["password_hash"].As<std::string>(),
      .first_name = row["first_name"].As<std::string>(),
      .last_name = row["last_name"].As<std::string>(),
      .role = row["role"].As<std::string>(),
  };
}

std::vector<models::User> PostgresStorage::SearchUsers(
    const std::string& first_name_mask,
    const std::string& last_name_mask) const {
  const auto result = pg_cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      R"(
        SELECT id, login, password_hash, first_name, last_name, role
        FROM users
        WHERE first_name ILIKE $1
          AND last_name ILIKE $2
        ORDER BY id
      )",
      first_name_mask + "%",
      last_name_mask + "%");

  std::vector<models::User> users;
  users.reserve(result.Size());

  for (const auto& row : result) {
    users.push_back(models::User{
        .id = row["id"].As<std::int64_t>(),
        .login = row["login"].As<std::string>(),
        .password = row["password_hash"].As<std::string>(),
        .first_name = row["first_name"].As<std::string>(),
        .last_name = row["last_name"].As<std::string>(),
        .role = row["role"].As<std::string>(),
    });
  }

  return users;
}

models::User PostgresStorage::CreateUser(const models::User& user) const {
  const auto result = pg_cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      R"(
        INSERT INTO users (login, password_hash, first_name, last_name, role)
        VALUES ($1, $2, $3, $4, $5)
        RETURNING id, login, password_hash, first_name, last_name, role
      )",
      user.login,
      user.password,
      user.first_name,
      user.last_name,
      user.role);

  const auto row = result.Front();

  return models::User{
      .id = row["id"].As<std::int64_t>(),
      .login = row["login"].As<std::string>(),
      .password = row["password_hash"].As<std::string>(),
      .first_name = row["first_name"].As<std::string>(),
      .last_name = row["last_name"].As<std::string>(),
      .role = row["role"].As<std::string>(),
  };
}

models::Course PostgresStorage::CreateCourse(const models::Course& course) const {
  const auto result = pg_cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      R"(
        INSERT INTO courses (title, description, teacher_id)
        VALUES ($1, $2, $3)
        RETURNING id, title, description, teacher_id
      )",
      course.title,
      course.description,
      course.teacher_id);

  const auto row = result.Front();

  return models::Course{
      .id = row["id"].As<std::int64_t>(),
      .title = row["title"].As<std::string>(),
      .description = row["description"].As<std::string>(),
      .teacher_id = row["teacher_id"].As<std::int64_t>(),
  };
}

std::vector<models::Course> PostgresStorage::GetAllCourses() const {
  const auto result = pg_cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      R"(
        SELECT id, title, description, teacher_id
        FROM courses
        ORDER BY id
      )");

  std::vector<models::Course> courses;
  courses.reserve(result.Size());

  for (const auto& row : result) {
    courses.push_back(models::Course{
        .id = row["id"].As<std::int64_t>(),
        .title = row["title"].As<std::string>(),
        .description = row["description"].As<std::string>(),
        .teacher_id = row["teacher_id"].As<std::int64_t>(),
    });
  }

  return courses;
}

models::Lesson PostgresStorage::AddLesson(const models::Lesson& lesson) const {
  const auto result = pg_cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      R"(
        INSERT INTO lessons (course_id, title, content, position)
        VALUES (
          $1,
          $2,
          $3,
          CASE
            WHEN $4 > 0 THEN $4
            ELSE (
              SELECT COALESCE(MAX(position), 0) + 1
              FROM lessons
              WHERE course_id = $1
            )
          END
        )
        RETURNING id, course_id, title, content, position
      )",
      lesson.course_id,
      lesson.title,
      lesson.content,
      lesson.position);

  const auto row = result.Front();

  return models::Lesson{
      .id = row["id"].As<std::int64_t>(),
      .course_id = row["course_id"].As<std::int64_t>(),
      .title = row["title"].As<std::string>(),
      .content = row["content"].As<std::string>(),
      .position = row["position"].As<std::int32_t>(),
  };
}

std::vector<models::Lesson> PostgresStorage::GetLessonsByCourseId(
    std::int64_t course_id) const {
  const auto result = pg_cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      R"(
        SELECT id, course_id, title, content, position
        FROM lessons
        WHERE course_id = $1
        ORDER BY position
      )",
      course_id);

  std::vector<models::Lesson> lessons;
  lessons.reserve(result.Size());

  for (const auto& row : result) {
    lessons.push_back(models::Lesson{
        .id = row["id"].As<std::int64_t>(),
        .course_id = row["course_id"].As<std::int64_t>(),
        .title = row["title"].As<std::string>(),
        .content = row["content"].As<std::string>(),
        .position = row["position"].As<std::int32_t>(),
    });
  }

  return lessons;
}

void PostgresStorage::EnrollUserToCourse(
    const models::Enrollment& enrollment) const {
  pg_cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      R"(
        INSERT INTO course_enrollments (user_id, course_id)
        VALUES ($1, $2)
        ON CONFLICT (user_id, course_id) DO NOTHING
      )",
      enrollment.user_id,
      enrollment.course_id);
}

std::vector<models::Course> PostgresStorage::GetCoursesByUserId(
    std::int64_t user_id) const {
  const auto result = pg_cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      R"(
        SELECT c.id, c.title, c.description, c.teacher_id
        FROM course_enrollments ce
        JOIN courses c ON c.id = ce.course_id
        WHERE ce.user_id = $1
        ORDER BY c.id
      )",
      user_id);

  std::vector<models::Course> courses;
  courses.reserve(result.Size());

  for (const auto& row : result) {
    courses.push_back(models::Course{
        .id = row["id"].As<std::int64_t>(),
        .title = row["title"].As<std::string>(),
        .description = row["description"].As<std::string>(),
        .teacher_id = row["teacher_id"].As<std::int64_t>(),
    });
  }

  return courses;
}

void PostgresStorage::MarkLessonCompleted(
    const models::Progress& progress) const {
  pg_cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      R"(
        INSERT INTO lesson_progress (user_id, lesson_id, is_completed, completed_at)
        VALUES ($1, $2, $3, CASE WHEN $3 THEN NOW() ELSE NULL END)
        ON CONFLICT (user_id, lesson_id)
        DO UPDATE SET
            is_completed = EXCLUDED.is_completed,
            completed_at = EXCLUDED.completed_at
      )",
      progress.user_id,
      progress.lesson_id,
      progress.completed);
}

}  // namespace lms_service::storage
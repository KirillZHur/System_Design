#include "mongo_storage.hpp"

#include <algorithm>
#include <chrono>
#include <stdexcept>

#include <userver/formats/bson.hpp>
#include <userver/formats/bson/inline.hpp>

namespace lms_service::storage {

namespace bson = userver::formats::bson;

MongoStorage::MongoStorage(userver::storages::mongo::PoolPtr pool)
    : pool_(std::move(pool)) {}

std::int64_t MongoStorage::GetNextCourseId() const {
  auto collection = pool_->GetCollection("courses");
  auto cursor = collection.Find({});
  std::int64_t max_id = 0;

  for (const auto& doc : cursor) {
    max_id = std::max<std::int64_t>(max_id, doc["_id"].As<std::int64_t>());
  }

  return max_id + 1;
}

std::int64_t MongoStorage::GetNextLessonId(std::int64_t course_id) const {
  auto collection = pool_->GetCollection("courses");
  const auto doc_opt = collection.FindOne(bson::MakeDoc("_id", static_cast<std::int64_t>(course_id)));

  if (!doc_opt) {
    throw std::runtime_error("Course not found");
  }

  std::int64_t max_id = 0;
  const auto lessons = (*doc_opt)["lessons"];

  for (const auto& lesson : lessons) {
    max_id = std::max<std::int64_t>(max_id, lesson["id"].As<std::int64_t>());
  }

  return max_id + 1;
}

models::Course MongoStorage::CreateCourse(const models::Course& course) const {
  auto collection = pool_->GetCollection("courses");
  const auto id = GetNextCourseId();

  collection.InsertOne(bson::MakeDoc(
    "_id", static_cast<std::int64_t>(id),
    "title", course.title,
    "description", course.description,
    "teacher_id", static_cast<std::int64_t>(course.teacher_id),
    "created_at", std::chrono::system_clock::now(),
    "lessons", bson::MakeArray()
));

  return {
      .id = id,
      .title = course.title,
      .description = course.description,
      .teacher_id = course.teacher_id,
  };
}

std::vector<models::Course> MongoStorage::GetAllCourses() const {
  auto collection = pool_->GetCollection("courses");
  auto cursor = collection.Find({});

  std::vector<models::Course> result;

  for (const auto& doc : cursor) {
    result.push_back({
        .id = doc["_id"].As<std::int64_t>(),
        .title = doc["title"].As<std::string>(),
        .description = doc["description"].As<std::string>(""),
        .teacher_id = doc["teacher_id"].As<std::int64_t>(),
    });
  }

  return result;
}

models::Lesson MongoStorage::AddLesson(const models::Lesson& lesson) const {
  auto collection = pool_->GetCollection("courses");
  const auto next_id = GetNextLessonId(lesson.course_id);

  collection.UpdateOne(
      bson::MakeDoc("_id", static_cast<std::int64_t>(lesson.course_id)),
      bson::MakeDoc(
          "$push",
          bson::MakeDoc(
              "lessons",
              bson::MakeDoc(
                  "id", static_cast<std::int64_t>(next_id),
                  "title", lesson.title,
                  "content", lesson.content,
                  "position", static_cast<std::int64_t>(lesson.position),
                  "created_at",  std::chrono::system_clock::now()
              )
          )
      )
  );

  return {
      .id = next_id,
      .course_id = lesson.course_id,
      .title = lesson.title,
      .content = lesson.content,
      .position = lesson.position,
  };
}

std::vector<models::Lesson> MongoStorage::GetLessonsByCourseId(std::int64_t course_id) const {
  auto collection = pool_->GetCollection("courses");
  const auto doc_opt = collection.FindOne(bson::MakeDoc("_id", static_cast<std::int64_t>(course_id)));

  if (!doc_opt) {
    return {};
  }

  std::vector<models::Lesson> result;
  const auto lessons = (*doc_opt)["lessons"];

  for (const auto& lesson : lessons) {
    result.push_back({
        .id = lesson["id"].As<std::int64_t>(),
        .course_id = course_id,
        .title = lesson["title"].As<std::string>(),
        .content = lesson["content"].As<std::string>(""),
        .position = lesson["position"].As<std::int32_t>(),
    });
  }

  std::sort(result.begin(), result.end(),
            [](const models::Lesson& lhs, const models::Lesson& rhs) {
              return lhs.position < rhs.position;
            });

  return result;
}

}  // namespace lms_service::storage
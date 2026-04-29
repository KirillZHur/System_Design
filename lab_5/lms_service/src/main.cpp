#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/auth/auth_checker_factory.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/storages/redis/component.hpp>
#include <userver/storages/secdist/component.hpp>
#include <userver/storages/secdist/provider_component.hpp>

#include "auth/jwt_auth_factory.hpp"
#include "storage/in_memory_storage.hpp"
#include "views/auth/login_handler.hpp"
#include "views/auth/register_handler.hpp"
#include "views/users/get_user_by_login_handler.hpp"
#include "views/users/search_users_handler.hpp"
#include "views/courses/create_course_handler.hpp"
#include "views/courses/list_courses_handler.hpp"
#include "views/lessons/add_lesson_handler.hpp"
#include "views/lessons/list_lessons_handler.hpp"
#include "views/enrollments/enroll_user_handler.hpp"
#include "views/enrollments/get_user_courses_handler.hpp"
#include "views/progress/mark_lesson_completed_handler.hpp"
#include "components/db_cleaner.hpp"


int main(int argc, char* argv[]) {
  userver::server::handlers::auth::RegisterAuthCheckerFactory<
      lms_service::auth::JwtAuthCheckerFactory>();

  auto component_list =
      userver::components::MinimalServerComponentList()
          .Append<userver::server::handlers::Ping>()
          .Append<userver::components::TestsuiteSupport>()
          .AppendComponentList(userver::clients::http::ComponentList())
          .Append<userver::clients::dns::Component>()
          .Append<userver::server::handlers::TestsControl>()
          .Append<lms_service::storage::InMemoryStorageComponent>()
          .Append<lms_service::views::auth::RegisterHandler>()
          .Append<lms_service::views::auth::LoginHandler>()
          .Append<lms_service::views::users::GetUserByLoginHandler>()
          .Append<lms_service::views::users::SearchUsersHandler>()
          .Append<lms_service::views::courses::CreateCourseHandler>()
          .Append<lms_service::views::courses::ListCoursesHandler>()
          .Append<lms_service::views::lessons::AddLessonHandler>()
          .Append<lms_service::views::lessons::ListLessonsHandler>()
          .Append<lms_service::views::enrollments::EnrollUserHandler>()
          .Append<lms_service::views::enrollments::GetUserCoursesHandler>()
          .Append<lms_service::views::progress::MarkLessonCompletedHandler>()
          .Append<userver::components::Postgres>("postgres-db")
          .Append<userver::components::Mongo>("mongo-db")
          .Append<lms_service::components::DbCleaner>()
          .Append<userver::components::Redis>()
          .Append<userver::components::Secdist>()
          .Append<userver::components::DefaultSecdistProvider>();
          


  return userver::utils::DaemonMain(argc, argv, component_list);
}
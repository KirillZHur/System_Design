#include "search_users_handler.hpp"

#include <string>

#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/component.hpp>

#include "../../dto/user_dto.hpp"
#include "../../storage/postgres_storage.hpp"

namespace lms_service::views::users {

SearchUsersHandler::SearchUsersHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      storage_(context.FindComponent<userver::components::Postgres>("postgres-db")
                   .GetCluster()) {}

userver::formats::json::Value SearchUsersHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {
  const auto first_name =
      request.GetArg("first_name").empty() ? "" : request.GetArg("first_name");
  const auto last_name =
      request.GetArg("last_name").empty() ? "" : request.GetArg("last_name");

  const auto users = storage_.SearchUsers(first_name, last_name);

  userver::formats::json::ValueBuilder result(userver::formats::common::Type::kArray);

  for (const auto& user : users) {
    result.PushBack(
        lms_service::dto::ToJson(lms_service::dto::ToUserResponseDto(user)));
  }

  request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
  return result.ExtractValue();
}

}  // namespace lms_service::views::users
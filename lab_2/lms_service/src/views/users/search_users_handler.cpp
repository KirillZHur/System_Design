#include "search_users_handler.hpp"

#include <optional>
#include <string>

#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>

#include "../../dto/user_dto.hpp"
#include "../../storage/in_memory_storage.hpp"

namespace lms_service::views::users {

namespace {

std::optional<std::string> GetOptionalArg(
    const userver::server::http::HttpRequest& request,
    const std::string& name) {
  const auto value = request.GetArg(name);
  if (value.empty()) return std::nullopt;
  return value;
}

}  // namespace

SearchUsersHandler::SearchUsersHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      storage_(context.FindComponent<lms_service::storage::InMemoryStorageComponent>()
                   .GetStorage()) {}

userver::formats::json::Value SearchUsersHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {
  const auto first_name = GetOptionalArg(request, "first_name");
  const auto last_name = GetOptionalArg(request, "last_name");

  const auto users = storage_.SearchUsers(first_name, last_name);

  userver::formats::json::ValueBuilder response(
      userver::formats::common::Type::kArray);

  for (const auto& user : users) {
    response.PushBack(
        lms_service::dto::ToJson(
            lms_service::dto::ToUserResponseDto(user)));
  }

  request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
  return response.ExtractValue();
}

}  // namespace lms_service::views::users
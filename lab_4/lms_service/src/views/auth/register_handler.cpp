#include "register_handler.hpp"

#include <string>

#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/component.hpp>

#include "../../dto/user_dto.hpp"
#include "../../models/user.hpp"
#include "../../storage/postgres_storage.hpp"
#include "../../utils/password_hash.hpp"

namespace lms_service::views::auth {

namespace {

userver::formats::json::Value MakeErrorResponse(const std::string& message) {
  userver::formats::json::ValueBuilder builder;
  builder["error"] = message;
  return builder.ExtractValue();
}

}  // namespace

RegisterHandler::RegisterHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      storage_(context.FindComponent<userver::components::Postgres>("postgres-db")
                   .GetCluster()) {}

userver::formats::json::Value RegisterHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext&) const {
  const auto dto = lms_service::dto::ParseRegisterUserRequest(request_json);

  if (dto.login.empty() || dto.password.empty() || dto.first_name.empty() ||
      dto.last_name.empty() || dto.role.empty()) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
    return MakeErrorResponse(
        "Fields login, password, first_name, last_name, role are required");
  }

  if (storage_.GetUserByLogin(dto.login)) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kConflict);
    return MakeErrorResponse("User with this login already exists");
  }

  const auto hashed_password =
    lms_service::utils::HashPassword(dto.password);

  const auto created = storage_.CreateUser({
    .id = 0,
    .login = dto.login,
    .password = hashed_password,
    .first_name = dto.first_name,
    .last_name = dto.last_name,
    .role = dto.role,
  });

  auto response =
      userver::formats::json::ValueBuilder(
          lms_service::dto::ToJson(lms_service::dto::ToUserResponseDto(created)));
  response["status"] = "registered";

  request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
  return response.ExtractValue();
}

}  // namespace lms_service::views::auth
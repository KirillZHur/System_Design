#include "get_user_by_login_handler.hpp"

#include <string>

#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>

#include "../../dto/user_dto.hpp"
#include "../../storage/in_memory_storage.hpp"

namespace lms_service::views::users {

namespace {

userver::formats::json::Value MakeErrorResponse(const std::string& message) {
  userver::formats::json::ValueBuilder builder;
  builder["error"] = message;
  return builder.ExtractValue();
}

}  // namespace

GetUserByLoginHandler::GetUserByLoginHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      storage_(context.FindComponent<lms_service::storage::InMemoryStorageComponent>()
                   .GetStorage()) {}

userver::formats::json::Value GetUserByLoginHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {
  const auto& login = request.GetPathArg("login");

  if (login.empty()) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
    return MakeErrorResponse("login is required");
  }

  const auto user = storage_.GetUserByLogin(login);
  if (!user) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
    return MakeErrorResponse("User not found");
  }

  request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
  return lms_service::dto::ToJson(
      lms_service::dto::ToUserResponseDto(*user));
}

}  // namespace lms_service::views::users
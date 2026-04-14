#include "login_handler.hpp"

#include <chrono>
#include <string>

#include <jwt-cpp/jwt.h>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/component.hpp>

#include "../../dto/auth_dto.hpp"
#include "../../storage/in_memory_storage.hpp"
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

LoginHandler::LoginHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      storage_(context.FindComponent<userver::components::Postgres>("postgres-db")
                   .GetCluster()),
      jwt_secret_(context
                      .FindComponent<lms_service::storage::InMemoryStorageComponent>()
                      .GetJwtSecret()) {}

userver::formats::json::Value LoginHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext&) const {
  const auto dto = lms_service::dto::ParseLoginRequest(request_json);

  if (dto.login.empty() || dto.password.empty()) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
    return MakeErrorResponse("Fields login and password are required");
  }

  const auto user = storage_.GetUserByLogin(dto.login);

  const auto hashed_input =
    lms_service::utils::HashPassword(dto.password);

  if (!user || user->password != hashed_input) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
    return MakeErrorResponse("Invalid login or password");
  }

  const auto now = std::chrono::system_clock::now();
  const auto token = jwt::create()
                         .set_issuer("lms_service")
                         .set_type("JWS")
                         .set_subject(user->login)
                         .set_payload_claim("role", jwt::claim(user->role))
                         .set_issued_at(now)
                         .set_expires_at(now + std::chrono::hours{24})
                         .sign(jwt::algorithm::hs256{jwt_secret_});

  const lms_service::dto::LoginResponseDto response_dto{
      .token = token,
      .token_type = "Bearer",
      .login = user->login,
      .role = user->role,
  };

  request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
  return lms_service::dto::ToJson(response_dto);
}

}  // namespace lms_service::views::auth
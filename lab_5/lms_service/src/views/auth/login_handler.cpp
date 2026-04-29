#include "login_handler.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <string>

#include <jwt-cpp/jwt.h>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/redis/component.hpp>

#include "../../dto/auth_dto.hpp"
#include "../../storage/in_memory_storage.hpp"
#include "../../storage/postgres_storage.hpp"
#include "../../utils/password_hash.hpp"

namespace lms_service::views::auth {

namespace {

constexpr int kLoginRateLimit = 10;
constexpr int kRateLimitWindowSeconds = 60;

userver::formats::json::Value MakeErrorResponse(const std::string& message) {
  userver::formats::json::ValueBuilder builder;
  builder["error"] = message;
  return builder.ExtractValue();
}

std::string MakeRateLimitKey(const std::string& client_id) {
  const auto now = std::time(nullptr);
  const auto window = now / kRateLimitWindowSeconds;
  return "rate_limit:login:" + client_id + ":" + std::to_string(window);
}

int GetSecondsUntilWindowReset() {
  const auto now = std::time(nullptr);
  return kRateLimitWindowSeconds -
         static_cast<int>(now % kRateLimitWindowSeconds);
}

std::string GetClientId(const userver::server::http::HttpRequest& request) {
  const auto forwarded_for = request.GetHeader("X-Forwarded-For");
  if (!forwarded_for.empty()) {
    return forwarded_for;
  }

  return "local";
}

void SetRateLimitHeaders(const userver::server::http::HttpRequest& request,
                         int remaining,
                         int reset_seconds) {
  request.GetHttpResponse().SetHeader(
      std::string{"X-RateLimit-Limit"},
      std::to_string(kLoginRateLimit));

  request.GetHttpResponse().SetHeader(
      std::string{"X-RateLimit-Remaining"},
      std::to_string(std::max(remaining, 0)));

  request.GetHttpResponse().SetHeader(
      std::string{"X-RateLimit-Reset"},
      std::to_string(reset_seconds));
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
                      .GetJwtSecret()),
      redis_client_(context.FindComponent<userver::components::Redis>("redis")
                        .GetClient("cache-db")),
      redis_cc_(std::chrono::seconds{1}, std::chrono::seconds{3}, 2) {}

userver::formats::json::Value LoginHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext&) const {
  const auto client_id = GetClientId(request);
  const auto rate_limit_key = MakeRateLimitKey(client_id);
  const auto reset_seconds = GetSecondsUntilWindowReset();

  int current_count = 0;

  const auto cached_count = redis_client_->Get(rate_limit_key, redis_cc_).Get();
  if (cached_count) {
    current_count = std::stoi(*cached_count);
  }

  if (current_count >= kLoginRateLimit) {
    SetRateLimitHeaders(request, 0, reset_seconds);
    request.SetResponseStatus(
        userver::server::http::HttpStatus::kTooManyRequests);
    return MakeErrorResponse("Too many login requests");
  }

  const auto new_count = current_count + 1;
  const auto remaining = kLoginRateLimit - new_count;

  redis_client_
      ->Set(rate_limit_key, std::to_string(new_count),
            std::chrono::seconds{kRateLimitWindowSeconds + 5}, redis_cc_)
      .Get();

  SetRateLimitHeaders(request, remaining, reset_seconds);

  const auto dto = lms_service::dto::ParseLoginRequest(request_json);

  if (dto.login.empty() || dto.password.empty()) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
    return MakeErrorResponse("Fields login and password are required");
  }

  const auto user = storage_.GetUserByLogin(dto.login);

  const auto hashed_input = lms_service::utils::HashPassword(dto.password);

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
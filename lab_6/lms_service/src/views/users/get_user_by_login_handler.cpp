#include "get_user_by_login_handler.hpp"

#include <chrono>
#include <string>

#include <userver/formats/json.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/redis/component.hpp>

#include "../../dto/user_dto.hpp"
#include "../../storage/postgres_storage.hpp"

namespace lms_service::views::users {

namespace {

constexpr auto kUserCacheTtl = std::chrono::minutes{5};

std::string MakeUserCacheKey(const std::string& login) {
  return "user:login:" + login;
}

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
      storage_(context.FindComponent<userver::components::Postgres>("postgres-db")
                   .GetCluster()),
      redis_client_(context.FindComponent<userver::components::Redis>("redis")
                        .GetClient("cache-db")),
      redis_cc_(std::chrono::seconds{1}, std::chrono::seconds{3}, 2) {}

userver::formats::json::Value GetUserByLoginHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {
  const auto& login = request.GetPathArg("login");

  if (login.empty()) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
    return MakeErrorResponse("login is required");
  }

  const auto cache_key = MakeUserCacheKey(login);

  const auto cached = redis_client_->Get(cache_key, redis_cc_).Get();
  if (cached) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    return userver::formats::json::FromString(*cached);
  }

  const auto user = storage_.GetUserByLogin(login);
  if (!user) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
    return MakeErrorResponse("User not found");
  }

  const auto response =
      lms_service::dto::ToJson(lms_service::dto::ToUserResponseDto(*user));

  redis_client_
      ->Set(cache_key, userver::formats::json::ToStableString(response),
            kUserCacheTtl, redis_cc_)
      .Get();

  request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
  return response;
}

}  // namespace lms_service::views::users
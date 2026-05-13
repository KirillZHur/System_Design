#include "search_users_handler.hpp"

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

constexpr auto kSearchCacheTtl = std::chrono::minutes{1};

std::string MakeSearchCacheKey(const std::string& first_name,
                               const std::string& last_name) {
  return "users:search:first_name:" + first_name + ":last_name:" + last_name;
}

}  // namespace

SearchUsersHandler::SearchUsersHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      storage_(context.FindComponent<userver::components::Postgres>("postgres-db")
                   .GetCluster()),
      redis_client_(context.FindComponent<userver::components::Redis>("redis")
                        .GetClient("cache-db")),
      redis_cc_(std::chrono::seconds{1}, std::chrono::seconds{3}, 2) {}

userver::formats::json::Value SearchUsersHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {
  const auto first_name =
      request.GetArg("first_name").empty() ? "" : request.GetArg("first_name");
  const auto last_name =
      request.GetArg("last_name").empty() ? "" : request.GetArg("last_name");

  const auto cache_key = MakeSearchCacheKey(first_name, last_name);

  const auto cached = redis_client_->Get(cache_key, redis_cc_).Get();
  if (cached) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    return userver::formats::json::FromString(*cached);
  }

  const auto users = storage_.SearchUsers(first_name, last_name);

  userver::formats::json::ValueBuilder result(
      userver::formats::common::Type::kArray);

  for (const auto& user : users) {
    result.PushBack(
        lms_service::dto::ToJson(lms_service::dto::ToUserResponseDto(user)));
  }

  auto response = result.ExtractValue();

  redis_client_
      ->Set(cache_key, userver::formats::json::ToStableString(response),
            kSearchCacheTtl, redis_cc_)
      .Get();

  request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
  return response;
}

}  // namespace lms_service::views::users
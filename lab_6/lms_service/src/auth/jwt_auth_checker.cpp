#include "jwt_auth_checker.hpp"

#include <string_view>

#include <jwt-cpp/jwt.h>
#include <userver/http/common_headers.hpp>

namespace lms_service::auth {

namespace {
constexpr std::string_view kBearerPrefix = "Bearer ";
}  // namespace

JwtAuthChecker::JwtAuthChecker(std::string secret) : secret_(std::move(secret)) {}

JwtAuthChecker::AuthCheckResult JwtAuthChecker::CheckAuth(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& /*context*/) const {
  const std::string_view auth_header =
      request.GetHeader(userver::http::headers::kAuthorization);

  if (auth_header.empty()) {
    return {AuthCheckResult::Status::kTokenNotFound,
            "Missing Authorization header"};
  }

  if (!auth_header.starts_with(kBearerPrefix)) {
    return {AuthCheckResult::Status::kInvalidToken,
            "Authorization header must start with Bearer"};
  }

  const std::string token{auth_header.substr(kBearerPrefix.size())};

  try {
    auto decoded = jwt::decode(token);

    auto verifier = jwt::verify()
                        .allow_algorithm(jwt::algorithm::hs256{secret_})
                        .with_issuer("lms_service");

    verifier.verify(decoded);

    return {};
  } catch (const std::exception& e) {
    return {AuthCheckResult::Status::kInvalidToken,
            std::string{"JWT verification failed: "} + e.what()};
  }
}

}  // namespace lms_service::auth
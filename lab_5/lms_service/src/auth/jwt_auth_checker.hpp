#pragma once

#include <memory>
#include <string>

#include <userver/server/handlers/auth/auth_checker_base.hpp>
#include <userver/server/request/request_context.hpp>

namespace lms_service::auth {

class JwtAuthChecker final : public userver::server::handlers::auth::AuthCheckerBase {
 public:
  using AuthCheckResult = userver::server::handlers::auth::AuthCheckResult;

  explicit JwtAuthChecker(std::string secret);

  AuthCheckResult CheckAuth(
      const userver::server::http::HttpRequest& request,
      userver::server::request::RequestContext& context) const override;

  bool SupportsUserAuth() const noexcept override { return true; }

 private:
  std::string secret_;
};

using JwtAuthCheckerPtr = std::shared_ptr<JwtAuthChecker>;

}  // namespace lms_service::auth
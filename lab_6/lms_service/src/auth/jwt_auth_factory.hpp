#pragma once

#include <userver/components/component_context.hpp>
#include <userver/server/handlers/auth/auth_checker_factory.hpp>

#include "../storage/in_memory_storage.hpp"
#include "jwt_auth_checker.hpp"

namespace lms_service::auth {

class JwtAuthCheckerFactory final
    : public userver::server::handlers::auth::AuthCheckerFactoryBase {
 public:
  static constexpr std::string_view kAuthType = "jwt-auth";

  explicit JwtAuthCheckerFactory(
      const userver::components::ComponentContext& context);

  userver::server::handlers::auth::AuthCheckerBasePtr MakeAuthChecker(
      const userver::server::handlers::auth::HandlerAuthConfig&) const override;

 private:
  JwtAuthCheckerPtr checker_;
};

}  // namespace lms_service::auth
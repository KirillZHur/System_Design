#include "jwt_auth_factory.hpp"

namespace lms_service::auth {

JwtAuthCheckerFactory::JwtAuthCheckerFactory(
    const userver::components::ComponentContext& context)
    : checker_(std::make_shared<JwtAuthChecker>(
          context.FindComponent<lms_service::storage::InMemoryStorageComponent>()
              .GetJwtSecret())) {}

userver::server::handlers::auth::AuthCheckerBasePtr
JwtAuthCheckerFactory::MakeAuthChecker(
    const userver::server::handlers::auth::HandlerAuthConfig&) const {
  return checker_;
}

}  // namespace lms_service::auth
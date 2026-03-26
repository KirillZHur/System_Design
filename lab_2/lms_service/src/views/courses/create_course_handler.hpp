#pragma once

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>

namespace lms_service::storage {
class InMemoryStorage;
}

namespace lms_service::views::courses {

class CreateCourseHandler final
    : public userver::server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-create-course";

  CreateCourseHandler(const userver::components::ComponentConfig& config,
                      const userver::components::ComponentContext& context);

  userver::formats::json::Value HandleRequestJsonThrow(
      const userver::server::http::HttpRequest& request,
      const userver::formats::json::Value& request_json,
      userver::server::request::RequestContext& context) const override;

 private:
  lms_service::storage::InMemoryStorage& storage_;
};

}  // namespace lms_service::views::courses
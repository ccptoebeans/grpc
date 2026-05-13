// Copyright © 2025 CCP ehf.
#ifndef METADATA_H
#define METADATA_H

#include <grpcpp/grpcpp.h>

#include <string>

namespace monolith_grpc::client {
class Metadata {
public:

  static void ApplyToContext(::grpc::ClientContext& context);

  [[nodiscard]] static std::string application_instance_uuid();
  static void set_application_instance_uuid(const std::string& uuid);

  [[nodiscard]] static std::string auth_token();
  static void set_auth_token(const std::string& token);

private:

  static std::string application_instance_uuid_;
  static std::string auth_token_;
};
}  // namespace monolith_grpc

#endif

// Copyright © 2025 CCP ehf.
#include "carbongrpc/client/metadata.h"
using namespace monolith_grpc::client;

std::string Metadata::application_instance_uuid_;
std::string Metadata::auth_token_;

// cppcheck-suppress unusedFunction
void Metadata::ApplyToContext(::grpc::ClientContext& context) {
  if (!application_instance_uuid_.empty()) {
    context.AddMetadata("application_instance_uuid", application_instance_uuid_);
  }

  if (!auth_token_.empty()) {
    context.AddMetadata("authorization", auth_token_);
  }
}

std::string Metadata::application_instance_uuid() {
  return application_instance_uuid_;
}

void Metadata::set_application_instance_uuid(const std::string& uuid) {
  application_instance_uuid_ = uuid;
}

std::string Metadata::auth_token() {
  return auth_token_;
}

void Metadata::set_auth_token(const std::string& token) {
  auth_token_ = token;
}

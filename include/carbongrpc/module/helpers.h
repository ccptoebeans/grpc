// Copyright © 2025 CCP ehf.
#pragma once

#include <chrono>
#include <mutex>
#include <string>

#include <google/protobuf/message.h>

#include <Python.h>

#define CCP_CONCATENATE_DIRECT(s1, s2) s1##s2
#define CCP_CONCATENATE(s1, s2) CCP_CONCATENATE_DIRECT(s1, s2)

namespace monolith_grpc::module {

class Helpers final {
public:

  [[nodiscard]] static Helpers& instance();

  Helpers();
  ~Helpers() = default;

  [[nodiscard]] std::string ReadFile(const std::string& filename) const;
  [[nodiscard]] std::string ConvertString(PyObject* str) const;
  [[nodiscard]] std::chrono::system_clock::time_point ToSystemTime(std::chrono::steady_clock::time_point t) const;

  [[nodiscard]] google::protobuf::Message* GetCProtoInsidePyProto(PyObject* msg) const;

  [[nodiscard]] PyObject* CreatePythonObject(const char* module, const char* type);

  // Debugging
#ifdef DEBUG
  void PrintDictionary(PyObject* dict) const;
  void PrintTimestamp(std::chrono::system_clock::time_point timestamp) const;
#endif

private:

  static std::once_flag init_flag_;
  static Helpers* instance_;

  std::unique_ptr<struct HelpersPrivate> private_;
};

}  // namespace monolith_grpc::module

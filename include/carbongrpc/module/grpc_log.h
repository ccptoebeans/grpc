#pragma once

// std
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>

// Python
#include <Python.h>

// native resolver
#include "carbongrpc/client/grpc_log.h"

namespace monolith_grpc {
namespace module {

class GrpcLog {
public:

  static PyObject* set_log_level(PyObject* self, PyObject* args, PyObject* keywords);
  static const char* docstring_set_log_level;
  static PyObject* get_logs(PyObject* self, PyObject* param);
  static const char* docstring_get_logs;

  static void RegisterModuleConstants(PyObject* module);

  /*protected:*/

  static PyMethodDef python_methods[];
};

}  // namespace module
}  // namespace monolith_grpc


#pragma once

#include <Python.h>

#include <string>

#include "carbongrpc/client/connection.h"

namespace monolith_grpc::module {

class Connection {
public:

  typedef struct {
    PyObject_HEAD std::shared_ptr<monolith_grpc::client::Connection> connection;
    PyObject* metric_registry;
  } PythonType;

  static int init(PythonType* self, PyObject* args, PyObject* keywords);
  static void dealloc(PythonType* self);
  static bool is_instance(PyObject* obj);

  static PyObject* connect(PythonType* self, PyObject* args, PyObject* keywords);
  static const char* docstring_connect;
  static PyObject* disconnect(PythonType* self, PyObject* param);
  static const char* docstring_disconnect;

  static PyObject* get_channel_state(PythonType* self, PyObject* param);
  static const char* docstring_get_channel_state;

  static PyObject* set_metric_registry(PythonType* self, PyObject* args, PyObject* keywords);
  static const char* docstring_set_metric_registry;

  /*protected:*/

  static PyMethodDef python_methods[];

  static std::string ReadFile(const std::string& filename);
  static std::string ConvertString(PyObject* str);

  static const char* default_metric_prefix;
};

void RegisterConnection(PyObject* module, const std::string& class_name);

}  // namespace monolith_grpc::module

#ifndef RESOLVE_RESULT_MODULE_H
#define RESOLVE_RESULT_MODULE_H

// std
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>

// Python
#include <Python.h>

// native resolver
#include "carbongrpc/client/resolver.h"

namespace monolith_grpc {
namespace module {

class ResolveResult {
public:

  typedef struct {
    PyObject_HEAD std::unique_ptr<monolith_grpc::client::Resolver::Result> result;
  } PythonType;

  static int init(PythonType* self, PyObject* args, PyObject* keywords);
  static void dealloc(PythonType* self);

  static PyObject* get_status(PythonType* self, PyObject* param);
  static const char* docstring_get_status;
  static PyObject* get_request_name(PythonType* self, PyObject* param);
  static const char* docstring_get_request_name;
  static PyObject* get_error(PythonType* self, PyObject* param);
  static const char* docstring_get_error;

  // hostname results
  static PyObject* get_official_name(PythonType* self, PyObject* param);
  static const char* docstring_get_official_name;
  static PyObject* get_addresses(PythonType* self, PyObject* param);
  static const char* docstring_get_addresses;
  static PyObject* get_aliases(PythonType* self, PyObject* param);
  static const char* docstring_get_aliases;

  // srv results
  static PyObject* get_srv_result(PythonType* self, PyObject* param);
  static const char* docstring_get_srv_result;

  // module-level functions
  // expose these at the module-level since they return a ResolveResult
  static PyObject* set_dns_servers(PythonType* self, PyObject* args, PyObject* keywords);
  static const char* docstring_set_dns_servers;
  static PyObject* query_hostname(PythonType* self, PyObject* args, PyObject* keywords);
  static const char* docstring_query_hostname;
  static PyObject* query_srv(PythonType* self, PyObject* args, PyObject* keywords);
  static const char* docstring_query_srv;

  /*protected:*/

  static PyMethodDef python_methods[];
};

}  // namespace module
}  // namespace monolith_grpc

// resolve_hostname and resolve_srv need to be able to create instances of
// ResolveResult But they cannot call PyObject_CallObject directly because they
// can't see the PyType struct The PyType struct has to be defined in each
// module because it needs to know the fully qualified name So,
// CreateResolveResult needs to be defined in each module.
PyObject* CreateResolveResult();  ///< Must return a new instance of ResolveResult

#endif

// Copyright © 2025 CCP ehf.
#include "grpc_log.h"

namespace monolith_grpc {
namespace module {

const char* GrpcLog::docstring_set_log_level = 
  "set_log_level(log_level: int) -> None"
  "\n"
  "Sets the log level.\n"
  "Valid log levels are defined as eve_grpc_client.enum_grpc_log_*\n"
  "\n"
  ":param log_level: The desired log level. One of eve_grpc_client.enum_grpc_log_*\n"
  ":type log_level: int\n"
  ":return: None\n"
  ":rtype: None\n";
PyObject* GrpcLog::set_log_level(PyObject*, PyObject* args, PyObject* keywords) {
  int arg_log_level;

  static const char* keyword_list[] = {"log_level", nullptr};

  if (!PyArg_ParseTupleAndKeywords(args, keywords, "i", (char**)keyword_list, &arg_log_level)) {
    return nullptr;
  }

  // The value of arg_log_level is not checked here on purpose.
  // gRPC doesn't specify a NONE level, but it does support it.
  monolith_grpc::client::GrpcLog::SetLogLevel((gpr_log_severity)arg_log_level);

  Py_RETURN_NONE;
}

const char* GrpcLog::docstring_get_logs =
  "get_logs() -> List[Tuple(int, string)]\n"
  "Returns all unread grpc logs.\n"
  "Once a log message has been returned by this function, it will not be returned again.\n"
  "This function returns only the 10000 most recent messages. Older messages are dropped.\n"
  "If no unread messages exist, this function returns an empty List.\n"
  "Each element of the returned List is a tuple with 4 elements containing:\n"
  "  0: The integer log level of the associated text, with values corresponding to enum_grpc_log_*\n"
  "  1: The source file the log originated from\n"
  "  2: The line of the source file the log originated from\n"
  "  1: The message string\n"
  "\n"
  ":return: A List of Tuples.  See the function documentation for details of the tuple elements.\n"
  ":rtype: List\n";
PyObject* GrpcLog::get_logs(PyObject*, PyObject*) {
  PyObject* result = PyList_New(0);  // new reference. don't decref or incref since we'll return this to the caller.

  auto logs = monolith_grpc::client::GrpcLog::GetLogEntries();

  for (const auto& log : logs) {
    if (log.message.size() >= ULONG_MAX || log.file.size() >= ULONG_MAX) {
      // Silently discard log messages that we can't return to Python.
      // We could throw here, but that would discard all the other valid log messages in this list
      // The correct thing to do is probably to accumulate errors and return those alongside the messages
      //   but realistically, a log message larger than ULONG_MAX sounds malicious
      continue;
    }
    PyObject* tuple = PyTuple_New(4);                             // new reference
    PyTuple_SetItem(tuple, 0, PyLong_FromLong((long)log.severity));  // new reference gets stolen by PyTuple_SetItem
    PyTuple_SetItem(tuple, 1, PyBytes_FromStringAndSize(log.file.c_str(), (Py_ssize_t)log.file.size()));  // new reference gets stolen by PyTuple_SetItem
    PyTuple_SetItem(tuple, 2, PyLong_FromLong((long)log.line));  // new reference gets stolen by PyTuple_SetItem
    PyTuple_SetItem(tuple, 3, PyBytes_FromStringAndSize(log.message.c_str(), (Py_ssize_t)log.message.size()));  // new reference gets stolen by PyTuple_SetItem
    PyList_Append(result, tuple);

    // PyList_Append borrows a reference to tuple, so we need to drop our reference here
    Py_DecRef(tuple);
  }

  return result;
}

void GrpcLog::RegisterModuleConstants(PyObject* module) {
  PyModule_AddIntConstant(module, "enum_grpc_log_debug", static_cast<long>(GPR_LOG_SEVERITY_DEBUG));
  PyModule_AddIntConstant(module, "enum_grpc_log_info", static_cast<long>(GPR_LOG_SEVERITY_INFO));
  PyModule_AddIntConstant(module, "enum_grpc_log_error", static_cast<long>(GPR_LOG_SEVERITY_ERROR));

  // GPR_LOG_SEVERITY_NONE is not exported, but it's defined internally.
  // Any arbitrarily large number should be fine though as long as it exceeds GPR_LOG_SEVERITY_ERROR.
  // https://github.com/grpc/grpc/blob/adfd009d3a255b825ea91959620c11805418b22b/src/core/lib/gpr/log.cc#L43
  PyModule_AddIntConstant(module, "enum_grpc_log_none", static_cast<long>(GPR_LOG_SEVERITY_ERROR + 11));
}

}  // namespace module
}  // namespace monolith_grpc


// Copyright © 2025 CCP ehf.
// std
#include "carbongrpc/module/connection.h"

#include <fstream>
#include <memory>
#include <sstream>
#include <string>

using NativeConnection = monolith_grpc::client::Connection;

namespace monolith_grpc::module {

int Connection::init(PythonType* self, PyObject*, PyObject*) {
  self->connection = std::make_shared<NativeConnection>();
  self->metric_registry = nullptr;
  return 0;
}

void Connection::dealloc(PythonType* self) {
  self->connection.reset();

  Py_XDECREF(self->metric_registry);

  Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

const char* Connection::docstring_connect =
  "connect(host: str, root_file: str, root_data: str, cert_file: str, "
  "cert_data: str, key_file: str, key_data: str, "
  "initial_reconnect_backoff_millis: int, max_reconnect_backoff_millis: int) "
  "-> bool\n"
  "\n"
  "Connect to a server.  This will connect in one of three ways:\n"
  "1: Over an insecure channel (only provide the host parameter).\n"
  "2: Over a secure channel, but without client authentication (only provide "
  "the host and root parameters).\n"
  "3: Over a secure channel, with client authentication (provide the host, "
  "root, key, and cert parameters).\n"
  "The root, cert, and key data can be specified as either a filename "
  "(root_file, cert_file, key_file) or as strings containing the actual data "
  "(root_data, cert_data, key_data).\n"
  "If both x_file and x_data are specified, x_data takes precedence.\n"
  "\n"
  "If the connection is dropped, reconnect will be done automatically with "
  "an exponential backoff.\n"
  "The minimum and maximum backoff times can be specified via "
  "initial_reconnect_backoff_millis and max_reconnect_backoff_millis.\n"
  "\n"
  ":return: None\n"
  ":rtype: None\n";

PyObject* Connection::connect(PythonType* self, PyObject* args, PyObject* keywords) {
  const char* arg_host = nullptr;
  const char* arg_root_file = nullptr;
  const char* arg_root_data = nullptr;
  const char* arg_cert_file = nullptr;
  const char* arg_cert_data = nullptr;
  const char* arg_key_file = nullptr;
  const char* arg_key_data = nullptr;
  const char* arg_server_name_override = nullptr;
  int arg_initial_backoff_millis = -1;
  int arg_max_backoff_millis = -1;

  static const char* keyword_list[] = {
    "host",
    "root_file",
    "root_data",
    "cert_file",
    "cert_data",
    "key_file",
    "key_data",
    "server_name_override",
    "initial_reconnect_backoff_millis",
    "max_reconnect_backoff_millis",
    nullptr};

  if (!PyArg_ParseTupleAndKeywords(
        args, keywords, "s|sssssssii", (char**)keyword_list, &arg_host, &arg_root_file, &arg_root_data, &arg_cert_file,
        &arg_cert_data, &arg_key_file, &arg_key_data, &arg_server_name_override, &arg_initial_backoff_millis,
        &arg_max_backoff_millis
      )) {
    return nullptr;
  }

  // Read file contents to populate arg_x_data if the data wasn't provided
  // directly

  monolith_grpc::client::Connection::ConnectParams params;
  if (arg_root_data) {
    params.root = arg_root_data;
  } else if (arg_root_file /* && !arg_root_data*/) {
    params.root = ReadFile(arg_root_file);
  }

  if (arg_cert_data) {
    params.cert = arg_cert_data;
  } else if (arg_cert_file /* && !arg_cert_data*/) {
    params.cert = ReadFile(arg_cert_file);
  }

  if (arg_key_data) {
    params.key = arg_key_data;
  } else if (arg_key_file /* && !arg_key_data*/) {
    params.key = ReadFile(arg_key_file);
  }

  if (arg_server_name_override) {
    params.server_name_override = arg_server_name_override;
  }

  params.host = arg_host;
  params.initial_reconnect_backoff_millis = arg_initial_backoff_millis;
  params.max_reconnect_backoff_millis = arg_max_backoff_millis;

  self->connection->Connect(params);

  Py_RETURN_NONE;
}

const char* Connection::docstring_disconnect =
  "disconnect() -> bool\n"
  "\n"
  "Disconnect from the server, closing the gRPC channel\n"
  "\n"
  ":return: None\n"
  ":rtype: None\n";

PyObject* Connection::disconnect(PythonType* self, PyObject* param) {
  self->connection->Disconnect();
  Py_RETURN_NONE;
}

const char* Connection::docstring_get_channel_state =
  "get_channel_state() -> int\n"
  "\n"
  "Returns the gRPC channel state. Internally, this is a "
  "grpc_connectivity_state enum. Values are:\n"
  "-1 = None (gateway client does not exist. call connect first.)\n"
  "0 = Idle (channel is idle)\n"
  "1 = Connecting (channel is connecting)\n"
  "2 = Ready (channel is ready for work)\n"
  "3 = TransientFailure (channel has seen a failure but expects to recover)\n"
  "4 = Shutdown (channel has seen a failure that it cannot recover from\n"
  "\n"
  ":return: An integer representing the gRPC channel state\n"
  ":rtype: int\n";

PyObject* Connection::get_channel_state(PythonType* self, PyObject* param) {
  return PyLong_FromLong(self->connection->channel_state());
}

const char* Connection::docstring_set_metric_registry =
  "set_metric_registry(registry: Object, prefix: str, labels: Dict[str,str]) "
  "-> bool\n"
  "\n"
  "Assign the prometheus_module.MetricRegistry to be used for reporting "
  "metrics.\n"
  "\n"
  ":param capsule: A capsule object returned by "
  "prometheus_module.MetricRegistry.GetCapsule().\n"
  ":type capsule: Object\n"
  ":param prefix: (optional) A string with which to prefix all metrics\n"
  ":type prefix: str\n"
  ":param labels: (optional) A dictionary of string->string mapping "
  "label_keys to label_values\n"
  ":type labels: Dict[str,str]\n"
  ":return: True on success, False on failure\n"
  ":rtype: bool\n";

PyObject* Connection::set_metric_registry(PythonType* self, PyObject* args, PyObject* keywords) {
  PyObject* arg_registry = nullptr;
  PyObject* arg_prefix = nullptr;
  PyObject* arg_labels = nullptr;

  static const char* keyword_list[] = {"registry", "prefix", "labels", "asynchronous", nullptr};

  if (!PyArg_ParseTupleAndKeywords(
        args, keywords, "O|OOO", (char**)keyword_list, &arg_registry, &arg_prefix, &arg_labels
      )) {
    return nullptr;
  }

  // Retrieve the native registry pointer from the capsule
  PyObject* capsule = PyObject_CallMethod(arg_registry, (char*)"GetCapsule", nullptr);
  if (capsule == nullptr) {
    return nullptr;
  }

  auto* metric_registry = (prometheus_module::MetricRegistryInterface*)PyCapsule_GetPointer(capsule, nullptr);

  // Get the prefix
  std::string metric_prefix = ConvertString(arg_prefix);
  if (metric_prefix.empty()) {
    metric_prefix = default_metric_prefix;
  }

  // Convert labels dictionary to map
  std::map<std::string, std::string> labels;
  if (arg_labels != nullptr && PyDict_Check(arg_labels)) {
    PyObject* py_key = nullptr;
    PyObject* py_value = nullptr;
    Py_ssize_t pos = 0;

    while (PyDict_Next(arg_labels, &pos, &py_key, &py_value)) {
      std::string key = ConvertString(py_key);
      std::string value = ConvertString(py_value);

      if (!key.empty()) {
        labels.insert(std::make_pair(key, value));
      }
    }
  }

  self->connection->set_metric_registry(metric_registry, metric_prefix, labels);

  // Save the capsule so it doesn't get deleted before we're ready
  // Also remove the old capsule if one exists
  Py_XDECREF(self->metric_registry);
  Py_XINCREF(arg_registry);
  self->metric_registry = arg_registry;

  Py_RETURN_TRUE;
}

PyMethodDef Connection::python_methods[] = {
  {"connect", (PyCFunction)Connection::connect, METH_VARARGS | METH_KEYWORDS, Connection::docstring_connect},
  {"disconnect", (PyCFunction)Connection::disconnect, METH_NOARGS, Connection::docstring_disconnect},

  {"get_channel_state", (PyCFunction)Connection::get_channel_state, METH_NOARGS,
   Connection::docstring_get_channel_state},

  {"set_metric_registry", (PyCFunction)Connection::set_metric_registry, METH_VARARGS | METH_KEYWORDS,
   Connection::docstring_set_metric_registry},

  {nullptr, nullptr, 0, nullptr} /* Sentinel */
};

std::string Connection::ReadFile(const std::string& filename) {
  std::ifstream file(filename.c_str(), std::ios::in);

  if (file.is_open()) {
    std::stringstream ss;
    ss << file.rdbuf();
    file.close();

    return ss.str();
  }

  return "";
}

std::string Connection::ConvertString(PyObject* str)
{
  std::string result;

  if (str == nullptr)
  {
    return result;
  }

  const char* convertedString{nullptr};
  if (PyUnicode_Check(str))
  {
    convertedString = PyUnicode_AsUTF8(str);
  }
  else if (PyBytes_Check(str))
  {
    convertedString = PyBytes_AsString(str);
  }

  if (!convertedString)
  {
    PyErr_Clear();
  }
  else
  {
    result = convertedString;
  }

  return result;
}

const char* Connection::default_metric_prefix = "grpc_";

std::string connection_class_name;
static PyTypeObject ConnectionPyType = {
  PyVarObject_HEAD_INIT(nullptr, 0) "Connection",    /* tp_name -- This is replaced in RegisterConnection */
  sizeof(Connection::PythonType),                    /* tp_basicsize */
  0,                                                 /* tp_itemsize */
  (destructor)Connection::dealloc,                   /* tp_dealloc */
  0,                          /* tp_vectorcall_offset*/
  nullptr,                                  /* tp_getattr */
  nullptr,                                           /* tp_setattr */
  nullptr,                                           /* tp_as_async */
  nullptr,                                           /* tp_repr */
  nullptr,                                           /* tp_as_number */
  nullptr,                                           /* tp_as_sequence */
  nullptr,                                           /* tp_as_mapping */
  nullptr,                                           /* tp_hash */
  nullptr,                                           /* tp_call */
  nullptr,                                           /* tp_str */
  nullptr,                                           /* tp_getattro */
  nullptr,                                           /* tp_setattro */
  nullptr,                                           /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT,                                /* tp_flags */
  "Connection for use with the grpc module clients", /* tp_doc */
  nullptr,                                           /* tp_traverse */
  nullptr,                                           /* tp_clear */
  nullptr,                                           /* tp_richcompare */
  0,                                                 /* tp_weaklistoffset */
  nullptr,                                           /* tp_iter */
  nullptr,                                           /* tp_iternext */
  Connection::python_methods,                        /* tp_methods */
  nullptr,                                           /* tp_members */
  nullptr,                                           /* tp_getset */
  nullptr,                                           /* tp_base */
  nullptr,                                           /* tp_dict */
  nullptr,                                           /* tp_descr_get */
  nullptr,                                           /* tp_descr_set */
  0,                                                 /* tp_dictoffset */
  (initproc)Connection::init,                        /* tp_init */
  nullptr,                                           /* tp_alloc */
  nullptr,                                           /* tp_new */
};

void RegisterConnection(PyObject* module, const std::string& class_name) {
  // Copy the class name to guarantee the lifetime of the string pointer
  connection_class_name = class_name;
  ConnectionPyType.tp_name = connection_class_name.c_str();

  ConnectionPyType.tp_new = PyType_GenericNew;
  if (PyType_Ready(&ConnectionPyType) < 0)
    return;

  Py_INCREF(&ConnectionPyType);
  PyModule_AddObject(module, "Connection", (PyObject*)&ConnectionPyType);
}

bool Connection::is_instance(PyObject* obj) {
  return PyObject_IsInstance(obj, (PyObject*)&ConnectionPyType);
}

}  // namespace monolith_grpc::module

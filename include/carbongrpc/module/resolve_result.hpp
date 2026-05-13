// Copyright © 2025 CCP ehf.
#include "resolve_result.h"
using namespace monolith_grpc;
using namespace monolith_grpc::module;

using monolith_grpc::client::Resolver;

int ResolveResult::init(PythonType*, PyObject*, PyObject*) {
  return 0;
}

void ResolveResult::dealloc(PythonType* self) {
  self->result.reset(nullptr);

  Py_TYPE(self)->tp_free((PyObject*)self);
}

const char* ResolveResult::docstring_get_status =
  "get_status() -> int\n"
  "\n"
  "Returns the current status of the query. Possible values are:\n"
  "0 = eve_grpc_client.resolve_in_progress = Query is still in progress\n"
  "1 = eve_grpc_client.resolve_success = Query completed successfully\n"
  "2 = eve_grpc_client.resolve_failure = Query failed.  Use get_error() to "
  "determine the reason for failure.\n"
  "\n"
  ":return: An integer containing the code for the current status, or None "
  "if the status is not available.\n"
  ":rtype: int\n";
PyObject* ResolveResult::get_status(PythonType* self, PyObject*) {
  if (!self->result) {
    Py_RETURN_NONE;
  }

  return PyLong_FromLong((long)self->result->status());
}

const char* ResolveResult::docstring_get_request_name =
  "get_request_name() -> str\n"
  "\n"
  "Returns the original request string passed to query_hostname() or "
  "query_srv().\n"
  "This is a convenience function.\n"
  "\n"
  ":return: A string containing the original query request parameter\n"
  ":rtype: str\n";
PyObject* ResolveResult::get_request_name(PythonType* self, PyObject*) {
  if (!self->result) {
    Py_RETURN_NONE;
  }

  return PyBytes_FromString(self->result->request_name().c_str());
}

const char* ResolveResult::docstring_get_error =
  "get_error() -> Object\n"
  "\n"
  "Returns a tuple with two values:\n"
  "Index 0 contains the integer error code from c-ares. These are mapped to "
  "enum values as eve_grpc_client.ares_error_*.\n"
  "Index 1 contains a human-readable string message for the error code.\n"
  "\n"
  ":return: A tuple containing (int, str), or None if the query is still in "
  "progress.\n"
  ":rtype: str\n";
PyObject* ResolveResult::get_error(PythonType* self, PyObject*) {
  if (!self->result) {
    Py_RETURN_NONE;
  }

  if (self->result->status() == Resolver::Status::kInProgress) {
    Py_RETURN_NONE;
  }

  int err = self->result->error();

  PyObject* tupl = PyTuple_New(2);
  PyTuple_SetItem(tupl, 0, PyLong_FromLong((long)err));

  if (err == ARES_SUCCESS) {
    PyTuple_SetItem(tupl, 1, PyBytes_FromString("Success"));
  } else if (err == ARES_ENOTIMP) {
    PyTuple_SetItem(
      tupl, 1,
      PyUnicode_FromString("The ares library does not know how to "
                          "find addresses of type family.")
    );
  } else if (err == ARES_ETIMEOUT) {
    PyTuple_SetItem(tupl, 1, PyBytes_FromString("No name servers responded within the timeout period."));
  } else if (err == ARES_ENOMEM) {
    PyTuple_SetItem(tupl, 1, PyBytes_FromString("Memory was exhausted."));
  } else if (err == ARES_ENODATA) {
    PyTuple_SetItem(tupl, 1, PyBytes_FromString("There was no data returned to extract a result from."));
  } else if (err == ARES_ECANCELLED) {
    PyTuple_SetItem(tupl, 1, PyBytes_FromString("The query was cancelled."));
  } else if (err == ARES_EDESTRUCTION) {
    PyTuple_SetItem(
      tupl, 1,
      PyUnicode_FromString("The channel is being destroyed; the "
                          "query will not be completed.")
    );
  } else if (err == ARES_EBADNAME) {
    PyTuple_SetItem(tupl, 1, PyBytes_FromString("The name is invalid for the given request."));
  } else if (err == ARES_EFORMERR) {
    PyTuple_SetItem(
      tupl, 1,
      PyBytes_FromString("The query completed but the server claims that "
                          "the query was malformatted.")
    );
  } else if (err == ARES_ESERVFAIL) {
    PyTuple_SetItem(
      tupl, 1,
      PyBytes_FromString("The query completed but the server claims to have "
                          "experienced a failure.")
    );
  } else if (err == ARES_ENOTFOUND) {
    PyTuple_SetItem(
      tupl, 1,
      PyBytes_FromString("The query completed but the requested "
                          "domain name was not found.")
    );
  } else if (err == ARES_EREFUSED) {
    PyTuple_SetItem(tupl, 1, PyBytes_FromString("The query completed but the server refused the query."));
  } else if (err == ARES_ECONNREFUSED) {
    PyTuple_SetItem(tupl, 1, PyBytes_FromString("No name servers could be contacted."));
  }

  return tupl;
}

const char* ResolveResult::docstring_get_official_name =
  "get_official_name() -> str\n"
  "\n"
  "Returns the official name of the queried-for domain as reported by the "
  "dns server.\n"
  "\n"
  ":return: A string containing the official name of the queried-for "
  "domain.\n"
  ":rtype: str\n";
PyObject* ResolveResult::get_official_name(PythonType* self, PyObject*) {
  if (!self->result) {
    Py_RETURN_NONE;
  }

  return PyBytes_FromString(self->result->official_name().c_str());
}

const char* ResolveResult::docstring_get_addresses =
  "get_addresses() -> List[str]\n"
  "\n"
  "Returns the list of addresses associated with the queried-for hostname.\n"
  "This is the primary result of query_hostname().\n"
  "\n"
  ":return: A List[str] containing all addresses associated with the "
  "queried-for hostname.\n"
  ":rtype: str\n";
PyObject* ResolveResult::get_addresses(PythonType* self, PyObject*) {
  if (!self->result) {
    Py_RETURN_NONE;
  }

  auto addresses = self->result->addresses();

  PyObject* py_result = PyList_New((Py_ssize_t)addresses.size());
  for (auto i = 0; i < addresses.size(); i++) {
    auto address = addresses[i];
    PyList_SetItem(py_result, i, PyBytes_FromString(address.c_str()));
  }

  return py_result;
}

const char* ResolveResult::docstring_get_aliases =
  "get_addresses() -> List[str]\n"
  "\n"
  "Returns the list of aliases associated with the queried-for hostname.\n"
  "\n"
  ":return: A List[str] containing all aliases associated with the "
  "queried-for hostname.\n"
  ":rtype: str\n";
PyObject* ResolveResult::get_aliases(PythonType* self, PyObject*) {
  if (!self->result) {
    Py_RETURN_NONE;
  }

  auto aliases = self->result->aliases();

  PyObject* py_result = PyList_New((Py_ssize_t)aliases.size());
  for (auto i = 0; i < aliases.size(); i++) {
    auto alias = aliases[i];
    PyList_SetItem(py_result, i, PyBytes_FromString(alias.c_str()));
  }

  return py_result;
}

const char* ResolveResult::docstring_get_srv_result =
  "get_srv_result() -> List[(host:str, port:int, priority:int, weight:int)]\n"
  "\n"
  "Returns a list of tuples containing the srv data.\n"
  "Each tuple in the list has four elements:\n"
  "Index 0 is the host (string).\n"
  "Index 1 is the port (int).\n"
  "Index 2 is the priority (int).\n"
  "Index 3 is the weight (int).\n"
  "\n"
  ":return: A list of tuples: List[(host, port, priority, weight)] "
  "containing srv data.\n"
  ":rtype: str\n";
PyObject* ResolveResult::get_srv_result(PythonType* self, PyObject*) {
  if (!self->result) {
    Py_RETURN_NONE;
  }

  auto srv_results = self->result->srv_result();
  PyObject* py_result = PyList_New((Py_ssize_t)srv_results.size());
  for (auto i = 0; i < srv_results.size(); i++) {
    auto srv = srv_results[i];
    PyObject* tupl = PyTuple_New(4);
    PyTuple_SetItem(tupl, 0, PyBytes_FromString(srv.host.c_str()));
    PyTuple_SetItem(tupl, 1, PyLong_FromLong((long)srv.port));
    PyTuple_SetItem(tupl, 2, PyLong_FromLong((long)srv.priority));
    PyTuple_SetItem(tupl, 3, PyLong_FromLong((long)srv.weight));

    PyList_SetItem(py_result, i, tupl);
  }

  return py_result;
}

const char* ResolveResult::docstring_set_dns_servers =
  "set_dns_servers(servers:str) -> Object\n"
  "\n"
  "Sets the dns servers that c-ares will use for lookups.\n"
  "The parameter should be a comma-separated list of servers: "
  "8.8.8.8,8.8.4.4,etc\n"
  "\n"
  ":return: True on success, False on failure\n"
  ":rtype: bool\n"
  "\n";
PyObject* ResolveResult::set_dns_servers(PythonType*, PyObject* args, PyObject* keywords) {
  const char* arg_servers = nullptr;

  static const char* keyword_list[] = {"servers", nullptr};

  if (!PyArg_ParseTupleAndKeywords(args, keywords, "s", (char**)keyword_list, &arg_servers) || arg_servers == nullptr) {
    PyErr_SetString(PyExc_TypeError, "Failed to parse the parameters. Expected servers:str");
    return nullptr;
  }

  Resolver::SetServers(arg_servers);

  Py_RETURN_TRUE;
}

const char* ResolveResult::docstring_query_hostname =
  "query_hostname(hostname:str) -> Object\n"
  "\n"
  "Queries a dns server for addresses (A records) and aliases (CNAME "
  "records) corresponding to the given hostname.\n"
  "The lookup is done asynchronously, and this function returns a result "
  "object which can be polled to monitor the job and get the result.\n"
  "\n"
  ":return: A ResolveResult Object\n"
  ":rtype: Object\n"
  "\n";
PyObject* ResolveResult::query_hostname(PythonType*, PyObject* args, PyObject* keywords) {
  const char* arg_name = nullptr;

  static const char* keyword_list[] = {"hostname", nullptr};

  if (!PyArg_ParseTupleAndKeywords(args, keywords, "s", (char**)keyword_list, &arg_name) || arg_name == nullptr) {
    PyErr_SetString(PyExc_TypeError, "Failed to parse the parameters. Expected hostname:str");
    return nullptr;
  }

  std::unique_ptr<Resolver::Result> native_result = Resolver::Resolve(arg_name, Resolver::QueryType::kHostname);
  auto* python_result = (ResolveResult::PythonType*)CreateResolveResult();
  python_result->result = std::move(native_result);

  return (PyObject*)python_result;
}

const char* ResolveResult::docstring_query_srv =
  "query_srv(hostname:str) -> Object\n"
  "\n"
  "Queries a dns server for SRV records associated with the given hostname.\n"
  "The lookup is done asynchronously, and this function returns a result "
  "object which can be polled to monitor the job and get the result.\n"
  "\n"
  ":return: A ResolveResult Object\n"
  ":rtype: Object\n"
  "\n";
PyObject* ResolveResult::query_srv(PythonType*, PyObject* args, PyObject* keywords) {
  const char* arg_name = nullptr;

  static const char* keyword_list[] = {"hostname", nullptr};

  if (!PyArg_ParseTupleAndKeywords(args, keywords, "s", (char**)keyword_list, &arg_name) || arg_name == nullptr) {
    PyErr_SetString(PyExc_TypeError, "Failed to parse the parameters. Expected hostname:str");
    return nullptr;
  }

  std::unique_ptr<Resolver::Result> native_result = Resolver::Resolve(arg_name, Resolver::QueryType::kSRV);
  auto* python_result = (ResolveResult::PythonType*)CreateResolveResult();
  python_result->result = std::move(native_result);

  return (PyObject*)python_result;
}

PyMethodDef ResolveResult::python_methods[] = {
  {"get_status", (PyCFunction)ResolveResult::get_status, METH_NOARGS, ResolveResult::docstring_get_status},
  {"get_request_name", (PyCFunction)ResolveResult::get_request_name, METH_NOARGS,
   ResolveResult::docstring_get_request_name},
  {"get_error", (PyCFunction)ResolveResult::get_error, METH_NOARGS, ResolveResult::docstring_get_error},

  // hostname
  {"get_official_name", (PyCFunction)ResolveResult::get_official_name, METH_NOARGS,
   ResolveResult::docstring_get_official_name},
  {"get_addresses", (PyCFunction)ResolveResult::get_addresses, METH_NOARGS, ResolveResult::docstring_get_addresses},
  {"get_aliases", (PyCFunction)ResolveResult::get_aliases, METH_NOARGS, ResolveResult::docstring_get_aliases},

  // srv
  {"get_srv_result", (PyCFunction)ResolveResult::get_srv_result, METH_NOARGS, ResolveResult::docstring_get_srv_result},

  // dns servers
  {"set_dns_servers", (PyCFunction)ResolveResult::set_dns_servers, METH_VARARGS | METH_KEYWORDS,
   ResolveResult::docstring_set_dns_servers},

  // query
  {"query_hostname", (PyCFunction)ResolveResult::query_hostname, METH_VARARGS | METH_KEYWORDS,
   ResolveResult::docstring_query_hostname},
  {"query_srv", (PyCFunction)ResolveResult::query_srv, METH_VARARGS | METH_KEYWORDS,
   ResolveResult::docstring_query_srv},

  {nullptr, nullptr, 0, nullptr} /* Sentinel */
};

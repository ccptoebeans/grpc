#include "carbongrpc/module/helpers.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>

#include <google/protobuf/message.h>
#include <google/protobuf/pyext/extension_dict.h>
#include <google/protobuf/pyext/message.h>

#include <Python.h>

namespace monolith_grpc::module {

struct HelpersPrivate {
  _typeobject* c_message_type = nullptr;
  const char* c_message_type_string = "google.protobuf.pyext._message.CMessage";
};

std::once_flag Helpers::init_flag_;
Helpers* Helpers::instance_;

/*static*/ Helpers& Helpers::instance() {
  std::call_once(Helpers::init_flag_, []() { Helpers::instance_ = new Helpers(); });
  return *Helpers::instance_;
}

Helpers::Helpers()
  : private_(std::make_unique<HelpersPrivate>()) {
}

std::string Helpers::ReadFile(const std::string& filename) const {
  std::ifstream file(filename.c_str(), std::ios::in);

  if (file.is_open()) {
    std::stringstream ss;
    ss << file.rdbuf();
    file.close();

    return ss.str();
  }

  return "";
}

std::string Helpers::ConvertString(PyObject* str) const
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

std::chrono::system_clock::time_point Helpers::ToSystemTime(std::chrono::steady_clock::time_point t) const {
  auto steady_now = std::chrono::steady_clock::now();
  auto system_now = std::chrono::system_clock::now();
  std::chrono::system_clock::time_point system_t =
    system_now + std::chrono::duration_cast<std::chrono::system_clock::duration>(t - steady_now);
  return system_t;
}

/*
GetCProtoInsidePyProto
This function pulls out a protobuf::Message* from a PyObject* if that PyObject
has a native descriptor loaded. Native descriptors inherit
google.protobuf.pyext._message.CMessage Python descriptors inherit
google.protobuf.pyext._message.MessageMeta

Unfortunately, examining the _message module does not expose CMessage in any way
that I've been able to find. _message.CMessage can't be looked up in attrs, even
after the pyd is loaded and protobufs objects have been created.
_message.Message points to MessageMeta

The only way I've been able to get a CMessage reference is to pull it out of a
valid CMessage that comes through this function. The initial validation has to
be done the slow way through string comparisons of the type. After that, we can
use the fast way by comparing the _typeobject directly.
*/

google::protobuf::Message* Helpers::GetCProtoInsidePyProto(PyObject* msg) const {
  // Fast way. Only works once we've found a valid CMessage.
  if (private_->c_message_type != nullptr) {
    _typeobject* tp = msg->ob_type;
    while (tp != nullptr) {
      if (tp == private_->c_message_type) {
        auto* cmsg = reinterpret_cast<google::protobuf::python::CMessage*>(msg);
        return cmsg->message;
      }
      tp = tp->tp_base;
    }
    return nullptr;
  }

  // Slow way using string comparisons. Used until we find a valid CMessage.
  else {
    bool isCMessage = false;
    if (strcmp(msg->ob_type->tp_name, private_->c_message_type_string) == 0) {
      isCMessage = true;
      private_->c_message_type = msg->ob_type;
    } else {
      _typeobject* base = msg->ob_type->tp_base;
      while (base != nullptr && !isCMessage) {
        if (strcmp(base->tp_name, private_->c_message_type_string) == 0) {
          isCMessage = true;
          private_->c_message_type = base;
        } else {
          base = base->tp_base;
        }
      }
    }

    if (!isCMessage) {
      return nullptr;
    }

    auto* cmsg = reinterpret_cast<google::protobuf::python::CMessage*>(msg);
    return cmsg->message;
  }
}

PyObject* Helpers::CreatePythonObject(const char* module_name, const char* type_name) {
  PyObject* global_module_dict = PyImport_GetModuleDict();                                         // borrowed reference
  PyObject* module = PyMapping_GetItemString(global_module_dict, const_cast<char*>(module_name));  // new reference
  if (module == nullptr) {
    std::string error_text = std::string("Failed to load the module");
    PyErr_SetString(PyExc_RuntimeError, error_text.c_str());
    return nullptr;
  }

  PyObject* module_dict = PyModule_GetDict(module);  // borrowed reference
  if (module_dict == nullptr) {
    std::string error_text = std::string("Failed to get the dictionary from the module: ") + std::string(module_name);
    PyErr_SetString(PyExc_RuntimeError, error_text.c_str());
    return nullptr;
  }

  PyObject* py_class = PyDict_GetItemString(module_dict,
                                            const_cast<char*>(type_name));  // borrowed reference
  if (py_class == nullptr) {
    std::string error_text = std::string("Failed to load the class from the module dict: ") + std::string(type_name);
    PyErr_SetString(PyExc_RuntimeError, error_text.c_str());
    return nullptr;
  }

  if (!PyCallable_Check(py_class)) {
    std::string error_text = std::string("class is not callable: ") + std::string(type_name);
    PyErr_SetString(PyExc_RuntimeError, error_text.c_str());
    return nullptr;
  }

  PyObject* py_instance = PyObject_CallObject(py_class, nullptr);  // new reference
  if (py_instance == nullptr) {
    std::string error_text = std::string("Failed to instantiate the type: ") + std::string(module_name) +
                             std::string(".") + std::string(type_name);
    PyErr_SetString(PyExc_RuntimeError, error_text.c_str());
    return nullptr;
  }

  Py_DecRef(module);

  return py_instance;
}

#ifdef DEBUG
// cppcheck-suppress unusedFunction
void Helpers::PrintDictionary(PyObject* dict) const {
  PyObject* key;
  PyObject* value;
  Py_ssize_t pos = 0;
  while (PyDict_Next(dict, &pos, &key, &value)) {
    PyObject* reprk = PyObject_Repr(key);
    PyObject* reprv = PyObject_Repr(value);
    printf("%s = %s\n", PyString_AsString(reprk), PyString_AsString(reprv));
    Py_DecRef(reprk);
    Py_DecRef(reprv);
  }
}

// cppcheck-suppress unusedFunction
void Helpers::PrintTimestamp(std::chrono::system_clock::time_point timestamp) const {
  std::chrono::system_clock::time_point::duration epoch = timestamp.time_since_epoch();
  std::time_t epoch_seconds = std::chrono::duration_cast<std::chrono::seconds>(epoch).count();
  long long epoch_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(epoch).count();
  // No safe cross-platform alternative to gmtime exists.
  // gmtime itself is unsafe.  This must never be used in production code.
  std::tm gmt_seconds = *std::gmtime(&epoch_seconds);
  long long remainder_microseconds = epoch_microseconds - (epoch_seconds * 1000000);
  std::cout << std::put_time(&gmt_seconds, "%Y-%m-%d %H:%M:%S.") << std::setw(6) << std::setfill('0')
            << int(epoch_microseconds - remainder_microseconds) << std::endl;
}
#endif

}  // namespace monolith_grpc::module

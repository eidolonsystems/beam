#include "Beam/Python/KeyValuePair.hpp"
#include <boost/lexical_cast.hpp>
#include <pybind11/operators.h>
#include "Beam/Python/Utilities.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace pybind11;

void Beam::Python::ExportKeyValuePair(pybind11::module& module) {
  class_<KeyValuePair<object, object>>(module, "KeyValuePair")
    .def(init())
    .def(init<object, object>())
    .def(init<const KeyValuePair<object, object>&>())
    .def_readwrite("key", &KeyValuePair<object, object>::m_key)
    .def_readwrite("value", &KeyValuePair<object, object>::m_value)
    .def("__str__", &lexical_cast<std::string, KeyValuePair<object, object>>)
    .def(self == self)
    .def(self != self);
}

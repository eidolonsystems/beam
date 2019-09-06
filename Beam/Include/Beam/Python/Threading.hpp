#ifndef BEAM_PYTHON_THREADING_HPP
#define BEAM_PYTHON_THREADING_HPP
#include <pybind11/pybind11.h>

namespace Beam::Python {

  /**
   * Exports the LiveTimer class.
   * @param module The module to export to.
   */
  void ExportLiveTimer(pybind11::module& module);

  /**
   * Exports the Threading namespace.
   * @param module The module to export to.
   */
  void ExportThreading(pybind11::module& module);

  /**
   * Exports the Timer class.
   * @param module The module to export to.
   */
  void ExportTimer(pybind11::module& module);

  /**
   * Exports the TriggerTimer class.
   * @param module The module to export to.
   */
  void ExportTriggerTimer(pybind11::module& module);
}

#endif

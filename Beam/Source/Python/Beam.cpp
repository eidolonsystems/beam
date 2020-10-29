#include "Beam/Python/Beam.hpp"
#include <datetime.h>
#include <pybind11/pybind11.h>
#include "Beam/Utilities/ApplicationInterrupt.hpp"

using namespace Beam;
using namespace Beam::Network;
using namespace Beam::Python;
using namespace Beam::Threading;
using namespace pybind11;

template struct Beam::Routines::Details::CurrentRoutineGlobal<void>;
template struct Beam::Routines::Details::NextId<void>;

PYBIND11_MODULE(_beam, module) {
  ExportIO(module);
  ExportKeyValuePair(module);
  ExportNetwork(module);
  ExportQueries(module);
  ExportQueues(module);
  ExportReactors(module);
  ExportRoutines(module);
  ExportServiceLocator(module);
  ExportSql(module);
  ExportThreading(module);
  ExportTimeService(module);
  ExportUidService(module);
  ExportWebServices(module);
  ExportYaml(module);
  module.def("is_running", &IsRunning);
  module.def("received_kill_event", &ReceivedKillEvent);
  module.def("wait_for_kill_event", &WaitForKillEvent);
}

#include "Beam/QueuesTests/QueueTester.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

using namespace Beam;
using namespace Beam::Routines;
using namespace Beam::Tests;
using namespace std;

void QueueTester::TestBreak() {
  Queue<int> q;
  RoutineHandler r1 = Spawn(
    [&] {
      q.Top();
    });
  RoutineHandler r2 = Spawn(
    [&] {
      q.Top();
    });
  q.Break();
  r1.Wait();
  r2.Wait();
}

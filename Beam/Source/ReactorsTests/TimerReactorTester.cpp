#include "Beam/ReactorsTests/TimerReactorTester.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;

void TimerReactorTester::TestExpiry() {
/*
  Trigger trigger;
  Trigger::SetEnvironmentTrigger(trigger);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  auto period = MakeConstantReactor(time_duration{seconds(5)});
  std::shared_ptr<TriggerTimer> timer;
  auto timerFactory =
    [&] (time_duration duration) {
      timer = std::make_shared<TriggerTimer>();
      return timer;
    };
  auto reactor = MakeTimerReactor<int>(timerFactory, period);
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::NONE);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 1, BaseReactor::Update::EVAL, 0);
  timer->Trigger();
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 2);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 2, BaseReactor::Update::EVAL, 1);
*/
}

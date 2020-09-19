#ifndef BEAM_PUBLISHER_REACTOR_HPP
#define BEAM_PUBLISHER_REACTOR_HPP
#include <memory>
#include <Aspen/Lift.hpp>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Reactors/QueueReactor.hpp"

namespace Beam::Reactors {
namespace Details {
  template<typename P>
  struct PublisherReactorCore {
    using Publisher = P;
    using Type = typename GetTryDereferenceType<Publisher>::Type;
    Publisher m_publisher;

    template<typename PF>
    PublisherReactorCore(PF&& publisher)
      : m_publisher(std::forward<PF>(publisher)) {}

    const Type& operator ()(const Type& type) const {
      return type;
    }
  };
}

  /**
   * Makes a Reactor that monitors a Publisher.
   * @param publisher The Publisher to monitor.
   */
  template<typename Type>
  auto PublisherReactor(const Publisher<Type>& publisher) {
    auto queue = std::make_shared<Queue<Type>>();
    publisher.Monitor(queue);
    return QueueReactor(std::move(queue));
  }

  /**
   * Makes a Reactor that monitors a Publisher, used when ownership of the
   * Publisher is managed by the Reactor.
   * @param publisher The Publisher to monitor.
   */
  template<typename Publisher>
  auto PublisherReactor(std::shared_ptr<Publisher> publisher) {
    return Aspen::lift(Details::PublisherReactorCore<
      std::shared_ptr<Publisher>>(publisher), PublisherReactor(*publisher));
  }
}

#endif

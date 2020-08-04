#ifndef BEAM_PUBLISHER_HPP
#define BEAM_PUBLISHER_HPP
#include <functional>
#include <memory>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/ScopedQueueWriter.hpp"

namespace Beam {

  /** Base class for the parametric Publisher. */
  class BasePublisher {
    public:
      virtual ~BasePublisher() = default;

      /**
       * Synchronizes access to this publisher.
       * @param f The synchronized action to perform.
       */
      virtual void With(const std::function<void ()>& f) const = 0;

    protected:

      /** Constructs a BasePublisher. */
      BasePublisher() = default;
  };

  /**
   * Interface for an object that publishes data to Queues.
   * @param <T> The type of data being published.
   */
  template<typename T>
  class Publisher : public BasePublisher {
    public:

      /** The type of data published. */
      using Type = T;

      /**
       * Monitors updates to this Publisher.
       * @param monitor The monitor to publish updates to.
       */
      virtual void Monitor(ScopedQueueWriter<Type> monitor) const = 0;

    protected:

      /** Constructs a Publisher. */
      Publisher() = default;
  };

  /**
   * Specifies the type of values a Publisher produces.
   * @param <P> The type of Publisher to inspect.
   */
  template<typename P>
  struct PublisherType {
    using type = typename GetTryDereferenceType<P>::Type;
  };

  template<typename P>
  using GetPublisherType = typename PublisherType<P>::type;
}

#endif

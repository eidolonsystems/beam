#ifndef BEAM_QUERIES_NATIVE_VALUE_HPP
#define BEAM_QUERIES_NATIVE_VALUE_HPP
#include <type_traits>
#include <utility>
#include "Beam/Queries/NativeDataType.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/Value.hpp"

namespace Beam::Queries {

  /**
   * Stores a Value using a native type.
   * @param <T> The DataType represented.
   */
  template<typename T>
  class NativeValue :
      public VirtualValue, public CloneableMixin<NativeValue<T>> {
    public:

      /** The DataType represented. */
      using Type = T;

      /** Constructs a NativeValue. */
      NativeValue();

      /**
       * Copies a NativeValue.
       * @param value The value to copy.
       */
      NativeValue(const NativeValue& value) = default;

      /**
       * Constructs a NativeValue.
       * @param value Initializes the value.
       */
      template<typename V, typename = std::enable_if_t<
        !std::is_base_of_v<NativeValue, std::decay_t<V>>>>
      explicit NativeValue(V&& value);

      const DataType& GetType() const override;

      /**
       * Compares two NativeValues for equality.
       * @param value The value to compare to.
       * @return <code>true</code> iff the value wrapped by <i>this</i> is equal
       *         to the value wrapped by <i>value</i>.
       */
      bool operator ==(const NativeValue& value) const;

      /**
       * Compares two NativeValues for inequality.
       * @param value The value to compare to.
       * @return <code>true</code> iff the value wrapped by <i>this</i> is
       *         not equal to the value wrapped by <i>value</i>.
       */
      bool operator !=(const NativeValue& value) const;

    protected:
      const void* GetValuePtr() const override;
      std::ostream& ToStream(std::ostream& out) const override;
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);

    private:
      friend struct Beam::Serialization::DataShuttle;
      DataType m_type;
      typename Type::Type m_value;
  };

  template<typename V, typename = void>
  NativeValue(V&& value) -> NativeValue<NativeDataType<std::decay_t<V>>>;

  template<typename T>
  NativeValue<T>::NativeValue()
    : m_value(),
      m_type(Type::GetInstance()) {}

  template<typename T>
  template<typename V, typename>
  NativeValue<T>::NativeValue(V&& value)
    : m_value(std::forward<V>(value)),
      m_type(Type::GetInstance()) {}

  template<typename T>
  const DataType& NativeValue<T>::GetType() const {
    return m_type;
  }

  template<typename T>
  bool NativeValue<T>::operator ==(const NativeValue& value) const {
    return m_value == value.m_value;
  }

  template<typename T>
  bool NativeValue<T>::operator !=(const NativeValue& value) const {
    return !(*this == value);
  }

  template<typename T>
  const void* NativeValue<T>::GetValuePtr() const {
    return &m_value;
  }

  template<typename T>
  std::ostream& NativeValue<T>::ToStream(std::ostream& out) const {
    return out << m_value;
  }

  template<typename T>
  template<typename Shuttler>
  void NativeValue<T>::Shuttle(Shuttler& shuttle, unsigned int version) {
    VirtualValue::Shuttle(shuttle, version);
    shuttle.Shuttle("value", m_value);
  }
}

#endif

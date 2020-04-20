#ifndef BEAM_LISTPARSER_HPP
#define BEAM_LISTPARSER_HPP
#include <cassert>
#include <type_traits>
#include <vector>
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SkipSpaceParser.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam::Parsers {

  /*! \class ListParser
      \brief Matches a list of values seperated by a delimiter.
      \tparam P The Parser used for each value in the list.
   */
  template<typename P>
  class ListParser {
    public:

      //! The Parser used for each value in the list.
      using Parser = P;
      using Result = std::conditional_t<std::is_same_v<typename Parser::Result,
        NullType>, NullType, std::vector<typename Parser::Result>>;

      //! Constructs a ListParser.
      /*!
        \param parser The Parser to match.
        \param delimiter The delimiter used to separate list items.
      */
      ListParser(Parser parser, char delimiter);

      template<typename Stream>
      bool Read(Stream& source, Result& value) const;

      template<typename Stream>
      bool Read(Stream& source) const;

    private:
      Parser m_parser;
      char m_delimiter;
  };

  //! Builds a ListParser based on its value Parser.
  /*!
    \param parser The Parser used for the values in the list.
    \param delimiter The list's delimiter.
  */
  template<typename Parser>
  auto List(Parser parser, char delimiter) {
    return ListParser(std::move(parser), delimiter);
  }

  template<typename P>
  ListParser<P>::ListParser(Parser parser, char delimiter)
    : m_parser(std::move(parser)),
      m_delimiter(delimiter) {}

  template<typename P>
  template<typename Stream>
  bool ListParser<P>::Read(Stream& source, Result& value) const {
    value.clear();
    {
      auto context = SubParserStream(source);
      auto listValue = typename Parser::Result();
      if(!m_parser.Read(context, listValue)) {
        return true;
      }
      value.push_back(std::move(listValue));
      context.Accept();
    }
    while(true) {
      auto context = SubParserStream(source);
      SkipSpaceParser().Read(context);
      if(!context.Read()) {
        return true;
      }
      if(context.GetChar() != m_delimiter) {
        return true;
      }
      SkipSpaceParser().Read(context);
      auto listValue = typename Parser::Result();
      if(!m_parser.Read(context, listValue)) {
        return true;
      }
      value.push_back(std::move(listValue));
      context.Accept();
    }
    assert(false);
    return false;
  }

  template<typename P>
  template<typename Stream>
  bool ListParser<P>::Read(Stream& source) const {
    {
      auto context = SubParserStream(source);
      if(!m_parser.Read(context)) {
        return true;
      }
      context.Accept();
    }
    while(true) {
      auto context = SubParserStream(source);
      SkipSpaceParser().Read(context);
      if(!context.Read()) {
        return true;
      }
      if(context.GetChar() != m_delimiter) {
        return true;
      }
      SkipSpaceParser().Read(context);
      if(!m_parser.Read(context)) {
        return true;
      }
      context.Accept();
    }
    assert(false);
    return false;
  }
}

#endif

#ifndef __MML_TARGETS_SYMBOL_H__
#define __MML_TARGETS_SYMBOL_H__

#include <string>
#include <memory>
#include <cdk/types/basic_type.h>
// #include "mml_parser.tab.h"

namespace mml {

  class symbol {
    std::shared_ptr<cdk::basic_type> _type;
    std::string _identifier;
    int _qualifier;
    int _offset;
    bool _function;
    bool _global;

  public:
    symbol(std::shared_ptr<cdk::basic_type> type, const std::string &identifier, int qualifier, bool function, bool global) :
        _type(type), _identifier(identifier), _qualifier(qualifier), _offset(0), _function(function), _global(global) {
    }

    virtual ~symbol() {
      // EMPTY
    }

    std::shared_ptr<cdk::basic_type> type() const {
      return _type;
    }
    bool is_typed(cdk::typename_type name) const {
      return _type->name() == name;
    }
    const std::string &identifier() const {
      return _identifier;
    }
    int qualifier() const {
      return _qualifier;
    }
    void offset(int offset) {
      _offset = offset;
    }
    int offset() const {
      return _offset;
    }
    bool function() const {
      return _function;
    }
    bool global() const {
	  return _global;
    }
    void global(bool global) {
      _global = global;
    }
  };

} // mml

#endif

#ifndef __MML_TARGETS_SYMBOL_H__
#define __MML_TARGETS_SYMBOL_H__

#include <string>
#include <memory>
#include <cdk/types/basic_type.h>

namespace mml {

  class symbol {
    std::shared_ptr<cdk::basic_type> _type;
    std::string _identifier;
    int _qualifier;
    int _offset;
    bool _global;
    void *_value;

  public:
    symbol(std::shared_ptr<cdk::basic_type> type, const std::string &identifier, int qualifier) :
        _type(type), _identifier(identifier), _qualifier(qualifier), _offset(0), _global(false) {
    }

    symbol(std::shared_ptr<cdk::basic_type> type, const std::string &identifier, int qualifier, void *value) :
        _type(type), _identifier(identifier), _qualifier(qualifier), _offset(0), _global(false), _value(value) {
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
    void global(bool global) {
      _global = global;
    }
    bool global() const {
      return _global;
    }
    void * value() const {
      return _value;
    }
    void value(void *value) {
      _value = value;
    }
  };

} // mml

#endif

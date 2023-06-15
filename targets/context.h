#ifndef __MML_TARGETS_CONTEXT_H__
#define __MML_TARGETS_CONTEXT_H__

#include <cstdint>

namespace mml {

  enum context_type : uint64_t {
    CONTEXT_GLOBAL = 0,
    CONTEXT_MAIN_BODY = 1UL << 0,
    CONTEXT_FUNCTION_BODY = 1UL << 1,
    CONTEXT_FUNCTION_ARGS = 1UL << 2,
  };

} // mml

#endif

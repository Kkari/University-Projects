#pragma once

#include <exception>
#include <stdexcept>
#include <iostream>
#include <cstdlib>

#define _CG_STRINGIFY2(a) #a
#define _CG_STRINGIFY1(a) _CG_STRINGIFY2(a)

#  ifndef CG_NO_ASSERTIONS
#    define cg_assert(a)                                        \
       do {                                                     \
           if(!(a)) {                                           \
               std::cerr                                        \
                   << __FILE__ ":" _CG_STRINGIFY1(__LINE__)     \
                   << ": Assertion '"#a"' failed" << std::endl; \
               std::abort();                                    \
           }                                                    \
       } while(0)
#  else
#    define cg_assert(a) ((void) 0)
#  endif

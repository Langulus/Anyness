#pragma once
#include "Logger.hpp"
#include <stdexcept>

namespace Langulus
{

   ///                                                                        
   ///   A common langulus exception                                          
   ///                                                                        
   struct Exception : public ::std::runtime_error {
      Exception() noexcept
         : ::std::runtime_error {"<no information provided>"} { }
      Exception(const char* what) noexcept
         : ::std::runtime_error {what} { }
      Exception(Logger::Instance&) noexcept
			: Exception { } { }

      /// Get exception name                                                  
      ///   @return the name of the exception                                 
      virtual Token GetName() const noexcept {
         return u8"Unknown";
      }
   };

} // namespace Langulus

/// Convenience macro of declaring an exception                               
#define LANGULUS_EXCEPTION(name) \
   namespace Langulus::Except \
   { \
      struct name : public Exception { \
         using Exception::Exception; \
         Token GetName() const noexcept override { \
            return u8#name; \
         } \
      }; \
   }

LANGULUS_EXCEPTION(Copy);
LANGULUS_EXCEPTION(Move);
LANGULUS_EXCEPTION(Access);
LANGULUS_EXCEPTION(Convert);
LANGULUS_EXCEPTION(Allocate);
LANGULUS_EXCEPTION(Mutate);
LANGULUS_EXCEPTION(Construct);
LANGULUS_EXCEPTION(Destruct);
LANGULUS_EXCEPTION(Reference);
LANGULUS_EXCEPTION(Overflow);
LANGULUS_EXCEPTION(Underflow);
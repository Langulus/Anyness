#pragma once
#include "Logger.hpp"
#include <stdexcept>

namespace Langulus::Except
{
   
   struct Exception : public std::runtime_error {
      Exception() noexcept : std::runtime_error{"Exception: "} { }
      Exception(Logger::Instance&) noexcept { }
   };
  
   struct Copy : public Exception {
      using Exception::Exception;
   };
   
   struct Move : public Exception {
      using Exception::Exception;
   };
   
   struct Access : public Exception {
      using Exception::Exception;
   };
   
   struct Convert : public Exception {
      using Exception::Exception;
   };
   
   struct Allocate : public Exception {
      using Exception::Exception;
   };
   
   struct Mutate : public Exception {
      using Exception::Exception;
   };
   
   struct Construct : public Exception {
      using Exception::Exception;
   };
   
   struct Destruct : public Exception {
      using Exception::Exception;
   };
   
   struct Reference : public Exception {
      using Exception::Exception;
   };
   
   struct Overflow : public Exception {
      using Exception::Exception;
   };
   
   struct Underflow : public Exception {
      using Exception::Exception;
   };
   
} // Langulus::Except

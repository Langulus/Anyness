#pragma once
#include "Integration.hpp"

namespace Langulus::Logger
{
   
   struct Instance {
      template<class T>
      Instance& operator << (const T&) noexcept;
   };
   
   extern Instance GlobalLogger;
   
   template<class... T>
   Instance& Error(T...) noexcept;
   template<class... T>
   Instance& Warning(T...) noexcept;
   template<class... T>
   Instance& Verbose(T...) noexcept;
   
} // namespace Langulus::Logger

#include "Logger.inl"

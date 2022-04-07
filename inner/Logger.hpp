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
   
} // Langulus::Logger

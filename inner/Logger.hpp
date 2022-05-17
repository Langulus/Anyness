///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 - 2022 Dimo Markov <langulusteam@gmail.com>					
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
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

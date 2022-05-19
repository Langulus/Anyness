///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include <Langulus.Core.hpp>

namespace Langulus::Logger
{
   
   class Interface {
   public:
      template<class T>
      Interface& operator << (const T&) noexcept;
   };
   
   extern Interface GlobalLogger;
   
   template<class... T>
   Interface& Error(T...) noexcept;
   template<class... T>
   Interface& Warning(T...) noexcept;
   template<class... T>
   Interface& Verbose(T...) noexcept;
   
} // namespace Langulus::Logger

#include "Logger.inl"

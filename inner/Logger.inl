///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "Logger.hpp"

namespace Langulus::Logger
{
   
   /// Convert a single element to text and log it                            
   ///   @param item - item convert to text and log                           
   ///   @return a reference to the logger for chaining                       
   template<class T>
   Interface& Interface::operator << (const T& item) noexcept {
      return GlobalLogger;
   }
    
   /// Log an error message                                                   
   ///   @param items... - a list of items to convert to text and log         
   ///   @return a reference to the logger for chaining                       
   template<class... T>
   Interface& Error(T... items) noexcept {
      return (GlobalLogger << ... << items);
   }
   
   /// Log a warning message                                                  
   ///   @param items... - a list of items to convert to text and log         
   ///   @return a reference to the logger for chaining                       
   template<class... T>
   Interface& Warning(T... items) noexcept {
      return (GlobalLogger << ... << items);
   }
   
   /// Log a verbose message                                                  
   ///   @param items... - a list of items to convert to text and log         
   ///   @return a reference to the logger for chaining                       
   template<class... T>
   Interface& Verbose(T... items) noexcept {
      return (GlobalLogger << ... << items);
   }
   
} // namespace Langulus::Logger

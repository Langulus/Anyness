#pragma once
#include "Logger.hpp"

namespace Langulus::Logger
{
   
   /// Convert a single element to text and log it                            
   ///   @param item - item convert to text and log                           
   ///   @return a reference to the logger for chaining                       
   template<class T>
   Instance& Instance::operator << (const T& item) noexcept {
      return GlobalLogger;
   }
    
   /// Log an error message                                                   
   ///   @param items... - a list of items to convert to text and log         
   ///   @return a reference to the logger for chaining                       
   template<class... T>
   Instance& Error(T... items) noexcept {
      return (GlobalLogger << ... << items);
   }
   
   /// Log a warning message                                                  
   ///   @param items... - a list of items to convert to text and log         
   ///   @return a reference to the logger for chaining                       
   template<class... T>
   Instance& Warning(T... items) noexcept {
      return (GlobalLogger << ... << items);
   }
   
   /// Log a verbose message                                                  
   ///   @param items... - a list of items to convert to text and log         
   ///   @return a reference to the logger for chaining                       
   template<class... T>
   Instance& Verbose(T... items) noexcept {
      return (GlobalLogger << ... << items);
   }
   
} // namespace Langulus::Logger

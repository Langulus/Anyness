///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Config.hpp"

namespace Langulus::Anyness
{

   ///                                                                        
   ///   Data comparison results                                              
   ///                                                                        
   struct Compared {
      LANGULUS(POD) true;
      LANGULUS(NULLIFIABLE) true;

      enum Enum : ::std::uint8_t {
         // Elements were not compared                                  
         Unknown = 0,

         // LHS == RHS                                                  
         Equal = 1,

         // LHS != RHS                                                  
         Unequal = 2,

         // LHS > RHS                                                   
         Greater = 3,

         // LHS < RHS                                                   
         Lower = 4
      };

      using Type = TypeOf<Enum>;

      Type mResult {Unknown};

   public:
      constexpr Compared() noexcept = default;
      constexpr Compared(const Type& a) noexcept
         : mResult {a} {}

      explicit constexpr operator bool() const noexcept {
         return mResult != Unknown;
      }
      constexpr bool operator == (const Compared::Enum& a) const noexcept {
         return mResult == a;
      }
   };
   
} // namespace Langulus::Anyness

///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Config.hpp"
#include "../Text.hpp"


namespace Langulus::Anyness
{

   ///                                                                        
   ///   Charge, carrying the four verb dimensions                            
   ///                                                                        
   class Charge {
      LANGULUS(POD) true;
      LANGULUS(NULLIFIABLE) false;
      LANGULUS_CONVERSIONS(Debug);

      // Mass of the verb                                               
      Real mMass = DefaultMass;
      // Frequency of the verb                                          
      Real mRate = DefaultRate;
      // Time of the verb                                               
      Real mTime = DefaultTime;
      // Priority of the verb                                           
      Real mPriority = DefaultPriority;

   public:
      static constexpr Real DefaultMass {1};
      static constexpr Real DefaultRate {0};
      static constexpr Real DefaultTime {0};

      static constexpr Real DefaultPriority {0};
      static constexpr Real MinPriority {-10000};
      static constexpr Real MaxPriority {+10000};

      constexpr Charge(
         Real = DefaultMass,
         Real = DefaultRate,
         Real = DefaultTime,
         Real = DefaultPriority
      ) noexcept;

      NOD() constexpr bool operator == (const Charge&) const noexcept;

      NOD() constexpr Charge operator * (const Real&) const noexcept;
      NOD() constexpr Charge operator ^ (const Real&) const noexcept;

      NOD() constexpr Charge& operator *= (const Real&) noexcept;
      NOD() constexpr Charge& operator ^= (const Real&) noexcept;

      NOD() constexpr bool IsDefault() const noexcept;
      NOD() constexpr bool IsFlowDependent() const noexcept;
      NOD() Hash GetHash() const noexcept;
      void Reset() noexcept;

      NOD() explicit operator Debug() const;
   };

} // namespace Langulus::Anyness

#include "Charge.inl"
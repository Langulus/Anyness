///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TUnorderedSet.hpp"

namespace Langulus::Anyness
{

   ///                                                                        
   /// A highly optimized ordered hashset implementation, using the Robin     
   /// Hood algorithm                                                         
   ///                                                                        
   template<CT::Data T>
   class TOrderedSet : public TUnorderedSet<T> {
   public:
      using Self = TOrderedSet<T>;
      using Base = TUnorderedSet<T>;

      static constexpr bool Ordered = true;

      using Base::TUnorderedSet;

      TOrderedSet(const TOrderedSet&);
      TOrderedSet(TOrderedSet&&) noexcept;

      //TODO defined in header due to MSVC compiler bug (02/2023)       
      // Might be fixed in the future                                   
      template<CT::Semantic S>
      constexpr TOrderedSet(S&& other) noexcept requires (CT::Exact<TypeOf<S>, Self>)
         : Base {other.template Forward<Base>()} {}

      TOrderedSet& operator = (const TOrderedSet&);
      TOrderedSet& operator = (TOrderedSet&&) noexcept;

      TOrderedSet& operator = (const T&);
      TOrderedSet& operator = (T&&);
      TOrderedSet& operator = (CT::Semantic auto&&);

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Count Insert(const T&);
      Count Insert(T&&);
      template<CT::Semantic S>
      Count Insert(S&&) requires (CT::Exact<TypeOf<S>, T>);

      TOrderedSet& operator << (const T&);
      TOrderedSet& operator << (T&&);
      template<CT::Semantic S>
      TOrderedSet& operator << (S&&) requires (CT::Exact<TypeOf<S>, T>);
   };

   /// The default set is always ordered                                      
   template<CT::Data T>
   using TSet = TOrderedSet<T>;

} // namespace Langulus::Anyness

#include "TOrderedSet.inl"

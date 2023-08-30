///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TUnorderedMap.hpp"

namespace Langulus::Anyness
{

   ///                                                                        
   /// A highly optimized ordered hashmap implementation, using the Robin     
   /// Hood algorithm                                                         
   ///                                                                        
   template<CT::Data K, CT::Data V>
   class TOrderedMap : public TUnorderedMap<K, V> {
   public:
      using Key = K;
      using Value = V;
      using Self = TOrderedMap<K, V>;
      using Pair = TPair<K, V>;
      using PairRef = TPair<K&, V&>;
      using PairConstRef = TPair<const K&, const V&>;
      using Base = TUnorderedMap<K, V>;

      static constexpr bool Ordered = true;

      using Base::TUnorderedMap;

      TOrderedMap(const TOrderedMap&);
      TOrderedMap(TOrderedMap&&) noexcept;

      //TODO defined in header due to MSVC compiler bug (02/2023)       
      // Might be fixed in the future                                   
      template<CT::Semantic S>
      constexpr TOrderedMap(S&& other) noexcept requires (CT::Exact<TypeOf<S>, Self>)
         : Base {other.template Forward<Base>()} {}

      TOrderedMap& operator = (const TOrderedMap&);
      TOrderedMap& operator = (TOrderedMap&&) noexcept;
      template<CT::Semantic S>
      TOrderedMap& operator = (S&&) noexcept requires (CT::Exact<TypeOf<S>, Self>);

      TOrderedMap& operator = (const Pair&);
      TOrderedMap& operator = (Pair&&) noexcept;
      template<CT::Semantic S>
      TOrderedMap& operator = (S&&) noexcept requires (CT::Pair<TypeOf<S>>);

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      bool operator == (const TOrderedMap&) const;

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Count Insert(const K&, const V&);
      Count Insert(K&&, const V&);
      Count Insert(const K&, V&&);
      Count Insert(K&&, V&&);
      template<CT::Semantic SK, CT::Semantic SV>
      Count Insert(SK&&, SV&&) noexcept requires (CT::Exact<TypeOf<SK>, K>&& CT::Exact<TypeOf<SV>, V>);

      TOrderedMap& operator << (const TPair<K, V>&);
      TOrderedMap& operator << (TPair<K, V>&&);
      template<CT::Semantic S>
      TOrderedMap& operator << (S&&) noexcept requires (CT::Pair<TypeOf<S>>);
   };


   /// The default map is always ordered                                      
   template<CT::Data K, CT::Data V>
   using TMap = TOrderedMap<K, V>;

} // namespace Langulus::Anyness

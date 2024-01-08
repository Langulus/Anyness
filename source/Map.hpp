///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "blocks/BlockMap.hpp"


namespace Langulus::Anyness
{

   ///                                                                        
   ///   Type-erased map                                                      
   ///                                                                        
   template<bool ORDERED = false>
   struct Map : BlockMap {
      LANGULUS(POD) false;
      LANGULUS_BASES(BlockMap);

      friend struct BlockMap;
      static constexpr bool Ownership = true;
      static constexpr bool Ordered = ORDERED;

      using Key = void;
      using Value = void;
      using Self = Map<ORDERED>;
      using Pair = TPair<Block, Block>;
      using PairRef = Pair;
      using PairConstRef = Pair;

      ///                                                                     
      ///   Construction                                                      
      ///                                                                     
      constexpr Map() noexcept = default;
      Map(const Map&);
      Map(Map&&);

      template<class T1, class...TAIL>
      requires CT::Inner::UnfoldInsertable<T1, TAIL...>
      Map(T1&&, TAIL&&...);

      ~Map();

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      Map& operator = (const Map&);
      Map& operator = (Map&&);
      Map& operator = (CT::Inner::UnfoldInsertable auto&&);

      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      NOD() DMeta GetKeyType() const noexcept;
      NOD() DMeta GetValueType() const noexcept;
      NOD() constexpr bool IsKeyUntyped() const noexcept;
      NOD() constexpr bool IsValueUntyped() const noexcept;
      NOD() constexpr bool IsKeyTypeConstrained() const noexcept;
      NOD() constexpr bool IsValueTypeConstrained() const noexcept;
      NOD() constexpr bool IsKeyDeep() const noexcept;
      NOD() constexpr bool IsValueDeep() const noexcept;
      NOD() constexpr bool IsKeySparse() const noexcept;
      NOD() constexpr bool IsValueSparse() const noexcept;
      NOD() constexpr bool IsKeyDense() const noexcept;
      NOD() constexpr bool IsValueDense() const noexcept;
      NOD() constexpr Size GetKeyStride() const noexcept;
      NOD() constexpr Size GetValueStride() const noexcept;
      NOD() Count GetKeyCountDeep() const noexcept;
      NOD() Count GetKeyCountElementsDeep() const noexcept;
      NOD() Count GetValueCountDeep() const noexcept;
      NOD() Count GetValueCountElementsDeep() const noexcept;
      NOD() bool IsMissingDeep() const;

      ///                                                                     
      ///   Indexing                                                          
      ///                                                                     
      NOD() Block GetKey(CT::Index auto);
      NOD() Block GetKey(CT::Index auto) const;
      NOD() Block GetValue(CT::Index auto);
      NOD() Block GetValue(CT::Index auto) const;
      NOD() Pair  GetPair(CT::Index auto);
      NOD() Pair  GetPair(CT::Index auto) const;

      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      using Iterator = BlockMap::Iterator<Map>;
      using ConstIterator = BlockMap::Iterator<const Map>;

      NOD() Iterator begin() noexcept;
      NOD() Iterator last() noexcept;
      NOD() ConstIterator begin() const noexcept;
      NOD() ConstIterator last() const noexcept;

      template<bool REVERSE = false>
      Count ForEachElement(auto&&);
      template<bool REVERSE = false>
      Count ForEachElement(auto&&) const;
      
      template<bool REVERSE = false>
      Count ForEach(auto&&...);
      template<bool REVERSE = false>
      Count ForEach(auto&&...) const;
   
      template<bool REVERSE = false, bool SKIP = true>
      Count ForEachDeep(auto&&...);
      template<bool REVERSE = false, bool SKIP = true>
      Count ForEachDeep(auto&&...) const;
      
      Count ForEachElementRev(auto&&...);
      Count ForEachElementRev(auto&&...) const;

      Count ForEachRev(auto&&...);
      Count ForEachRev(auto&&...) const;

      template<bool SKIP = true>
      Count ForEachDeepRev(auto&&...);
      template<bool SKIP = true>
      Count ForEachDeepRev(auto&&...) const;

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Map& operator << (CT::Inner::UnfoldInsertable auto&&);
      Map& operator >> (CT::Inner::UnfoldInsertable auto&&);
   };

} // namespace Langulus::Anyness

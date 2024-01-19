///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../blocks/BlockMap.hpp"


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
      Count ForEach(auto&&) const;
      template<bool REVERSE = false>
      Count ForEach(auto&&);

      template<bool REVERSE = false>
      Count ForEachKeyElement(auto&&) const;
      template<bool REVERSE = false>
      Count ForEachKeyElement(auto&&);

      template<bool REVERSE = false>
      Count ForEachValueElement(auto&&) const;
      template<bool REVERSE = false>
      Count ForEachValueElement(auto&&);

      template<bool REVERSE = false>
      Count ForEachKey(auto&&...) const;
      template<bool REVERSE = false>
      Count ForEachKey(auto&&...);

      template<bool REVERSE = false>
      Count ForEachValue(auto&&...) const;
      template<bool REVERSE = false>
      Count ForEachValue(auto&&...);

      template<bool REVERSE = false, bool SKIP = true>
      Count ForEachKeyDeep(auto&&...) const;
      template<bool REVERSE = false, bool SKIP = true>
      Count ForEachKeyDeep(auto&&...);

      template<bool REVERSE = false, bool SKIP = true>
      Count ForEachValueDeep(auto&&...) const;
      template<bool REVERSE = false, bool SKIP = true>
      Count ForEachValueDeep(auto&&...);

      ///                                                                     
      ///   RTTI                                                              
      ///                                                                     
      template<CT::Data, CT::Data...>
      NOD() constexpr bool IsKey() const noexcept;
      NOD() bool IsKey(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool IsKeySimilar() const noexcept;
      NOD() bool IsKeySimilar(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool IsKeyExact() const noexcept;
      NOD() bool IsKeyExact(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool IsValue() const noexcept;
      NOD() bool IsValue(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool IsValueSimilar() const noexcept;
      NOD() bool IsValueSimilar(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool IsValueExact() const noexcept;
      NOD() bool IsValueExact(DMeta) const noexcept;

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      bool operator == (CT::Map  auto const&) const;
      bool operator == (CT::Pair auto const&) const;

      NOD() Hash GetHash() const;

      NOD() bool ContainsKey(const CT::NotSemantic auto&) const;
      NOD() bool ContainsValue(const CT::NotSemantic auto&) const;
      NOD() bool ContainsPair(const CT::Pair auto&) const;

      NOD() Index Find(const CT::NotSemantic auto&) const;
      NOD() Iterator FindIt(const CT::NotSemantic auto&);
      NOD() ConstIterator FindIt(const CT::NotSemantic auto&) const;

      NOD() decltype(auto) At(const CT::NotSemantic auto&);
      NOD() decltype(auto) At(const CT::NotSemantic auto&) const;

      NOD() decltype(auto) operator[] (const CT::NotSemantic auto&);
      NOD() decltype(auto) operator[] (const CT::NotSemantic auto&) const;

      ///                                                                     
      ///   Memory management                                                 
      ///                                                                     
      void Reserve(Count);

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Count Insert(auto&&, auto&&);

      template<class T1, class T2>
      requires CT::Block<Desem<T1>, Desem<T2>>
      Count InsertBlock(T1&&, T2&&);

      template<class T1, class...TAIL>
      Count InsertPair(T1&&, TAIL&&...);

      Map& operator << (CT::Inner::UnfoldInsertable auto&&);
      Map& operator >> (CT::Inner::UnfoldInsertable auto&&);

      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      Count RemoveKey(const CT::NotSemantic auto&);
      Count RemoveValue(const CT::NotSemantic auto&);
      Count RemovePair(const CT::Pair auto&);
      Iterator RemoveIt(const Iterator&);

      void Clear();
      void Reset();
      void Compact();
   };

} // namespace Langulus::Anyness

///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "../blocks/BlockMap.hpp"
#include "../pairs/TPair.hpp"


namespace Langulus::Anyness
{

   ///                                                                        
   ///   Type-erased map                                                      
   ///                                                                        
   template<bool ORDERED = false>
   struct Map : BlockMap {
      LANGULUS(POD) false;
      LANGULUS(ACT_AS) Map;
      LANGULUS_BASES(BlockMap);

      friend struct BlockMap;
      static constexpr bool Ownership = true;
      static constexpr bool Ordered = ORDERED;

      using Key = void;
      using Value = void;
      using Self = Map<ORDERED>;
      using Pair = Anyness::Pair;
      using PairRef = Pair;
      using PairConstRef = Pair;

      ///                                                                     
      ///   Construction                                                      
      ///                                                                     
      constexpr Map() noexcept = default;
      Map(const Map&);
      Map(Map&&);

      template<class T1, class...TN>
      requires CT::UnfoldInsertable<T1, TN...>
      Map(T1&&, TN&&...);

      ~Map();

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      auto operator = (const Map&) -> Map&;
      auto operator = (Map&&) -> Map&;
      auto operator = (CT::UnfoldInsertable auto&&) -> Map&;

      ///                                                                     
      ///   Indexing                                                          
      ///                                                                     
      NOD() auto GetKey  (CT::Index auto) -> Block<>;
      NOD() auto GetKey  (CT::Index auto) const -> Block<>;
      NOD() auto GetValue(CT::Index auto) -> Block<>;
      NOD() auto GetValue(CT::Index auto) const -> Block<>;
      NOD() auto GetPair (CT::Index auto) -> Pair;
      NOD() auto GetPair (CT::Index auto) const -> Pair;

      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      using Iterator      = BlockMap::Iterator<Map>;
      using ConstIterator = BlockMap::Iterator<const Map>;

      NOD() auto begin() noexcept -> Iterator;
      NOD() auto last() noexcept -> Iterator;
      NOD() auto begin() const noexcept -> ConstIterator;
      NOD() auto last() const noexcept -> ConstIterator;

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
      using BlockMap::operator ==;

      NOD() auto Find(const CT::NoIntent auto&) const -> Index;
      NOD() auto FindIt(const CT::NoIntent auto&) -> Iterator;
      NOD() auto FindIt(const CT::NoIntent auto&) const -> ConstIterator;

      NOD() decltype(auto) At(const CT::NoIntent auto&);
      NOD() decltype(auto) At(const CT::NoIntent auto&) const;

      NOD() decltype(auto) operator[] (const CT::NoIntent auto&);
      NOD() decltype(auto) operator[] (const CT::NoIntent auto&) const;

      ///                                                                     
      ///   Memory management                                                 
      ///                                                                     
      void Reserve(Count);

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Count Insert(auto&&, auto&&);

      template<class T1, class T2> requires CT::Block<Deint<T1>, Deint<T2>>
      Count InsertBlock(T1&&, T2&&);

      template<class T1, class...TN>
      Count InsertPair(T1&&, TN&&...);

      auto operator << (CT::UnfoldInsertable auto&&) -> Map&;
      auto operator >> (CT::UnfoldInsertable auto&&) -> Map&;

      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      auto RemoveKey(const CT::NoIntent auto&) -> Count;
      auto RemoveValue(const CT::NoIntent auto&) -> Count;
      auto RemovePair(const CT::Pair auto&) -> Count;
      auto RemoveIt(const Iterator&) -> Iterator;

      void Clear();
      void Reset();
      void Compact();
   };

} // namespace Langulus::Anyness

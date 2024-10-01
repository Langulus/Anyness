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
      Map& operator = (const Map&);
      Map& operator = (Map&&);
      Map& operator = (CT::UnfoldInsertable auto&&);

      ///                                                                     
      ///   Indexing                                                          
      ///                                                                     
      NOD() Block<> GetKey  (CT::Index auto);
      NOD() Block<> GetKey  (CT::Index auto) const;
      NOD() Block<> GetValue(CT::Index auto);
      NOD() Block<> GetValue(CT::Index auto) const;
      NOD() Pair    GetPair (CT::Index auto);
      NOD() Pair    GetPair (CT::Index auto) const;

      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      using Iterator      = BlockMap::Iterator<Map>;
      using ConstIterator = BlockMap::Iterator<const Map>;

      NOD() Iterator      begin() noexcept;
      NOD() Iterator      last() noexcept;
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
      using BlockMap::operator ==;

      NOD() Index Find(const CT::NoIntent auto&) const;
      NOD() Iterator FindIt(const CT::NoIntent auto&);
      NOD() ConstIterator FindIt(const CT::NoIntent auto&) const;

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

      Map& operator << (CT::UnfoldInsertable auto&&);
      Map& operator >> (CT::UnfoldInsertable auto&&);

      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      Count RemoveKey(const CT::NoIntent auto&);
      Count RemoveValue(const CT::NoIntent auto&);
      Count RemovePair(const CT::Pair auto&);
      Iterator RemoveIt(const Iterator&);

      void Clear();
      void Reset();
      void Compact();
   };

} // namespace Langulus::Anyness

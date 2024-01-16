///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "blocks/Block.hpp"


namespace Langulus::Anyness
{
   
   ///                                                                        
   ///   Any                                                                  
   ///                                                                        
   ///   More of an equivalent to std::vector, instead of std::any, since     
   /// it can contain any number of similarly-typed type-erased elements.     
   /// It gracefully wraps sparse and dense arrays, keeping track of static   
   /// and constant data blocks.                                              
   ///   For a faster statically-optimized equivalent of this, use TAny       
   ///   You can always ReinterpretAs a statically optimized equivalent for   
   /// the cost of one runtime type check, because all Any variants are       
   /// binary-compatible.                                                     
   ///                                                                        
   class Any : public Block {
      LANGULUS(POD) false;
      LANGULUS_BASES(Block);

   protected: IF_LANGULUS_TESTING(public:)
      #if LANGULUS_DEBUG()
         using Block::mRawChar;
      #endif

      using Block::mRaw;
      using Block::mRawSparse;
      using Block::mState;
      using Block::mCount;
      using Block::mReserved;
      using Block::mType;
      using Block::mEntry;

   public:
      static constexpr bool Ownership = true;

      ///                                                                     
      ///   Construction                                                      
      ///                                                                     
      constexpr Any() noexcept = default;
      Any(const Any&);
      Any(Any&&) noexcept;

      template<class T1, class...TAIL>
      requires CT::Inner::UnfoldInsertable<T1, TAIL...>
      Any(T1&&, TAIL&&...);

      ~Any();

      NOD() static Any FromMeta(DMeta, DataState = {}) noexcept;
      NOD() static Any FromBlock(const CT::Block auto&, DataState = {}) noexcept;
      NOD() static Any FromState(const CT::Block auto&, DataState = {}) noexcept;
      template<CT::Data T>
      NOD() static Any From(DataState = {}) noexcept;

      template<class AS = void, CT::Data T1, CT::Data T2, CT::Data... TN>
      NOD() static Any WrapAs(T1&&, T2&&, TN&&...);

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      Any& operator = (const Any&);
      Any& operator = (Any&&) noexcept;
      Any& operator = (CT::Inner::UnfoldInsertable auto&&);

   public:
      NOD() Any Crop(Offset, Count) const;
      NOD() Any Crop(Offset, Count);

      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      using Iterator = Block::Iterator<Any>;
      using ConstIterator = Block::Iterator<const Any>;

      template<bool REVERSE = false>
      Count ForEachElement(auto&&) const;
      template<bool REVERSE = false>
      Count ForEachElement(auto&&);

      template<bool REVERSE = false>
      Count ForEach(auto&&...) const;
      template<bool REVERSE = false>
      Count ForEach(auto&&...);

      template<bool REVERSE = false, bool SKIP = true>
      Count ForEachDeep(auto&&...) const;
      template<bool REVERSE = false, bool SKIP = true>
      Count ForEachDeep(auto&&...);

      ///                                                                     
      ///   RTTI                                                              
      ///                                                                     
      template<CT::Data, CT::Data...>
      NOD() constexpr bool Is() const noexcept;
      NOD() bool Is(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool IsSimilar() const noexcept;
      NOD() bool IsSimilar(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool IsExact() const noexcept;
      NOD() bool IsExact(DMeta) const noexcept;

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      using Block::operator==;

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Any& operator <<  (CT::Inner::UnfoldInsertable auto&&);
      Any& operator >>  (CT::Inner::UnfoldInsertable auto&&);

      Any& operator <<= (CT::Inner::UnfoldInsertable auto&&);
      Any& operator >>= (CT::Inner::UnfoldInsertable auto&&);

      ///                                                                     
      ///   Concatenation                                                     
      ///                                                                     
      NOD() Any  operator +  (CT::Inner::UnfoldInsertable auto&&) const;
            Any& operator += (CT::Inner::UnfoldInsertable auto&&);
   };

} // namespace Langulus::Anyness
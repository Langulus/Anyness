///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../blocks/Block.hpp"


namespace Langulus::Anyness
{
   
   ///                                                                        
   ///   Many                                                                 
   ///                                                                        
   ///   Equivalent to an std::vector - it can contain any number of          
   /// similarly-typed type-erased elements. It gracefully wraps sparse and   
   /// dense arrays, keeping track of static and constant data blocks.        
   ///   For a faster statically-optimized equivalent of this, use TMany      
   ///   You can always ReinterpretAs a statically optimized equivalent for   
   /// the cost of one runtime type check, because all Many variants are      
   /// binary-compatible.                                                     
   ///                                                                        
   class Many : public Block<> {
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
      constexpr Many() noexcept = default;
      Many(const Many&);
      Many(Many&&) noexcept;

      template<class T1, class...TN>
      requires CT::UnfoldInsertable<T1, TN...>
      Many(T1&&, TN&&...);

      ~Many();

      NOD() static Many FromMeta(DMeta, DataState = {}) noexcept;
      NOD() static Many FromBlock(const CT::Block auto&, DataState = {}) noexcept;
      NOD() static Many FromState(const CT::Block auto&, DataState = {}) noexcept;
      template<CT::Data T>
      NOD() static Many From(DataState = {}) noexcept;

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      Many& operator = (const Many&);
      Many& operator = (Many&&) noexcept;
      Many& operator = (CT::UnfoldInsertable auto&&);

   public:
      NOD() Many Crop(Offset, Count) const;
      NOD() Many Crop(Offset, Count);

      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      using Iterator = Block::Iterator<Many>;
      using ConstIterator = Block::Iterator<const Many>;

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
      Many& operator <<  (CT::UnfoldInsertable auto&&);
      Many& operator >>  (CT::UnfoldInsertable auto&&);

      Many& operator <<= (CT::UnfoldInsertable auto&&);
      Many& operator >>= (CT::UnfoldInsertable auto&&);

      ///                                                                     
      ///   Concatenation                                                     
      ///                                                                     
      NOD() Many  operator +  (CT::UnfoldInsertable auto&&) const;
            Many& operator += (CT::UnfoldInsertable auto&&);
   };

} // namespace Langulus::Anyness
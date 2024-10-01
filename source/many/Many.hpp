///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
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
      using Base = Block<>;
      LANGULUS(POD) false;
      LANGULUS(ACT_AS) Many;
      LANGULUS_BASES(Base);

   protected:
	   template<class>
	   friend struct Block;
	   friend struct BlockSet;
	   friend struct BlockMap;
	   template<CT::Data>
	   friend class THive;
	   
	   #if LANGULUS_DEBUG()
         using Base::mRawChar;
      #endif

      using Base::mRaw;
      using Base::mRawSparse;
      using Base::mState;
      using Base::mCount;
      using Base::mReserved;
      using Base::mType;
      using Base::mEntry;

   public:
      static constexpr bool Ownership = true;

      ///                                                                     
      ///   Construction                                                      
      ///                                                                     
      constexpr Many() noexcept = default;
      Many(const Many&);
      Many(Many&&) noexcept;

      template<class T1, class...TN> requires CT::UnfoldInsertable<T1, TN...>
      Many(T1&&, TN&&...);

      ~Many();

      NOD() static Many FromMeta(DMeta, DataState = {}) noexcept;
      NOD() static Many FromBlock(const CT::Block auto&, DataState = {}) noexcept;
      NOD() static Many FromState(const CT::Block auto&, DataState = {}) noexcept;
      template<CT::Data T>
      NOD() static Many From(DataState = {}) noexcept;
      template<class AS = void, CT::Data...TN>
      NOD() static Many Wrap(TN&&...);
      NOD() static Many Past() noexcept;
      NOD() static Many Future() noexcept;

      #if LANGULUS(DEBUG)
         using Base::TrackingReport;
      #endif

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      Many& operator = (const Many&);
      Many& operator = (Many&&) noexcept;
      Many& operator = (CT::UnfoldInsertable auto&&);

      ///                                                                     
      ///   Indexing                                                          
      ///                                                                     
      NOD() Many Select(Offset, Count) const IF_UNSAFE(noexcept);
      NOD() Many Select(Offset, Count) IF_UNSAFE(noexcept);

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
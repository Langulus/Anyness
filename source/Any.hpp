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

      template<CT::Data T>
      friend class TAny;
      friend class Block;

      template<bool MUTABLE>
      struct TIterator;

      using Iterator = TIterator<true>;
      using ConstIterator = TIterator<false>;

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
      NOD() static Any FromBlock(const Block&, DataState = {}) noexcept;
      NOD() static Any FromState(const Block&, DataState = {}) noexcept;
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

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      using Block::operator ==;

      template<bool REVERSE = false>
      Index Find(const CT::Data auto&, const Offset & = 0) const;

   public:
      using Block::Swap;
      void Swap(Any&) noexcept;

      NOD() Any Crop(const Offset&, const Count&) const;
      NOD() Any Crop(const Offset&, const Count&);

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Any& operator <<  (CT::Inner::UnfoldInsertable auto&&);
      Any& operator >>  (CT::Inner::UnfoldInsertable auto&&);

      Any& operator <<= (CT::Inner::UnfoldInsertable auto&&);
      Any& operator >>= (CT::Inner::UnfoldInsertable auto&&);

      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      using Block::RemoveIndex;
      Iterator RemoveIt(const Iterator&, Count = 1);

      void Clear();
      void Reset();

      ///                                                                     
      ///   Concatenation                                                     
      ///                                                                     
      NOD() Any  operator +  (CT::Inner::UnfoldInsertable auto&&) const;
            Any& operator += (CT::Inner::UnfoldInsertable auto&&);

      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      NOD() Iterator begin() noexcept;
      NOD() Iterator end() noexcept;
      NOD() Iterator last() noexcept;
      NOD() ConstIterator begin() const noexcept;
      NOD() ConstIterator end() const noexcept;
      NOD() ConstIterator last() const noexcept;
   };


   ///                                                                        
   ///   Block iterator                                                       
   ///                                                                        
   template<bool MUTABLE>
   struct Any::TIterator {
   protected:
      friend class Any;

      Block mValue;

      TIterator(const Block&) noexcept;

   public:
      TIterator() noexcept = default;
      TIterator(const TIterator&) noexcept = default;
      TIterator(TIterator&&) noexcept = default;

      NOD() bool operator == (const TIterator&) const noexcept;

      NOD() const Block& operator * () const noexcept;

      // Prefix operator                                                
      TIterator& operator ++ () noexcept;

      // Suffix operator                                                
      NOD() TIterator operator ++ (int) noexcept;
   };

} // namespace Langulus::Anyness
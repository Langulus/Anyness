///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
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
      LANGULUS(DEEP) true;
      LANGULUS(UNINSERTABLE) false;
      LANGULUS(POD) false;
      LANGULUS_BASES(Block);

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

      Any(const CT::NotSemantic auto&);
      Any(CT::NotSemantic auto&);
      Any(CT::NotSemantic auto&&);
      Any(CT::Semantic auto&&) noexcept;

      template<CT::Data T1, CT::Data T2, CT::Data... TN>
      Any(T1&&, T2&&, TN&&...);

      ~Any();
   
      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      Any& operator = (const Any&);
      Any& operator = (Any&&) noexcept;

      Any& operator = (const CT::NotSemantic auto&);
      Any& operator = (CT::NotSemantic auto&);
      Any& operator = (CT::NotSemantic auto&&);
      Any& operator = (CT::Semantic auto&&);

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      using Block::operator ==;

   protected:
      void CreateFrom(CT::Semantic auto&&) noexcept;

   public:
      NOD() static Any FromMeta(DMeta, const DataState& = {}) noexcept;
      NOD() static Any FromBlock(const Block&, const DataState& = {}) noexcept;
      NOD() static Any FromState(const Block&, const DataState& = {}) noexcept;
      template<CT::Data T>
      NOD() static Any From(const DataState& = {}) noexcept;

      template<class AS = void, CT::Data T1, CT::Data T2, CT::Data... TN>
      NOD() static Any WrapAs(T1&&, T2&&, TN&&...);

      void Clear();
      void Reset();

      template<bool REVERSE = false, CT::Data T>
      Index Find(const T&, const Offset& = 0) const;

      using Block::Swap;
      void Swap(Any&) noexcept;

      NOD() Any Crop(const Offset&, const Count&) const;
      NOD() Any Crop(const Offset&, const Count&);

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Any& operator << (const CT::NotSemantic auto&);
      Any& operator << (CT::NotSemantic auto&);
      Any& operator << (CT::NotSemantic auto&&);
      Any& operator << (CT::Semantic auto&&);
   
      Any& operator >> (const CT::NotSemantic auto&);
      Any& operator >> (CT::NotSemantic auto&);
      Any& operator >> (CT::NotSemantic auto&&);
      Any& operator >> (CT::Semantic auto&&);

      Any& operator <<= (const CT::NotSemantic auto&);
      Any& operator <<= (CT::NotSemantic auto&);
      Any& operator <<= (CT::NotSemantic auto&&);
      Any& operator <<= (CT::Semantic auto&&);

      Any& operator >>= (const CT::NotSemantic auto&);
      Any& operator >>= (CT::NotSemantic auto&);
      Any& operator >>= (CT::NotSemantic auto&&);
      Any& operator >>= (CT::Semantic auto&&);

      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      using Block::RemoveIndex;
      Iterator RemoveIndex(const Iterator&, Count = 1);

      ///                                                                     
      ///   Concatenation                                                     
      ///                                                                     
      NOD() Any operator + (const CT::Deep auto&) const;
      NOD() Any operator + (CT::Deep auto&) const;
      NOD() Any operator + (CT::Deep auto&&) const;
      NOD() Any operator + (CT::Semantic auto&&) const;

      Any& operator += (const CT::Deep auto&);
      Any& operator += (CT::Deep auto&);
      Any& operator += (CT::Deep auto&&);
      Any& operator += (CT::Semantic auto&&);

      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      NOD() Iterator begin() noexcept;
      NOD() Iterator end() noexcept;
      NOD() Iterator last() noexcept;
      NOD() ConstIterator begin() const noexcept;
      NOD() ConstIterator end() const noexcept;
      NOD() ConstIterator last() const noexcept;

   protected:
      template<CT::Block WRAPPER, CT::Semantic S>
      WRAPPER Concatenate(S&&) const;
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
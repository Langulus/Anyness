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
      LANGULUS_BASES(Block);
   public:
      static constexpr bool Ownership = true;

      template<CT::Data T>
      friend class TAny;
      friend class Block;

      ///                                                                     
      ///   Construction                                                      
      ///                                                                     
      constexpr Any() noexcept = default;

      Any(const Any&);
      Any(Any&&) noexcept;

      template<CT::Deep T>
      Any(const T&);
      template<CT::Deep T>
      Any(T&);
      template<CT::Deep T>
      Any(T&&) requires CT::Mutable<T>;
      template<CT::Semantic S>
      Any(S&& other) noexcept requires (CT::Deep<TypeOf<S>>);

      template<CT::CustomData T>
      Any(const T&);
      template<CT::CustomData T>
      Any(T&);
      template<CT::CustomData T>
      Any(T&&) requires CT::Mutable<T>;
      template<CT::Semantic S>
      Any(S&&) requires (CT::CustomData<TypeOf<S>>);

      template<CT::Data HEAD, CT::Data... TAIL>
      Any(HEAD&&, TAIL&&...) requires (sizeof...(TAIL) >= 1);

      ~Any();
   
      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      Any& operator = (const Any&);
      Any& operator = (Any&&) noexcept;

      template<CT::Deep T>
      Any& operator = (const T&);
      template<CT::Deep T>
      Any& operator = (T&);
      template<CT::Deep T>
      Any& operator = (T&&) requires CT::Mutable<T>;
   
      template<CT::CustomData T>
      Any& operator = (const T&);
      template<CT::CustomData T>
      Any& operator = (T&);
      template<CT::CustomData T>
      Any& operator = (T&&) requires CT::Mutable<T>;

      template<CT::Semantic S>
      Any& operator = (S&&);

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      using Block::operator ==;

   public:
      NOD() static Any FromMeta(DMeta, const DataState& = {}) noexcept;
      NOD() static Any FromBlock(const Block&, const DataState& = {}) noexcept;
      NOD() static Any FromState(const Block&, const DataState& = {}) noexcept;
      template<CT::Data T>
      NOD() static Any From(const DataState& = {}) noexcept;

      template<CT::Data... LIST>
      NOD() static Any Wrap(LIST&&...);
      template<class AS = void, CT::Data HEAD, CT::Data... TAIL>
      NOD() static Any WrapAs(HEAD&&, TAIL&&...);

      void Clear();
      void Reset();
      NOD() Any Clone() const;

      template<bool REVERSE = false, bool BY_ADDRESS_ONLY = false, CT::Data T>
      Index Find(const T&, const Offset& = 0) const;

      using Block::Swap;
      void Swap(Any&) noexcept;

      NOD() Any Crop(const Offset&, const Count&) const;
      NOD() Any Crop(const Offset&, const Count&);

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      template<CT::NotSemantic T>
      Any& operator << (const T&);
      template<CT::NotSemantic T>
      Any& operator << (T&);
      template<CT::NotSemantic T>
      Any& operator << (T&&);
      template<CT::Semantic S>
      Any& operator << (S&&);
   
      template<CT::NotSemantic T>
      Any& operator >> (const T&);
      template<CT::NotSemantic T>
      Any& operator >> (T&);
      template<CT::NotSemantic T>
      Any& operator >> (T&&);
      template<CT::Semantic S>
      Any& operator >> (S&&);

      template<CT::NotSemantic T>
      Any& operator <<= (const T&);
      template<CT::NotSemantic T>
      Any& operator <<= (T&);
      template<CT::NotSemantic T>
      Any& operator <<= (T&&);
      template<CT::Semantic S>
      Any& operator <<= (S&&);

      template<CT::NotSemantic T>
      Any& operator >>= (const T&);
      template<CT::NotSemantic T>
      Any& operator >>= (T&);
      template<CT::NotSemantic T>
      Any& operator >>= (T&&);
      template<CT::Semantic S>
      Any& operator >>= (S&&);

      ///                                                                     
      ///   Concatenation                                                     
      ///                                                                     
      template<CT::Deep T>
      NOD() Any operator + (const T&) const requires CT::Dense<T>;
      template<CT::Deep T>
      NOD() Any operator + (T&) const requires CT::Dense<T>;
      template<CT::Deep T>
      NOD() Any operator + (T&&) const requires CT::Dense<T>;
      template<CT::Semantic S>
      NOD() Any operator + (S&&) const requires (CT::Deep<TypeOf<S>> && CT::Dense<TypeOf<S>>);

      template<CT::Deep T>
      Any& operator += (const T&) requires CT::Dense<T>;
      template<CT::Deep T>
      Any& operator += (T&) requires CT::Dense<T>;
      template<CT::Deep T>
      Any& operator += (T&&) requires CT::Dense<T>;
      template<CT::Semantic S>
      Any& operator += (S&&) requires (CT::Deep<TypeOf<S>>&& CT::Dense<TypeOf<S>>);

      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      template<bool MUTABLE>
      struct TIterator;

      using Iterator = TIterator<true>;
      using ConstIterator = TIterator<false>;

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

#include "Any.inl"

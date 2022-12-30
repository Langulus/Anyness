///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Text.hpp"

namespace Langulus::Anyness
{

   ///                                                                        
   ///   An owned value, dense or sparse                                      
   ///                                                                        
   /// Provides ownership and MADCC, for when you need to cleanup after a     
   /// move. By default, fundamental types and pointers are not reset after   
   /// a move. Wrapping them inside this ensures they are.                    
   ///                                                                        
   template<CT::Data T>
   class TOwned {
   protected:
      T mValue {};

      static constexpr bool Trivial
         = CT::Fundamental<T> || CT::Sparse<T> || CT::POD<T>;

      template<CT::Semantic S, class F>
      static constexpr bool SemanticMember
         = CT::Dense<T> && CT::Exact<TypeOf<S>, F> && CT::SemanticMakable<S, T>;

      template<CT::Semantic S, class F>
      static constexpr bool TrivialMember
         = Trivial && CT::Exact<TypeOf<S>, F>;

   public:
      static constexpr bool Ownership = true;

      /// Makes TOwned CT::Typed                                              
      using MemberType = T;

      constexpr TOwned() noexcept = default;
      constexpr TOwned(const TOwned&);
      constexpr TOwned(TOwned&&);

      template<CT::Semantic S>
      constexpr TOwned(S&& value) requires (SemanticMember<S, TOwned>)
         : mValue {S::Nest(value.mValue.mValue)} {
         if constexpr (S::Move && S::Keep)
            value.mValue.mValue = {};
      }

      template<CT::Semantic S>
      constexpr TOwned(S&& value) requires (TrivialMember<S, TOwned>)
         : mValue {value.mValue.mValue} {
         if constexpr (S::Move && S::Keep)
            value.mValue.mValue = {};
      }

      constexpr TOwned(const T&);
      constexpr TOwned(T&&);

      template<CT::Semantic S>
      constexpr TOwned(S&& value) requires (SemanticMember<S, T>)
         : mValue {S::Nest(value.mValue)} {}

      template<CT::Semantic S>
      constexpr TOwned(S&& value) requires (TrivialMember<S, T>)
         : mValue {value.mValue} {}

      NOD() DMeta GetType() const;

      /// Makes TOwned CT::Resolvable                                         
      NOD() Block GetBlock() const;

      void Reset();

      constexpr TOwned& operator = (const TOwned&) noexcept;
      constexpr TOwned& operator = (TOwned&&) noexcept;

      template<CT::Semantic S>
      constexpr TOwned& operator = (S&& rhs) noexcept requires (CT::Exact<TypeOf<S>, TOwned>) {
         SemanticAssign(mValue, S::Nest(rhs.mValue.mValue));
         if constexpr (S::Move && S::Keep)
            rhs.mValue.mValue = {};
         return *this;
      }

      constexpr TOwned& operator = (const T&) noexcept;
      constexpr TOwned& operator = (T&&) noexcept;

      template<CT::Semantic S>
      constexpr TOwned& operator = (S&& rhs) noexcept requires (CT::Exact<TypeOf<S>, T>) {
         SemanticAssign(mValue, S::Nest(rhs.mValue));
         return *this;
      }

      NOD() Hash GetHash() const requires CT::Hashable<T>;

      NOD() decltype(auto) Get() const noexcept;
      NOD() decltype(auto) Get() noexcept;

      template<class>
      NOD() auto As() const noexcept requires CT::Sparse<T>;

      NOD() auto operator -> () const requires CT::Sparse<T>;
      NOD() auto operator -> () requires CT::Sparse<T>;
      NOD() decltype(auto) operator * () const requires CT::Sparse<T>;
      NOD() decltype(auto) operator * () requires CT::Sparse<T>;

      NOD() explicit operator bool() const noexcept;
      NOD() explicit operator const T&() const noexcept;
      NOD() explicit operator T&() noexcept;

      NOD() bool operator == (const TOwned&) const noexcept;
      NOD() bool operator == (const T&) const noexcept;
      NOD() bool operator == (::std::nullptr_t) const noexcept requires CT::Sparse<T>;
   };

   /// Just a short handle for value with ownership                           
   /// If sparse/fundamental, value will be explicitly nulled after a move    
   template<class T>
   using Own = TOwned<T>;

} // namespace Langulus::Anyness

#include "TOwned.inl"

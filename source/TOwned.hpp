///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Text.hpp"

namespace Langulus::A
{
   struct Owned {};
}

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
   class TOwned : public A::Owned {
   protected:
      T mValue {};

   public:
      LANGULUS(TYPED) T;

      static constexpr bool Ownership = true;

      constexpr TOwned() noexcept = default;
      constexpr TOwned(const TOwned&);
      constexpr TOwned(TOwned&&);

      constexpr TOwned(const CT::NotSemantic auto&);
      constexpr TOwned(CT::NotSemantic auto&);
      constexpr TOwned(CT::NotSemantic auto&&);

      template<CT::Semantic S>
      constexpr TOwned(S&&);

      NOD() DMeta GetType() const;

      /// Makes TOwned CT::Resolvable                                         
      NOD() Block GetBlock() const;

      void Reset();

      constexpr TOwned& operator = (const TOwned&);
      constexpr TOwned& operator = (TOwned&&);

      constexpr TOwned& operator = (const CT::NotSemantic auto&);
      constexpr TOwned& operator = (CT::NotSemantic auto&);
      constexpr TOwned& operator = (CT::NotSemantic auto&&);

      template<CT::Semantic S>
      constexpr TOwned& operator = (S&&);

      NOD() Hash GetHash() const;

      NOD() const T& Get() const noexcept;
      NOD() T& Get() noexcept;

      template<class>
      NOD() auto As() const noexcept requires CT::Sparse<T>;

      NOD() T operator -> () const requires CT::Sparse<T>;
      NOD() T operator -> () requires CT::Sparse<T>;
      NOD() decltype(auto) operator * () const requires CT::Sparse<T>;
      NOD() decltype(auto) operator * () requires CT::Sparse<T>;

      NOD() explicit operator bool() const noexcept;
      NOD() operator const T&() const noexcept;
      NOD() operator T&() noexcept;

      NOD() bool operator == (const TOwned&) const noexcept;
      NOD() bool operator == (const T&) const noexcept;
      NOD() bool operator == (::std::nullptr_t) const noexcept requires CT::Sparse<T>;
   };

   /// Just a short handle for value with ownership                           
   /// If sparse/fundamental, value will be explicitly nulled after a move    
   template<class T>
   using Own = TOwned<T>;

} // namespace Langulus::Anyness

namespace Langulus::CT
{

   template<class... T>
   concept Owned = (DerivedFrom<T, A::Owned> && ...);

} // namespace Langulus::CT

#include "TOwned.inl"

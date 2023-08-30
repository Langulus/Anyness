///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Text.hpp"

namespace Langulus
{
   namespace A
   {
      /// An abstract owned value                                             
      struct Owned {
         LANGULUS(ABSTRACT) true;
      };
   }

   namespace CT
   {
      /// Anything derived from A::Owned                                      
      template<class... T>
      concept Owned = (DerivedFrom<T, A::Owned> && ...);
   }

} // namespace Langulus

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
      LANGULUS(ABSTRACT) false;
      LANGULUS(TYPED) T;

      static constexpr bool Ownership = true;

      constexpr TOwned() noexcept = default;
      constexpr TOwned(const TOwned&);
      constexpr TOwned(TOwned&&);

      constexpr TOwned(const CT::NotSemantic auto&);
      constexpr TOwned(CT::NotSemantic auto&);
      constexpr TOwned(CT::NotSemantic auto&&);
      constexpr TOwned(CT::Semantic auto&&);

      NOD() DMeta GetType() const;

      /// Makes TOwned CT::Resolvable                                         
      NOD() Block GetBlock() const;

      void Reset();

      constexpr TOwned& operator = (const TOwned&);
      constexpr TOwned& operator = (TOwned&&);

      constexpr TOwned& operator = (const CT::NotSemantic auto&);
      constexpr TOwned& operator = (CT::NotSemantic auto&);
      constexpr TOwned& operator = (CT::NotSemantic auto&&);
      constexpr TOwned& operator = (CT::Semantic auto&&);

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

      NOD() bool operator == (const T&) const noexcept requires CT::Inner::Comparable<T>;
      NOD() bool operator == (::std::nullptr_t) const noexcept requires CT::Sparse<T>;
   };

   /// Just a short handle for value with ownership                           
   /// If sparse/fundamental, value will be explicitly nulled after a move    
   template<class T>
   using Own = TOwned<T>;

} // namespace Langulus::Anyness

namespace fmt
{

   ///                                                                        
   /// Extend FMT to be capable of logging any owned values                   
   ///                                                                        
   template<Langulus::CT::Owned T>
   struct formatter<T> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(T const& element, CONTEXT& ctx) {
         using namespace Langulus;

         if constexpr (CT::Sparse<TypeOf<T>>) {
            if (element == nullptr) {
               const auto type = element.GetType();
               if (type)
                  return fmt::format_to(ctx.out(), "{}(null)", *type);
               else
                  return fmt::format_to(ctx.out(), "null");
            }
            else return fmt::format_to(ctx.out(), "{}", *element.Get());
         }
         else {
            static_assert(CT::Dense<decltype(element.Get())>,
               "T not dense, but not sparse either????");
            return fmt::format_to(ctx.out(), "{}", element.Get());
         }
      }
   };

} // namespace fmt
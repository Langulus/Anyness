///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
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
      concept Owned = (DerivedFrom<T, A::Owned> and ...);

      /// Anything not derived from A::Owned                                  
      template<class... T>
      concept NotOwned = CT::Data<T...> and not Owned<T...>;

      /// Any owned pointer                                                   
      template<class... T>
      concept Pointer = Owned<T...> and Sparse<TypeOf<T>...>;

      /// Anything usable to initialize a shared pointer                      
      template<class... T>
      concept PointerRelated = ((Pointer<T> or Sparse<T> or Nullptr<T>) and ...);
   }

} // namespace Langulus

namespace Langulus::Anyness
{

   ///                                                                        
   ///   An owned value, dense or sparse                                      
   ///                                                                        
   /// Provides ownership and semantics, for when you need to cleanup after a 
   /// move, for example. By default, fundamental types are not reset after a 
   /// move - wrapping them inside this ensures they are.                     
   ///   @attention this container is suboptimal for pointers, because it     
   ///              will constantly search the allocation corresponding to    
   ///              them, as it doesn't cache it in order to minimize size.   
   ///              For pointers, use Ptr or Ref instead. This doesn't really 
   ///              matter, if built without the MANAGED_MEMORY feature       
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
      TOwned(CT::ShallowSemantic auto&&);
      TOwned(CT::DeepSemantic auto&&) requires CT::CloneMakable<T>;

      NOD() DMeta GetType() const;

      /// Makes TOwned CT::Resolvable                                         
      NOD() Block GetBlock() const;

      void Reset();

      constexpr TOwned& operator = (const TOwned&);
      constexpr TOwned& operator = (TOwned&&);

      constexpr TOwned& operator = (const CT::NotSemantic auto&);
      constexpr TOwned& operator = (CT::NotSemantic auto&);
      constexpr TOwned& operator = (CT::NotSemantic auto&&);
      TOwned& operator = (CT::ShallowSemantic auto&&);
      TOwned& operator = (CT::DeepSemantic auto&&) requires CT::CloneAssignable<T>;

      NOD() Hash GetHash() const requires CT::Hashable<T>;

      NOD() const T& Get() const noexcept;
      NOD() T& Get() noexcept;

      template<class>
      NOD() auto As() const noexcept requires CT::Sparse<T>;

      NOD() auto operator -> () const;
      NOD() auto operator -> ();
      NOD() const T& operator * () const; 
      NOD()       T& operator * ();

      NOD() explicit operator bool() const noexcept;
      NOD() explicit operator const T&() const noexcept;
      NOD() explicit operator T&() noexcept;

   private:
      void ConstructFrom(CT::Semantic auto&&);
      TOwned& AssignFrom(CT::Semantic auto&&);
   };

   /// Just a short handle for value with ownership                           
   /// If sparse/fundamental, value will be explicitly nulled after a move    
   template<class T>
   using Own = TOwned<T>;

   template<CT::Data T1, CT::Data T2>
   LANGULUS(INLINED)
   bool operator == (const TOwned<T1>& lhs, const TOwned<T2>& rhs) noexcept requires CT::Inner::Comparable<T1, T2> {
      return *lhs == *rhs;
   }

   template<CT::Data T1, CT::NotOwned T2>
   LANGULUS(INLINED)
   bool operator == (const TOwned<T1>& lhs, const T2& rhs) noexcept requires CT::Inner::Comparable<T1, T2> {
      return *lhs == rhs;
   }

   template<CT::Data T1, CT::NotOwned T2>
   LANGULUS(INLINED)
   bool operator == (const T2& lhs, const TOwned<T1>& rhs) noexcept requires CT::Inner::Comparable<T2, T1> {
      return lhs == *rhs;
   }

   template<CT::Data T1, CT::Data T2>
   LANGULUS(INLINED)
   bool operator == (const TOwned<T1>& lhs, ::std::nullptr_t) noexcept requires CT::Sparse<T1> {
      return *lhs == nullptr;
   }

   template<CT::Data T1, CT::Data T2>
   LANGULUS(INLINED)
   bool operator == (::std::nullptr_t, const TOwned<T1>& rhs) noexcept requires CT::Sparse<T1> {
      return *rhs == nullptr;
   }

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
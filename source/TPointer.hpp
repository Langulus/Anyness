///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TOwned.hpp"
#include "inner/Handle.hpp"

namespace Langulus
{
   namespace A
   {
      /// An abstract shared pointer                                          
      struct Pointer : Owned {
         LANGULUS_BASES(Owned);
      };
   }

   namespace CT
   {
      /// Anything derived from A::Pointer                                    
      template<class... T>
      concept Pointer = (DerivedFrom<T, A::Pointer> && ...);

      /// Anything usable to initialize a shared pointer                      
      template<class... T>
      concept PointerRelated = ((Pointer<T> || Sparse<T> || Nullptr<T>) && ...);
   }

} // namespace Langulus

namespace Langulus::Anyness
{

   ///                                                                        
   ///   A shared pointer                                                     
   ///                                                                        
   /// Provides ownership and referencing. Also, for single-element           
   /// containment, it is a bit more efficient than TAny. So, essentially     
   /// it's equivalent to std::shared_ptr                                     
   ///                                                                        
   template<class T, bool DR>
   class TPointer 
      : public A::Pointer
      , public TOwned<Conditional<CT::Constant<T>, const T*, T*>> {
      using Base = TOwned<Conditional<CT::Constant<T>, const T*, T*>>;
      using Self = TPointer<T, DR>;
      using Type = TypeOf<Base>;

   protected:
      using Base::mValue;
      Allocation* mEntry {};
   
      void ResetInner();

   public:
      LANGULUS(ABSTRACT) false;
      LANGULUS(TYPED) Type;

      constexpr TPointer() noexcept = default;

      TPointer(const TPointer&);
      TPointer(TPointer&&);

      TPointer(const CT::PointerRelated auto&);
      TPointer(CT::PointerRelated auto&);
      TPointer(CT::PointerRelated auto&&);
      TPointer(CT::Semantic auto&&);

      ~TPointer();

      NOD() Block GetBlock() const;
      NOD() auto GetHandle() const;
      NOD() constexpr bool HasAuthority() const noexcept;
      NOD() constexpr Count GetUses() const noexcept;

      template<class... ARGS>
      void New(ARGS&&...);

      void Reset();

      TPointer& operator = (const TPointer&);
      TPointer& operator = (TPointer&&);

      TPointer& operator = (const CT::PointerRelated auto&);
      TPointer& operator = (CT::PointerRelated auto&);
      TPointer& operator = (CT::PointerRelated auto&&);
      TPointer& operator = (CT::Semantic auto&&);

      NOD() operator TPointer<const T, DR>() const noexcept requires CT::Mutable<T>;

      using Base::operator bool;
      using Base::operator ->;
      using Base::operator *;

      template<class ALT_T, bool ALT_DR>
      NOD() bool operator == (const TPointer<ALT_T, ALT_DR>&) const noexcept requires (CT::Inner::Comparable<T*, ALT_T*>);
      NOD() bool operator == (::std::nullptr_t) const noexcept;
   };

   /// A shared pointer, that provides ownership and basic reference counting 
   /// Referencing comes from the block of memory that the pointer points to  
   template<class T>
   using Ptr = TPointer<T, false>;

   /// A shared pointer, that provides ownership and more reference counting  
   /// Referencing comes first from the block of memory that the pointer      
   /// points to, and second - the instance's individual reference counter    
   /// Useful for keeping track not only of the memory, but of the individual 
   /// element inside the memory block. Used to keep track of elements inside 
   /// THive and Hive (component factories for example)                       
   template<class T>
   using Ref = TPointer<T, true>;

   /// Get value behind Ptr/Ref/Own                                           
   ///   @param value - the container to decay                                
   ///   @return the contained value                                          
   LANGULUS(INLINED)
   decltype(auto) PointerDecay(const CT::Pointer auto& value) {
      using T = TypeOf<Decay<decltype(value)>>;
      return static_cast<const T&>(value);
   }
   
   /// Get value behind Ptr/Ref/Own                                           
   ///   @param value - the container to decay                                
   ///   @return the contained value                                          
   LANGULUS(INLINED)
   decltype(auto) PointerDecay(const CT::Sparse auto& value) {
      return value;
   }

} // namespace Langulus::Anyness

#include "TPointer.inl"

namespace fmt
{
   
   ///                                                                        
   /// Extend FMT to be capable of logging any shared pointers                
   ///                                                                        
   template<Langulus::CT::Pointer T>
   struct formatter<T> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(T const& element, CONTEXT& ctx) {
         if (element == nullptr) {
            const auto type = element.GetType();
            if (type)
               return fmt::format_to(ctx.out(), "{}(null)", *type);
            else
               return fmt::format_to(ctx.out(), "null");
         }
         else return fmt::format_to(ctx.out(), "{}", *element.Get());
      }
   };

} // namespace fmt
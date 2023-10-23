///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TOwned.hpp"
#include "inner/Handle.hpp"


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
   class TPointer : public TOwned<T*> {
      using Base = TOwned<T*>;
      using Self = TPointer<T, DR>;
      using Type = TypeOf<Base>;

   protected:
      using Base::mValue;
      const Allocation* mEntry {};
   
      void ResetInner();

   public:
      constexpr TPointer() noexcept = default;

      TPointer(const TPointer&);
      TPointer(TPointer&&);

      TPointer(const CT::PointerRelated auto&);
      TPointer(CT::PointerRelated auto&);
      TPointer(CT::PointerRelated auto&&);
      TPointer(CT::ShallowSemantic auto&&);
      TPointer(CT::DeepSemantic auto&&) requires CT::CloneMakable<T>;

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
      TPointer& operator = (CT::ShallowSemantic auto&&);
      TPointer& operator = (CT::DeepSemantic auto&&) requires CT::CloneAssignable<T>;

      NOD() operator TPointer<const T, DR>() const noexcept requires CT::Mutable<T>;

      using Base::operator bool;
      using Base::operator ->;

      NOD() const T* operator * () const noexcept;
      NOD()       T* operator * () noexcept;

   private:
      void ConstructFrom(CT::Semantic auto&&);
      TPointer& AssignFrom(CT::Semantic auto&&);
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

} // namespace Langulus::Anyness

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
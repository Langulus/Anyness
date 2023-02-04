///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TOwned.hpp"

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
   class TPointer : public TOwned<Conditional<CT::Constant<T>, const T*, T*>> {
      using Base = TOwned<Conditional<CT::Constant<T>, const T*, T*>>;
      using Self = TPointer<T, DR>;

   protected:
      using Base::mValue;
      Inner::Allocation* mEntry {};
   
      void ResetInner();

   public:
      LANGULUS(TYPED) TypeOf<Base>;

      constexpr TPointer() noexcept = default;

      TPointer(const TPointer&);
      TPointer(TPointer&&) noexcept;

      /// Constructor needs to be declared here to avoid MSVC parser bug      
      template<CT::Semantic S>
      TPointer(S&& other) noexcept requires (CT::Exact<TypeOf<S>, TPointer>)
         : Base {S::Nest(other.mValue.mValue)} {
         if constexpr (S::Move) {
            // Move in the contents of the other shared pointer         
            mEntry = other.mValue.mEntry;
            other.mValue.mEntry = nullptr;
         }
         else if constexpr (S::Keep) {
            // Copy the entry of the other shared pointer               
            mEntry = other.mValue.mEntry;

            if (mValue) {
               // And reference the memory if pointer is valid          
               if (mEntry)
                  mEntry->Keep();

               if constexpr (DR && CT::Referencable<T>)
                  mValue->Keep();
            }
         }
         else mEntry = nullptr;
      }

      TPointer(const CTTI_InnerType&);
      TPointer(CTTI_InnerType&&);

      /// Constructor needs to be declared here to avoid MSVC parser bug      
      template<CT::Semantic S>
      TPointer(S&& ptr) noexcept requires (CT::Exact<TypeOf<S>, CTTI_InnerType>)
         : Base {S::Nest(ptr.mValue)} {
         if constexpr (S::Move) {
            // Move in the contents of the other shared pointer         
            #if LANGULUS_FEATURE(MANAGED_MEMORY)
               mEntry = Inner::Allocator::Find(MetaData::Of<T>(), mValue);
            #endif
         }
         else if constexpr (S::Keep) {
            if (mValue) {
               // Copy the entry of the other shared pointer            
               #if LANGULUS_FEATURE(MANAGED_MEMORY)
                  mEntry = Inner::Allocator::Find(MetaData::Of<T>(), mValue);
               #endif

               // And reference the memory if pointer is valid          
               if (mEntry)
                  mEntry->Keep();

               if constexpr (DR && CT::Referencable<T>)
                  mValue->Keep();
            }
         }
         else mEntry = nullptr;
      }

      ~TPointer();

      NOD() Block GetBlock() const;
      NOD() constexpr bool HasAuthority() const noexcept;
      NOD() constexpr Count GetUses() const noexcept;
      using Base::Get;

      template<class... ARGS>
      void New(ARGS&&...);

      void Reset();
      TPointer Clone() const;

      using Base::operator bool;
      NOD() explicit operator const T* () const noexcept;
      NOD() explicit operator T* () noexcept;

      TPointer& operator = (const TPointer&);
      TPointer& operator = (TPointer&&);

      /// Constructor needs to be declared here to avoid MSVC parser bug      
      template<CT::Semantic S>
      TPointer& operator = (S&& rhs) noexcept requires (CT::Exact<TypeOf<S>, TPointer>) {
         if (rhs.mValue.mValue) {
            if constexpr (!S::Move && S::Keep) {
               // Always first reference the other, before dereferencing
               // so we don't prematurely lose the data in the rare     
               // case pointers are the same                            
               if constexpr (DR && CT::Referencable<T>)
                  rhs.mValue.mValue->Keep();
               if (rhs.mValue.mEntry)
                  rhs.mValue.mEntry->Keep();
            }

            if (mValue)
               ResetInner();

            mValue = rhs.mValue.mValue;
            mEntry = rhs.mValue.mEntry;
            if constexpr (S::Move) {
               if constexpr (S::Keep)
                  rhs.mValue.mValue = {};
               rhs.mValue.mEntry = {};
            }

            return *this;
         }

         Reset();
         return *this;
      }

      TPointer& operator = (const CTTI_InnerType&);
      TPointer& operator = (CTTI_InnerType&&);

      /// Constructor needs to be declared here to avoid MSVC parser bug      
      template<CT::Semantic S>
      TPointer& operator = (S&& rhs) noexcept requires (CT::Exact<TypeOf<S>, CTTI_InnerType>) {
         if (mValue)
            ResetInner();

         new (this) TPointer {rhs.Forward()};
         return *this;
      }

      NOD() operator TPointer<const T, DR>() const noexcept requires CT::Mutable<T>;

      using Base::operator ==;
      NOD() bool operator == (const TPointer&) const noexcept;

      using Base::operator ->;
      using Base::operator *;
   };

   /// A shared pointer, that provides ownage and basic reference counting    
   /// Referencing comes from the block of memory that the pointer points to  
   /// The memory block might contain more data, that will be implicitly      
   /// referenced, too                                                        
   template<class T>
   using Ptr = TPointer<T, false>;

   /// A shared pointer, that provides ownage and more reference counting     
   /// Referencing comes first from the block of memory that the pointer      
   /// points to, and second - the instance's individual reference counter    
   /// Useful for keeping track not only of the memory, but of the individual 
   /// element inside the memory block                                        
   template<class T>
   using Ref = TPointer<T, true>;

} // namespace Langulus::Anyness

#include "TPointer.inl"

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
   ///   Provides only ownership, for when you need to cleanup after a move   
   /// By default, fundamental types and pointers are not reset after a move  
   /// Wrapping them inside this ensures they are                             
   ///                                                                        
   template<CT::Data T>
   class TOwned {
   protected:
      T mValue {};

   public:
      /// Makes TOwned CT::Typed																
      using MemberType = T;

      constexpr TOwned() noexcept = default;
      constexpr TOwned(const TOwned&) noexcept = default;
      constexpr TOwned(TOwned&&) noexcept;
      constexpr TOwned(const T&) noexcept;

      NOD() DMeta GetType() const;
      NOD() Block GetBlock() const;

      void Reset() noexcept;

      constexpr TOwned& operator = (const TOwned&) noexcept = default;
      constexpr TOwned& operator = (TOwned&&) noexcept;
      constexpr TOwned& operator = (const T&) noexcept;

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


   ///                                                                        
   ///   A shared pointer                                                     
   ///                                                                        
   ///   Provides ownership and referencing. Also, for single-element         
   /// containment, it is a bit more efficient than TAny. So, essentially     
   /// its equivalent to std::shared_ptr                                      
   ///                                                                        
   template<class T, bool DOUBLE_REFERENCED>
   class TPointer : public TOwned<Conditional<CT::Constant<T>, const T*, T*>> {
      using Base = TOwned<Conditional<CT::Constant<T>, const T*, T*>>;
   protected:
      using Base::mValue;
      Inner::Allocation* mEntry {};
   
      void ResetInner();

   public:
      using typename Base::MemberType;
      
      constexpr TPointer() noexcept = default;
      TPointer(const TPointer&);
      TPointer(TPointer&&) noexcept;
      TPointer(MemberType);
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
      TPointer& operator = (MemberType);

      template<CT::Sparse ALT_T>
      TPointer& operator = (ALT_T);
      template<class ALT_T>
      TPointer& operator = (const TPointer<ALT_T, DOUBLE_REFERENCED>&);

      NOD() operator TPointer<const T, DOUBLE_REFERENCED>() const noexcept requires CT::Mutable<T>;

      using Base::operator ==;
      NOD() bool operator == (const TPointer&) const noexcept;

      using Base::operator ->;
      using Base::operator *;
   };

   /// Just a handle for a pointer, that provides ownage                      
   /// Pointer will be explicitly nulled after a move                         
   template<class T>
   using Own = TOwned<T>;

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

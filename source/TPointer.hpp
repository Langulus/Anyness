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
      using typename Base::MemberType;
      
      constexpr TPointer() noexcept = default;

      TPointer(const TPointer&);
      TPointer(TPointer&&) noexcept;
      template<CT::Semantic S>
      TPointer(S&&) noexcept requires (S::template Exact<TPointer<T, DR>>);

      TPointer(MemberType);
      template<CT::Semantic S>
      TPointer(S&&) noexcept requires (S::template Exact<typename TPointer<T, DR>::MemberType>);

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
      template<CT::Semantic S>
      TPointer& operator = (S&&) noexcept requires (S::template Exact<TPointer<T, DR>>);

      TPointer& operator = (MemberType);
      template<CT::Semantic S>
      TPointer& operator = (S&&) noexcept requires (S::template Exact<typename TPointer<T, DR>::MemberType>);

      template<CT::Sparse ALT_T>
      TPointer& operator = (ALT_T);
      template<class ALT_T>
      TPointer& operator = (const TPointer<ALT_T, DR>&);
      template<class ALT_T>
      TPointer& operator = (TPointer<ALT_T, DR>&&);
      template<class ALT_T>
      TPointer& operator = (Abandoned<TPointer<ALT_T, DR>>&&);
      template<class ALT_T>
      TPointer& operator = (Disowned<TPointer<ALT_T, DR>>&&);

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

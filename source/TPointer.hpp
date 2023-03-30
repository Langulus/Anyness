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
      using Type = TypeOf<Base>;

      /// Semantic constraints, should be kept to date with constructor       
      template<CT::NotSemantic S>
      static constexpr bool RelevantT = CT::ExactAsOneOf<S, Self, ::std::nullptr_t, T*>;
      template<CT::Semantic S>
      static constexpr bool RelevantS = RelevantT<TypeOf<S>>;

   protected:
      using Base::mValue;
      Inner::Allocation* mEntry {};
   
      void ResetInner();

   public:
      LANGULUS(TYPED) TypeOf<Base>;

      constexpr TPointer() noexcept = default;

      TPointer(const TPointer&);
      TPointer(TPointer&&);

      template<CT::NotSemantic ALT>
      TPointer(const ALT&) requires RelevantT<ALT>;
      template<CT::NotSemantic ALT>
      TPointer(ALT&) requires RelevantT<ALT>;
      template<CT::NotSemantic ALT>
      TPointer(ALT &&) requires RelevantT<ALT>;

      template<CT::Semantic S>
      TPointer(S&&) requires RelevantS<S>;

      ~TPointer();

      NOD() Block GetBlock() const;
      NOD() auto GetHandle() const;
      NOD() constexpr bool HasAuthority() const noexcept;
      NOD() constexpr Count GetUses() const noexcept;
      using Base::Get;

      template<class... ARGS>
      void New(ARGS&&...);

      void Reset();

      using Base::operator bool;
      NOD() explicit operator const T* () const noexcept;
      NOD() explicit operator T* () noexcept;

      TPointer& operator = (const TPointer&);
      TPointer& operator = (TPointer&&);

      TPointer& operator = (const CT::NotSemantic auto&);
      TPointer& operator = (CT::NotSemantic auto&);
      TPointer& operator = (CT::NotSemantic auto&&);

      template<CT::Semantic S>
      TPointer& operator = (S&&);

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

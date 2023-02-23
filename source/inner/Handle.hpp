///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../blocks/Block.hpp"

namespace Langulus::A
{

   struct Handle {};

} // namespace Langulus::A

namespace Langulus::Anyness
{
   
   ///                                                                        
   ///   An element & allocation pair                                         
   ///                                                                        
   /// Used as intermediate type when managed memory is enabled, to keep      
   /// track of pointers inserted to containers. This does not have ownership 
   /// and can be used as iterator only when EMBEDed.                         
   ///                                                                        
   template<CT::Sparse T, bool EMBED>
   struct Handle : A::Handle {
      LANGULUS(TYPED) T;
      LANGULUS_BASES(A::Handle);

      static_assert(CT::Void<Deptr<T>> || CT::Allocatable<Deptr<T>>,
         "Handle to unallocatable non-void T is pointless");
      static constexpr bool Embedded = EMBED;

   protected: TESTING(public:)
      friend class Block;

      /// @cond show_protected                                                
      // The value                                                      
      Conditional<EMBED, T*, T> mValue;
      // The entry                                                      
      Conditional<EMBED, Inner::Allocation**, Inner::Allocation*> mEntry;
      /// @endcond show_protected                                             

   public:
      Handle() = delete;

      constexpr Handle(const Handle&) noexcept = default;
      constexpr Handle(Handle&&) noexcept = default;
      template<CT::Semantic S>
      constexpr Handle(S&&) noexcept;
      constexpr Handle(decltype(mValue), decltype(mEntry)) SAFETY_NOEXCEPT();

      constexpr Handle& operator = (const Handle&) noexcept = default;
      constexpr Handle& operator = (Handle&&) noexcept = default;

      NOD() T Get() const noexcept;
      NOD() Inner::Allocation* GetEntry() const noexcept;
      
      void Set(T) noexcept;
      void SetEntry(Inner::Allocation*) noexcept;
      
      // Prefix operators                                               
      Handle& operator ++ () noexcept requires EMBED;
      Handle& operator -- () noexcept requires EMBED;
      Handle& operator * () noexcept;

      // Suffix operators                                               
      NOD() Handle operator ++ (int) noexcept requires EMBED;
      NOD() Handle operator -- (int) noexcept requires EMBED;
      NOD() Handle operator + (int) noexcept requires EMBED;
      NOD() Handle operator - (int) noexcept requires EMBED;

      template<bool RESET>
      void Destroy() const;
   };
   
} // namespace Langulus::Anyness

namespace Langulus::CT
{

   template<class... T>
   concept Handle = (DerivedFrom<T, A::Handle> && ...);

} // namespace Langulus::CT

#include "Handle.inl"
///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TMany.hpp"
#include "Trait.hpp"
#include "../maps/TMap.hpp"
#include "../Charge.inl"


namespace Langulus::Anyness
{
   namespace Inner
   {

      /// Constructs pushed to Neat are deconstructed, because Construct      
      /// itself uses Neat as base, and that's a circular dependency          
      struct DeConstruct {
         Hash mHash;
         Charge mCharge;
         Many mData;

         template<template<class> class S>
         DeConstruct(Hash, const Charge&, S<Neat>&&);
         template<template<class> class S>
         DeConstruct(S<DeConstruct>&&);

         Hash GetHash() const noexcept;

         bool operator == (const DeConstruct&) const;
      };

   } // namespace Langulus::Anyness::Inner


   ///                                                                        
   ///   Neat - a normalized data container                                   
   ///                                                                        
   ///   Turns messy containers into neatly and consistently orderless ones,  
   /// that are very fast on compare/search/insert/remove, albeit quite a bit 
   /// larger.                                                                
   ///   Neats are extensively used as descriptors in factories, to check     
   /// whether an element with the same signature already exists. This is why 
   /// traits like Traits::Parent are never actually considered, when taking  
   /// the hash of the Neat, or when comparing two Neat containers.           
   ///                                                                        
   class Neat {
   protected:
      friend class Construct;

      // The hash of the container                                      
      // Kept as first member, in order to quickly access it            
      mutable Hash mHash;

      // Traits are ordered first by their trait type, then by their    
      // order of appearance. Duplicate trait types are allowed         
      // Trait contents are also normalized all the way through         
      TUnorderedMap<TMeta, TMany<Many>> mTraits;

      // Subconstructs are sorted first by the construct type, and then 
      // by their order of appearance. Their contents are also          
      // nest-normalized all the way through                            
      TUnorderedMap<DMeta, TMany<Inner::DeConstruct>> mConstructs;

      // Any other block type that doesn't fit in the above is sorted   
      // first by the block type, then by the order of appearance       
      // These sub-blocks won't contain Neat elements                   
      TUnorderedMap<DMeta, TMany<Messy>> mAnythingElse;

   public:
      LANGULUS(DEEP) true;
      static constexpr bool Ownership = true;
      static constexpr bool CTTI_Container = true;

      ///                                                                     
      ///   Construction                                                      
      ///                                                                     
      constexpr Neat() = default;
      Neat(const Neat&);
      Neat(Neat&&) noexcept;

      template<template<class> class S> requires CT::Intent<S<Neat>>
      Neat(S<Neat>&&);

      template<class T1, class...TN> requires CT::UnfoldInsertable<T1, TN...>
      Neat(T1&&, TN&&...);

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      Neat& operator = (const Neat&) = default;
      Neat& operator = (Neat&&) noexcept = default;

      template<template<class> class S> requires CT::Intent<S<Neat>>
      Neat& operator = (S<Neat>&&);

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      bool operator == (const Neat&) const;

      void Clear();
      void Reset();

      ///                                                                     
      ///   Encapsulation                                                     
      ///                                                                     
      NOD() Hash GetHash() const;
      NOD() bool IsEmpty() const noexcept;
      NOD() bool IsMissingDeep() const;
      NOD() bool IsExecutable() const noexcept;

      NOD() explicit operator bool() const noexcept;

      template<CT::Trait>
      TMany<Many>* GetTraits();

      template<CT::Trait>
      const TMany<Many>* GetTraits() const;
            TMany<Many>* GetTraits(TMeta);
      const TMany<Many>* GetTraits(TMeta) const;

      template<CT::Data>
      TMany<Messy>* GetData();

      template<CT::Data>
      const TMany<Messy>* GetData() const;
            TMany<Messy>* GetData(DMeta);
      const TMany<Messy>* GetData(DMeta) const;
      
      template<CT::Data>
      TMany<Inner::DeConstruct>* GetConstructs();

      template<CT::Data>
      DMeta FindType() const;
      DMeta FindType(DMeta) const;

      template<CT::Data>
      const TMany<Inner::DeConstruct>* GetConstructs() const;
            TMany<Inner::DeConstruct>* GetConstructs(DMeta);
      const TMany<Inner::DeConstruct>* GetConstructs(DMeta) const;

      template<CT::Trait>
      void SetDefaultTrait(CT::Data auto&&);

      template<CT::Trait>
      void OverwriteTrait(CT::Data auto&&);

      template<CT::Trait...>
      bool ExtractTrait(CT::Data auto&...) const;
      Count ExtractData(CT::Data auto&) const;

      // Intentionally undefined, because it requires Langulus::Flow    
      // and relies on Verbs::Interpret                                 
      // If you receive missing externals, include the following:       
      //    #include <Flow/Verbs/Interpret.hpp>                         
      Count ExtractDataAs(CT::Data auto&) const;

      template<CT::Trait>
      NOD() const Many* Get(Offset = 0) const;
      NOD() const Many* Get(TMeta, Offset = 0) const;

   protected:
      template<CT::Trait>
      bool ExtractTraitInner(CT::Data auto&...) const;

   public:
      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      template<bool MUTABLE = true>
      Count ForEach(auto&&...);
      Count ForEach(auto&&...) const;

      template<bool MUTABLE = true>
      Count ForEachDeep(auto&&...);
      Count ForEachDeep(auto&&...) const;

      template<bool MUTABLE = true>
      Count ForEachTrait(auto&&);
      Count ForEachTrait(auto&&) const;

      template<bool MUTABLE = true>
      Count ForEachConstruct(auto&&);
      Count ForEachConstruct(auto&&) const;

      template<bool MUTABLE = true>
      Count ForEachTail(auto&&);
      Count ForEachTail(auto&&) const;

   protected:
      template<bool MUTABLE = true>
      Count ForEachInner(auto&&);
      Count ForEachInner(auto&&) const;

   public:
      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      template<class T1, class...TN>
      Count Insert(T1&&, TN&&...);

      Neat& operator << (auto&&);

      void Merge(const Neat&);

      Neat& operator <<= (auto&&);

      Neat& Set(CT::TraitBased auto&&, Offset = 0);

   protected:
      Count UnfoldInsert(auto&&);
      void InsertInner(auto&&);

      void AddTrait(CT::Intent auto&&);
      void AddConstruct(CT::Intent auto&&);
      void AddVerb(CT::Intent auto&&);

      template<Offset... IDX>
      bool ExtractTraitInner(const TMany<Many>&, ::std::integer_sequence<Offset, IDX...>, CT::Data auto&...) const;
      template<Offset>
      bool ExtractTraitInnerInner(const TMany<Many>&, CT::Data auto&) const;

   public:
      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      template<CT::Data, bool EMPTY_TOO = false>
      Count RemoveData();
      template<CT::Data>
      Count RemoveConstructs();
      template<CT::Trait, bool EMPTY_TOO = false>
      Count RemoveTrait();

      ///                                                                     
      ///   Conversion                                                        
      ///                                                                     
      Count Serialize(CT::Serial auto&) const;
   };

} // namespace Langulus::Anyness

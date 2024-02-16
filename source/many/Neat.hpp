///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TAny.hpp"
#include "Trait.hpp"
#include "../maps/TMap.hpp"
#include "../Charge.hpp"


namespace Langulus::Anyness
{
   namespace Inner
   {

      /// Constructs pushed to Neat are deconstructed, because Construct      
      /// itself uses Neat as base, and that's a circular dependency          
      struct DeConstruct {
         Hash mHash;
         Charge mCharge;
         Any mData;

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
      TUnorderedMap<TMeta, TAny<Any>> mTraits;

      // Subconstructs are sorted first by the construct type, and then 
      // by their order of appearance. Their contents are also          
      // nest-normalized all the way through                            
      TUnorderedMap<DMeta, TAny<Inner::DeConstruct>> mConstructs;

      // Any other block type that doesn't fit in the above is sorted   
      // first by the block type, then by the order of appearance       
      // These sub-blocks won't contain Neat elements                   
      TUnorderedMap<DMeta, TAny<Messy>> mAnythingElse;

   public:
      ///                                                                     
      ///   Construction                                                      
      ///                                                                     
      constexpr Neat() = default;
      Neat(const Neat&);
      Neat(Neat&&) noexcept;

      template<template<class> class S>
      Neat(S<Neat>&&) requires CT::Semantic<S<Neat>>;

      template<class T1, class...TN>
      Neat(T1&&, TN&&...)
      requires CT::Inner::UnfoldInsertable<T1, TN...>;

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      Neat& operator = (const Neat&) = default;
      Neat& operator = (Neat&&) noexcept = default;
      template<template<class> class S>
      Neat& operator = (S<Neat>&&) requires CT::Semantic<S<Neat>>;

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      bool operator == (const Neat&) const;

      void Clear();
      void Reset();

      NOD() Messy MakeMessy() const;

      ///                                                                     
      ///   Encapsulation                                                     
      ///                                                                     
      NOD() Hash GetHash() const;
      NOD() bool IsEmpty() const noexcept;
      NOD() bool IsMissingDeep() const;
      NOD() bool IsExecutable() const noexcept;

      NOD() explicit operator bool() const noexcept;

      template<CT::Trait>
      TAny<Any>* GetTraits();

      template<CT::Trait>
      const TAny<Any>* GetTraits() const;
            TAny<Any>* GetTraits(TMeta);
      const TAny<Any>* GetTraits(TMeta) const;

      template<CT::Data>
      TAny<Messy>* GetData();

      template<CT::Data>
      const TAny<Messy>* GetData() const;
            TAny<Messy>* GetData(DMeta);
      const TAny<Messy>* GetData(DMeta) const;
      
      template<CT::Data>
      TAny<Inner::DeConstruct>* GetConstructs();

      template<CT::Data>
      const TAny<Inner::DeConstruct>* GetConstructs() const;
            TAny<Inner::DeConstruct>* GetConstructs(DMeta);
      const TAny<Inner::DeConstruct>* GetConstructs(DMeta) const;

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
      NOD() const Any* Get(Offset = 0) const;
      NOD() const Any* Get(TMeta, Offset = 0) const;

   protected:
      template<CT::Trait>
      bool ExtractTraitInner(CT::Data auto&...) const;

   public:
      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      template<bool MUTABLE = true, class...F>
      Count ForEach(F&&...);
      template<class...F>
      Count ForEach(F&&...) const;

      template<bool MUTABLE = true, class...F>
      Count ForEachDeep(F&&...);
      template<class...F>
      Count ForEachDeep(F&&...) const;

      template<bool MUTABLE = true, class F>
      Count ForEachTrait(F&&);
      template<class F>
      Count ForEachTrait(F&&) const;

      template<bool MUTABLE = true, class F>
      Count ForEachConstruct(F&&);
      template<class F>
      Count ForEachConstruct(F&&) const;

      template<bool MUTABLE = true, class F>
      Count ForEachTail(F&&);
      template<class F>
      Count ForEachTail(F&&) const;

   protected:
      template<bool MUTABLE = true, class F>
      Count ForEachInner(F&&);
      template<class F>
      Count ForEachInner(F&&) const;

   public:
      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      template<class T1, class...TAIL>
      Count Insert(T1&&, TAIL&&...);

      Neat& operator << (auto&&);

      void Merge(const Neat&);

      Neat& operator <<= (auto&&);

      Neat& Set(CT::TraitBased auto&&, Offset = 0);

   protected:
      Count UnfoldInsert(auto&&);
      void InsertInner(auto&&);

      void AddTrait(auto&&);
      void AddData(auto&&);
      void AddConstruct(auto&&);

      template<Offset... IDX>
      bool ExtractTraitInner(const TAny<Any>&, ::std::integer_sequence<Offset, IDX...>, CT::Data auto&...) const;
      template<Offset>
      bool ExtractTraitInnerInner(const TAny<Any>&, CT::Data auto&) const;

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

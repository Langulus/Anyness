///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Trait.hpp"
#include "TUnorderedMap.hpp"
#include "inner/Charge.hpp"


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

         LANGULUS(INLINED)
         Hash GetHash() const noexcept {
            return mHash;
         }

         LANGULUS(INLINED)
         bool operator == (const DeConstruct& rhs) const {
            return mHash == rhs.mHash
               and mCharge == rhs.mCharge
               and mData == rhs.mData;
         }
      };

   } // namespace Langulus::Anyness::Inner


   ///                                                                        
   ///   Neat - a normalized data container                                   
   ///                                                                        
   ///   Turns messy containers into neatly and consistently orderless ones,  
   /// that are very fast on compare/search/insert/remove, albeit quite a bit 
   /// larger.                                                                
   ///   Neats are extensively used as descriptors in factories, to check     
   /// whether an element with the same signature already exists.             
   ///                                                                        
   class Neat {
   protected:
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
      explicit Neat(const Neat&);
      explicit Neat(Neat&&) noexcept;

      template<CT::Semantic S>
      explicit Neat(S&&) requires (CT::Neat<TypeOf<S>>);

      template<CT::NotSemantic T>
      explicit Neat(const T&) requires (not CT::Neat<T>);
      template<CT::NotSemantic T>
      explicit Neat(T&) requires (not CT::Neat<T>);
      template<CT::NotSemantic T>
      explicit Neat(T&&) requires (not CT::Neat<T>);
      template<CT::Semantic S>
      explicit Neat(S&&) requires (not CT::Neat<TypeOf<S>>);

      template<CT::Data HEAD, CT::Data... TAIL>
      explicit Neat(HEAD&&, TAIL&&...) requires (sizeof...(TAIL) >= 1);

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      Neat& operator = (const Neat&) = default;
      Neat& operator = (Neat&&) noexcept = default;
      Neat& operator = (CT::Semantic auto&&);

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
      NOD() constexpr bool IsEmpty() const noexcept;
      NOD() bool IsMissing() const;
      NOD() bool IsMissingDeep() const;

      NOD() constexpr explicit operator bool() const noexcept;

      // Intentionally left undefined                                   
      template<CT::Text T>
      NOD() T SerializeAs() const;

      template<CT::Trait T>
      TAny<Any>* GetTraits();
      template<CT::Trait T>
      const TAny<Any>* GetTraits() const;

      TAny<Any>* GetTraits(TMeta);
      const TAny<Any>* GetTraits(TMeta) const;

      template<CT::Data T>
      TAny<Messy>* GetData();
      template<CT::Data T>
      const TAny<Messy>* GetData() const;
      
      TAny<Messy>* GetData(DMeta);
      const TAny<Messy>* GetData(DMeta) const;
      
      template<CT::Data T>
      TAny<Inner::DeConstruct>* GetConstructs();
      template<CT::Data T>
      const TAny<Inner::DeConstruct>* GetConstructs() const;

      TAny<Inner::DeConstruct>* GetConstructs(DMeta);
      const TAny<Inner::DeConstruct>* GetConstructs(DMeta) const;

      template<CT::Trait T>
      void SetDefaultTrait(CT::Data auto&&);

      template<CT::Trait T>
      void OverwriteTrait(CT::Data auto&&);

      template<CT::Trait... T>
      bool ExtractTrait(CT::Data auto&...) const;
      Count ExtractData(CT::Data auto&) const;

      // Intentionally undefined, because it requires Langulus::Flow    
      // and relies on Verbs::Interpret                                 
      // If you receive missing externals, include the following:       
      //    #include <Flow/Verbs/Interpret.hpp>                         
      Count ExtractDataAs(CT::Data auto&) const;

      NOD() const Any* Get(TMeta, const Offset& = 0) const;
      template<CT::Trait T>
      NOD() const Any* Get(const Offset& = 0) const;

   protected:
      template<CT::Trait T>
      bool ExtractTraitInner(CT::Data auto&...) const;

   public:
      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      template<bool MUTABLE = true, class... F>
      Count ForEach(F&&...);
      template<class... F>
      Count ForEach(F&&...) const;

      template<bool MUTABLE = true, class... F>
      Count ForEachDeep(F&&...);
      template<class... F>
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
      Neat& operator << (const CT::NotSemantic auto&);
      Neat& operator << (CT::NotSemantic auto&);
      Neat& operator << (CT::NotSemantic auto&&);
      Neat& operator << (CT::Semantic auto&&);

      Neat& operator <<= (const CT::NotSemantic auto&);
      Neat& operator <<= (CT::NotSemantic auto&);
      Neat& operator <<= (CT::NotSemantic auto&&);
      Neat& operator <<= (CT::Semantic auto&&);

      Neat& Set(const Trait&, const Offset& = 0);
      template<CT::Trait T>
      void Set(CT::Semantic auto&&) const;

      void Merge(const Neat&);

   protected:
      template<CT::Semantic S>
      void AddTrait(S&&) requires (CT::TraitBased<TypeOf<S>>);

      template<Offset... IDX>
      bool ExtractTraitInner(const TAny<Any>&, ::std::integer_sequence<Offset, IDX...>, CT::Data auto&...) const;
      template<Offset>
      bool ExtractTraitInnerInner(const TAny<Any>&, CT::Data auto&) const;
   };

} // namespace Langulus::Anyness

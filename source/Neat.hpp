///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TUnorderedMap.hpp"
#include "Construct.hpp"

namespace Langulus::Anyness
{

   using Messy = Any;


   ///                                                                        
   ///   Neat - a normalized data container                                   
   ///                                                                        
   /// Turns messy containers into neatly and consistently ordered ones,      
   /// that are very fast on compare/search/insert/remove, but a bit larger.  
   ///                                                                        
   struct Neat {
      // Traits are ordered first by their trait type, then by their    
      // order of appearance. Duplicate trait types are allowed         
      // Trait contents are also normalized all the way through         
      TUnorderedMap<TMeta, TAny<Any>> mTraits;
      // Subconstructs are sorted first by the construct type, and then 
      // by their order of appearance. Their contents are also          
      // nest-normalized all the way through                            
      TUnorderedMap<DMeta, TAny<Construct>> mConstructs;
      // Any other block type that doesn't fit in the above is sorted   
      // first by the block type, then by the order of appearance       
      // These sub-blocks won't contain Neat elements                   
      TUnorderedMap<DMeta, TAny<Messy>> mAnythingElse;

      mutable Hash mHash;

   public:
      Neat(const Neat&) = default;
      Neat(Neat&&) noexcept = default;
      template<CT::Semantic S>
      Neat(S&&);

      Neat(const Messy&);

      Neat& operator = (const Neat&) = default;
      Neat& operator = (Neat&&) noexcept = default;
      template<CT::Semantic S>
      Neat& operator = (S&&);

      NOD() Messy MakeMessy() const;
      template<CT::Data T>
      NOD() Construct MakeConstruct() const;

      NOD() Hash GetHash() const;

      bool operator == (const Neat&) const;

      void Merge(const Neat&);

      template<CT::Trait T>
      TAny<Any>* GetTraits();
      template<CT::Trait T>
      const TAny<Any>* GetTraits() const;

      template<CT::Data T>
      TAny<Messy>* GetData();
      template<CT::Data T>
      const TAny<Messy>* GetData() const;
      
      template<CT::Data T>
      TAny<Construct>* GetConstructs();
      template<CT::Data T>
      const TAny<Construct>* GetConstructs() const;

      template<CT::Trait T, CT::Data D>
      void SetDefaultTrait(D&&);

      template<CT::Trait T, CT::Data D>
      void OverwriteTrait(D&&);

      template<CT::Trait T, CT::Data... D>
      bool ExtractTrait(D&...) const;
      template<CT::Data D>
      bool ExtractData(D&) const;
      template<CT::Data D>
      bool ExtractDataAs(D&) const;

   protected:
      template<CT::Data... D, Offset... IDX>
      bool ExtractTraitInner(const TAny<Any>&, ::std::integer_sequence<Offset, IDX...>, D&...) const;
      template<Offset, CT::Data D>
      bool ExtractTraitInnerInner(const TAny<Any>&, D&) const;
   };

} // namespace Langulus::Anyness

#include "Neat.inl"
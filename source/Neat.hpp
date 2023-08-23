///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TUnorderedMap.hpp"
#include "Trait.hpp"
#include "inner/Charge.hpp"


namespace Langulus::Anyness
{

   class Construct;
   using Messy = Any;


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

      // Constructs pushed to Neat are deconstructed, because Construct 
      // itself uses Neat as base, and we can't nest dense types        
      struct DeConstruct {
         Hash mHash;
         Charge mCharge;
         Any mData;

         LANGULUS(INLINED)
         bool operator == (const DeConstruct& rhs) const {
            return mHash == rhs.mHash
               and mCharge == rhs.mCharge
               and mData == rhs.mData;
         }
      };

      // Subconstructs are sorted first by the construct type, and then 
      // by their order of appearance. Their contents are also          
      // nest-normalized all the way through                            
      TUnorderedMap<DMeta, TAny<DeConstruct>> mConstructs;
      // Any other block type that doesn't fit in the above is sorted   
      // first by the block type, then by the order of appearance       
      // These sub-blocks won't contain Neat elements                   
      TUnorderedMap<DMeta, TAny<Messy>> mAnythingElse;

   public:
      constexpr Neat() = default;
      constexpr Neat(const Neat&) = default;
      constexpr Neat(Neat&&) noexcept = default;

      template<CT::Semantic S>
      Neat(S&&);

      Neat(const Messy&);

      constexpr Neat& operator = (const Neat&) = default;
      constexpr Neat& operator = (Neat&&) noexcept = default;
      template<CT::Semantic S>
      Neat& operator = (S&&);

      bool operator == (const Neat&) const;

      void Clear();
      void Reset();

      NOD() Messy MakeMessy() const;
      template<CT::Data T>
      NOD() Construct MakeConstruct() const;

      ///                                                                     
      ///   Encapsulation                                                     
      ///                                                                     
      NOD() Hash GetHash() const;
      NOD() constexpr bool IsEmpty() const noexcept;
      NOD() constexpr bool IsMissing() const;
      NOD() constexpr bool IsMissingDeep() const;

      NOD() constexpr explicit operator bool() const noexcept;

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

      NOD() const Any* Get(TMeta, const Offset & = 0) const;
      template<CT::Trait T>
      NOD() const Any* Get(const Offset & = 0) const;

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Neat& operator << (const CT::NotSemantic auto&);
      Neat& operator << (CT::NotSemantic auto&);
      Neat& operator << (CT::NotSemantic auto&&);
      Neat& operator << (CT::Semantic auto&&);

      Neat& operator >> (const CT::NotSemantic auto&);
      Neat& operator >> (CT::NotSemantic auto&);
      Neat& operator >> (CT::NotSemantic auto&&);
      Neat& operator >> (CT::Semantic auto&&);

      Neat& operator <<= (const CT::NotSemantic auto&);
      Neat& operator <<= (CT::NotSemantic auto&);
      Neat& operator <<= (CT::NotSemantic auto&&);
      Neat& operator <<= (CT::Semantic auto&&);

      Neat& operator >>= (const CT::NotSemantic auto&);
      Neat& operator >>= (CT::NotSemantic auto&);
      Neat& operator >>= (CT::NotSemantic auto&&);
      Neat& operator >>= (CT::Semantic auto&&);

      Neat& Set(const Trait&, const Offset & = 0);
      template<CT::Trait T, CT::Semantic S>
      void Set(S&&) const;

      void Merge(const Neat&);

   protected:
      template<CT::Data... D, Offset... IDX>
      bool ExtractTraitInner(const TAny<Any>&, ::std::integer_sequence<Offset, IDX...>, D&...) const;
      template<Offset, CT::Data D>
      bool ExtractTraitInnerInner(const TAny<Any>&, D&) const;
   };

} // namespace Langulus::Anyness

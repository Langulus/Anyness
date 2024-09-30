///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "../Block.hpp"
#include "../../many/Trait.hpp"
#include "../../many/Construct.hpp"


namespace Langulus::Anyness
{

   ///                                                                        
   template<class TYPE> template<class TRAIT>
   void Block<TYPE>::SetDefaultTrait(CT::Data auto&&) {
      static_assert(CT::Trait<TRAIT>, "TRAIT is not a trait type");
      TODO();
   }

   ///                                                                        
   template<class TYPE> template<class TRAIT>
   void Block<TYPE>::OverwriteTrait(CT::Data auto&&) {
      static_assert(CT::Trait<TRAIT>, "TRAIT is not a trait type");
      TODO();
   }

   /// Extract a trait from the descriptor                                    
   ///   @tparam TRAIT... - trait(s) we're searching for                      
   ///   @param values - [out] where to save the value, if found              
   ///   @return true if value changed                                        
   template<class TYPE> template<class...TRAIT>
   bool Block<TYPE>::ExtractTrait(CT::Data auto&...values) const {
      static_assert(CT::Trait<TRAIT...>, "TRAIT is not a trait type");
      return (ExtractTraitInner<TRAIT>(values...) or ...);
   }

   /// Extract data of an exact type, doing only pointer arithmetic           
   ///   @param value - [out] where to save the value(s), if found            
   ///   @return the number of extracted values (always 1 if not an array)    
   template<class TYPE>
   auto Block<TYPE>::ExtractData(CT::Data auto& value) const -> Count {
      using D = Deref<decltype(value)>;
      Count progress = 0;

      ForEachDeep([&](const Decay<D>& data) {
         if constexpr (CT::Array<D>) {
            //TODO can be optimized-out for POD
            value[progress] = data;
            ++progress;
            return (progress >= ExtentOf<D>) ? Loop::Break : Loop::Continue;
         }
         else {
            value = data;
            ++progress;
            return Loop::Break;
         }
      });

      return progress;
   }

   /// Extract any data that is convertible to D                              
   ///   @param value - [out] where to save the value, if found               
   ///   @return the number of extracted values (always 1 if not an array)    
   template<class TYPE>
   auto Block<TYPE>::ExtractDataAs(CT::Data auto& value) const -> Count {
      using D = Deref<decltype(value)>;
      Count progress = 0;

      ForEachDeep([&](const Many& group) {
         if constexpr (CT::Array<D>) {
            const auto toscan = ::std::min(ExtentOf<D> - progress, group.GetCount());
            for (Offset i = 0; i < toscan; ++i) {
               //TODO can be optimized-out for POD
               try {
                  value[progress] = group.template AsCast<Deext<D>>(i);
                  ++progress;
               }
               catch (...) {}
            }

            return (progress >= ExtentOf<D>) ? Loop::Break : Loop::Continue;
         }
         else {
            try {
               value = group.template AsCast<D>();
               ++progress;
               return Loop::Break;
            }
            catch (...) {}
            return Loop::Continue;
         }
      });

      return progress;
   }

   ///                                                                        
   template<class TYPE> template<CT::Data>
   auto Block<TYPE>::FindType() const -> DMeta {
      TODO();
      return {};
   }

   ///                                                                        
   template<class TYPE>
   auto Block<TYPE>::FindType(DMeta) const -> DMeta {
      TODO();
      return {};
   }

   ///                                                                        
   template<class TYPE> template<class TRAIT>
   bool Block<TYPE>::ExtractTraitInner(CT::Data auto&...values) const {
      static_assert(CT::Trait<TRAIT>, "TRAIT is not a trait type");
      return ExtractTraitInner<TRAIT>(
         Sequence<sizeof...(values)>::Expand, values...
      );
   }

   ///                                                                        
   template<class TYPE> template<class TRAIT, Offset...IDX>
   bool Block<TYPE>::ExtractTraitInner(
      ExpandedSequence<IDX...>, CT::Data auto&...values
   ) const {
      static_assert(CT::Trait<TRAIT>, "TRAIT is not a trait type");
      return (ExtractTraitInnerInner<TRAIT, IDX>(values) or ...);
   }

   ///                                                                        
   template<class TYPE> template<class TRAIT, Offset IDX>
   bool Block<TYPE>::ExtractTraitInnerInner(CT::Data auto& value) const {
      static_assert(CT::Trait<TRAIT>, "TRAIT is not a trait type");
      using D = Deref<decltype(value)>;
      bool satisfied = false;
      Count counter = 0;

      ForEachDeep([&](const TRAIT& trait) {
         if (counter < IDX) {
            // We're only interested in the Nth trait                   
            ++counter;
            return Loop::Continue;
         }

         if constexpr (CT::Deep<D>) {
            value = static_cast<const Many&>(trait);
            satisfied = true;
         }
         else try {
            value = trait.template AsCast<D>();
            satisfied = true;
         }
         catch (...) {}
         return Loop::Break;
      });

      return satisfied;
   }

} // namespace Langulus::Anyness
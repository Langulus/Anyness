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

   template<class TYPE> template<class...TRAIT>
   bool Block<TYPE>::ExtractTrait(CT::Data auto&...) const {
      static_assert(CT::Trait<TRAIT...>, "TRAIT is not a trait type");
      TODO();
      return false;
   }

   template<class TYPE>
   auto Block<TYPE>::ExtractData(CT::Data auto&) const -> Count {
      TODO();
      return 0;
   }

   template<class TYPE>
   auto Block<TYPE>::ExtractDataAs(CT::Data auto&) const -> Count {
      TODO();
      return 0;
   }

   template<class TYPE> template<CT::Data>
   auto Block<TYPE>::FindType() const -> DMeta {
      TODO();
      return {};
   }

   template<class TYPE>
   auto Block<TYPE>::FindType(DMeta) const -> DMeta {
      TODO();
      return {};
   }

} // namespace Langulus::Anyness
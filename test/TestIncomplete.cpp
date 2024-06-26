///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include <Anyness/Text.hpp>
#include <Anyness/Trait.hpp>
#include <Anyness/Own.hpp>
#include <Anyness/Ref.hpp>
#include <Anyness/TMap.hpp>
#include "Common.hpp"


struct Resolvable {
   Resolvable(DMeta d) : mMeta {d} {}
   DMeta mMeta {};
};

class Unit;

using UnitMap = TUnorderedMap<DMeta, TMany<Unit*>>;
using TraitMap = TUnorderedMap<TMeta, TMany<Trait>>;

struct Thing final : Resolvable {
   static_assert(CT::Complete<Resolvable>);
   static_assert(CT::Complete<Own<Thing*>>);
   static_assert(CT::Complete<Ref<Thing>>);
   static_assert(CT::Complete<TMany<Thing*>>);
   static_assert(CT::Complete<UnitMap>);
   static_assert(CT::Complete<TraitMap>);

   LANGULUS(ABSTRACT) false;
   LANGULUS(PRODUCER) Thing;
   LANGULUS(UNINSERTABLE) false;
   LANGULUS_BASES(Resolvable);

   Thing();

   Own<Thing*> mOwned;
   Ref<Thing> mOwner;
   TMany<Thing*> mChildren;
   UnitMap mUnits;
   TraitMap mTraits;
};

Thing::Thing() : Resolvable {MetaOf<Thing>()} {}

SCENARIO("Testing incomplete type hierarchy", "[incomplete]") {
   GIVEN("A thing instance") {
      Thing thing;
   }
}
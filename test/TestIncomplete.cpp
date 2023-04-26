///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "Main.hpp"
#include <Anyness/Text.hpp>
#include <Anyness/Trait.hpp>
#include <Anyness/Own.hpp>
#include <Anyness/Ref.hpp>
#include <Anyness/TUnorderedMap.hpp>
#include <catch2/catch.hpp>

struct Resolvable {
   Resolvable(DMeta d) : mMeta {d} {}
   DMeta mMeta {};
};

class Unit;

using UnitMap = TUnorderedMap<DMeta, TAny<Unit*>>;
using TraitMap = TUnorderedMap<TMeta, TAny<Trait>>;

struct Thing final : Resolvable {
   LANGULUS(ABSTRACT) false;
   LANGULUS(PRODUCER) Thing;
   LANGULUS(UNINSERTABLE) false;
   LANGULUS_BASES(Resolvable);

   Thing();

   TOwned<Thing*> mOwned;
   Ptr<Thing> mOwner;
   TAny<Thing*> mChildren;
   UnitMap mUnits;
   TraitMap mTraits;
};

Thing::Thing() : Resolvable {MetaData::Of<Thing>()} {}

/// See https://github.com/catchorg/Catch2/blob/devel/docs/tostring.md        
CATCH_TRANSLATE_EXCEPTION(::Langulus::Exception const& ex) {
   const Text serialized {ex};
   return ::std::string {Token {serialized}};
}

SCENARIO("Testing incomplete type hierarchy", "[incomplete]") {
   GIVEN("A thing instance") {
      Thing thing;
   }
}
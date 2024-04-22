///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "TestManyCommon.hpp"


TEMPLATE_TEST_CASE("Converting to text", "[many]",
   Traits::Name, TMany<DMeta>, Many
) {
   static Allocator::State memoryState;

   using T = TestType;

   GIVEN("A container with three meta data definitions") {
      T pack {
         MetaDataOf<double>(),
         MetaDataOf<float>(),
         MetaDataOf<bool>()
      };

      WHEN("Converted to texts using a templated destination") {
         TMany<Text> converted;
         pack.Convert(converted);

         REQUIRE(converted.GetCount() == 3);
         REQUIRE(converted[0] == "double");
         REQUIRE(converted[1] == "float");
         REQUIRE(converted[2] == "bool");
      }
   }

   REQUIRE(memoryState.Assert());
}

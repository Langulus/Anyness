///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "TestAnyCommon.hpp"


TEMPLATE_TEST_CASE("Converting to text", "[any]", Traits::Name, TAny<DMeta>, Any) {
   static Allocator::State memoryState;

   using T = TestType;

   GIVEN("A container with three meta data definitions") {
      T pack {
         MetaDataOf<double>(),
         MetaDataOf<float>(),
         MetaDataOf<bool>()
      };

      WHEN("Converted to texts using a templated destination") {
         TAny<Text> converted;
         pack.Convert(converted);

         REQUIRE(converted.GetCount() == 3);
         REQUIRE(converted[0] == "double");
         REQUIRE(converted[1] == "float");
         REQUIRE(converted[2] == "bool");
      }
   }

   REQUIRE(memoryState.Assert());
}

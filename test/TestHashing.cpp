///                                                                           
/// Langulus::RTTI                                                            
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include <Anyness/Text.hpp>
#include <Anyness/Path.hpp>
#include "Common.hpp"


SCENARIO("Hashing different kinds of containers", "[hash]") {
   WHEN("Hashing all kinds of containers") {
      Token same1 = "Same1"; 
      std::string same1str = "Same1";
      Text same1txt = "Same1";
      Path same1pat = "Same1";
      Any same1any {'S', 'a', 'm', 'e', '1'};
      std::vector<char> same1vec = {'S', 'a', 'm', 'e', '1'};
      //std::span<char, 5> sape1spa = "Same1";
      std::array<char, 5> sape1arr = {'S', 'a', 'm', 'e', '1'};

      REQUIRE(HashOf(same1) == HashOf(same1str));
      REQUIRE(HashOf(same1) == HashOf(same1any));
      REQUIRE(HashOf(same1) == HashOf(same1txt));
      REQUIRE(HashOf(same1) == HashOf(same1pat));
      REQUIRE(HashOf(same1) == HashOf(same1vec));
      //REQUIRE(HashOf(same1) == HashOf(sape1spa));
      REQUIRE(HashOf(same1) == HashOf(sape1arr));
      REQUIRE(HashOf(same1) == HashBytes("Same1", 5));
   }
}
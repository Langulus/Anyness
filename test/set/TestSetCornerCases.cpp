///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "TestSetCommon.hpp"


/// Testing some corner cases encountered during the use of the container     
TEMPLATE_TEST_CASE("Set corner cases", "[set]",
   (SetTest<UnorderedSet, Text>),
   (SetTest<TUnorderedSet<Text>, Text>),
   (SetTest<TOrderedSet<Text>, Text>),
   (SetTest<OrderedSet, Text>)
) {
   using T = typename TestType::Container;
   using K = typename TestType::Key;

   GIVEN("Set instance initialized with 10 specific elements for the corner case") {
      const K keys[10] = {
         {"VulkanLayer"},
         {"VulkanRenderer"},
         {"VulkanCamera"},
         {"Platform"},
         {"Vulkan"},
         {"Window"},
         {"VulkanLight"},
         {"Monitor"},
         {"VulkanRenderable"},
         {"Cursor"}
      };

      T set {keys};

      WHEN("Removing around-the-end elements by value (corner case)") {
         Count removed = 0;
         removed += set.Remove("VulkanRenderer");
         removed += set.Remove("VulkanCamera");
         removed += set.Remove("Vulkan");
         removed += set.Remove("VulkanRenderable");
         removed += set.Remove("VulkanLight");
         removed += set.Remove("VulkanLayer");

         REQUIRE(removed == 6);
         REQUIRE(set.GetCount() == 4);

         REQUIRE(set.Contains("Platform"));
         REQUIRE(set.Contains("Window"));
         REQUIRE(set.Contains("Monitor"));
         REQUIRE(set.Contains("Cursor"));
         REQUIRE_FALSE(set.Contains("VulkanLayer"));
         REQUIRE_FALSE(set.Contains("VulkanRenderer"));
         REQUIRE_FALSE(set.Contains("VulkanCamera"));
         REQUIRE_FALSE(set.Contains("Vulkan"));
         REQUIRE_FALSE(set.Contains("VulkanLight"));
         REQUIRE_FALSE(set.Contains("VulkanRenderable"));
      }
   }
}

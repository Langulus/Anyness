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
#include <Anyness/TMap.hpp>
#include <Anyness/Map.hpp>
#include <unordered_map>
#include "../Common.hpp"


struct VulkanLayer {};
struct VulkanRenderer {};
struct VulkanCamera {};
struct Platform {};
struct Vulkan {};
struct Window {};
struct VulkanLight {};
struct Monitor {};
struct VulkanRenderable {};
struct Cursor {};

/// Testing some corner cases encountered during the use of the container     
TEMPLATE_TEST_CASE("Map corner cases", "[map]",
   (MapPair<UnorderedMap, DMeta, Text>),
   (MapPair<TUnorderedMap<DMeta, Text>, DMeta, Text>),
   (MapPair<TOrderedMap<DMeta, Text>, DMeta, Text>),
   (MapPair<OrderedMap, DMeta, Text>)
) {
   using T = typename TestType::Container;
   using K = typename TestType::Key;
   using V = typename TestType::Value;
   using Pair = TPair<K, V>;

   GIVEN("Map instance initialized with 10 specific pairs for the corner case") {
      const Pair pairs[10] = {
         {MetaOf<VulkanLayer>(),       "VulkanLayer"},
         {MetaOf<VulkanRenderer>(),    "VulkanRenderer"},
         {MetaOf<VulkanCamera>(),      "VulkanCamera"},
         {MetaOf<Platform>(),          "Platform"},
         {MetaOf<Vulkan>(),            "Vulkan"},
         {MetaOf<Window>(),            "Window"},
         {MetaOf<VulkanLight>(),       "VulkanLight"},
         {MetaOf<Monitor>(),           "Monitor"},
         {MetaOf<VulkanRenderable>(),  "VulkanRenderable"},
         {MetaOf<Cursor>(),            "Cursor"}
      };

      T map {pairs};

      WHEN("Removing around-the-end elements by value (corner case)") {
         Count removed = 0;
         removed += map.RemoveValue("VulkanRenderer");
         removed += map.RemoveValue("VulkanCamera");
         removed += map.RemoveValue("Vulkan");
         removed += map.RemoveValue("VulkanRenderable");
         removed += map.RemoveValue("VulkanLight");
         removed += map.RemoveValue("VulkanLayer");

         REQUIRE(removed == 6);
         REQUIRE(map.GetCount() == 4);

         REQUIRE_THROWS(map[MetaOf<VulkanLayer>()] == "");
         REQUIRE_THROWS(map[MetaOf<VulkanRenderer>()] == "");
         REQUIRE_THROWS(map[MetaOf<VulkanCamera>()] == "");
         REQUIRE       (map[MetaOf<Platform>()] == "Platform");
         REQUIRE_THROWS(map[MetaOf<Vulkan>()] == "");
         REQUIRE       (map[MetaOf<Window>()] == "Window");
         REQUIRE_THROWS(map[MetaOf<VulkanLight>()] == "");
         REQUIRE       (map[MetaOf<Monitor>()] == "Monitor");
         REQUIRE_THROWS(map[MetaOf<VulkanRenderable>()] == "");
         REQUIRE       (map[MetaOf<Cursor>()] == "Cursor");
      }

      WHEN("Removing around-the-end elements by key (corner case)") {
         Count removed = 0;
         removed += map.RemoveKey(MetaOf<VulkanRenderer>());
         removed += map.RemoveKey(MetaOf<VulkanCamera>());
         removed += map.RemoveKey(MetaOf<Vulkan>());
         removed += map.RemoveKey(MetaOf<VulkanRenderable>());
         removed += map.RemoveKey(MetaOf<VulkanLight>());
         removed += map.RemoveKey(MetaOf<VulkanLayer>());

         REQUIRE(removed == 6);
         REQUIRE(map.GetCount() == 4);

         REQUIRE_THROWS(map[MetaOf<VulkanLayer>()] == "");
         REQUIRE_THROWS(map[MetaOf<VulkanRenderer>()] == "");
         REQUIRE_THROWS(map[MetaOf<VulkanCamera>()] == "");
         REQUIRE       (map[MetaOf<Platform>()] == "Platform");
         REQUIRE_THROWS(map[MetaOf<Vulkan>()] == "");
         REQUIRE       (map[MetaOf<Window>()] == "Window");
         REQUIRE_THROWS(map[MetaOf<VulkanLight>()] == "");
         REQUIRE       (map[MetaOf<Monitor>()] == "Monitor");
         REQUIRE_THROWS(map[MetaOf<VulkanRenderable>()] == "");
         REQUIRE       (map[MetaOf<Cursor>()] == "Cursor");
      }
   }
}

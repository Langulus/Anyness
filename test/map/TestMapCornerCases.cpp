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
#include <Anyness/TUnorderedMap.hpp>
#include <Anyness/TOrderedMap.hpp>
#include <Anyness/UnorderedMap.hpp>
#include <Anyness/OrderedMap.hpp>
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
         {MetaData::Of<VulkanLayer>(),       "VulkanLayer"},
         {MetaData::Of<VulkanRenderer>(),    "VulkanRenderer"},
         {MetaData::Of<VulkanCamera>(),      "VulkanCamera"},
         {MetaData::Of<Platform>(),          "Platform"},
         {MetaData::Of<Vulkan>(),            "Vulkan"},
         {MetaData::Of<Window>(),            "Window"},
         {MetaData::Of<VulkanLight>(),       "VulkanLight"},
         {MetaData::Of<Monitor>(),           "Monitor"},
         {MetaData::Of<VulkanRenderable>(),  "VulkanRenderable"},
         {MetaData::Of<Cursor>(),            "Cursor"}
      };

      T map {pairs};

      WHEN("Removing around-the-end elements by value (corner case)") {
         Count removed {};
         removed += map.RemoveValue("VulkanRenderer");
         removed += map.RemoveValue("VulkanCamera");
         removed += map.RemoveValue("Vulkan");
         removed += map.RemoveValue("VulkanRenderable");
         removed += map.RemoveValue("VulkanLight");
         removed += map.RemoveValue("VulkanLayer");

         THEN("The map should be correct") {
            REQUIRE(removed == 6);
            REQUIRE(map.GetCount() == 4);

            REQUIRE_THROWS(map[MetaData::Of<VulkanLayer>()] == "");
            REQUIRE_THROWS(map[MetaData::Of<VulkanRenderer>()] == "");
            REQUIRE_THROWS(map[MetaData::Of<VulkanCamera>()] == "");
            REQUIRE       (map[MetaData::Of<Platform>()] == "Platform");
            REQUIRE_THROWS(map[MetaData::Of<Vulkan>()] == "");
            REQUIRE       (map[MetaData::Of<Window>()] == "Window");
            REQUIRE_THROWS(map[MetaData::Of<VulkanLight>()] == "");
            REQUIRE       (map[MetaData::Of<Monitor>()] == "Monitor");
            REQUIRE_THROWS(map[MetaData::Of<VulkanRenderable>()] == "");
            REQUIRE       (map[MetaData::Of<Cursor>()] == "Cursor");
         }
      }

      WHEN("Removing around-the-end elements by key (corner case)") {
         Count removed {};
         removed += map.RemoveKey(MetaData::Of<VulkanRenderer>());
         removed += map.RemoveKey(MetaData::Of<VulkanCamera>());
         removed += map.RemoveKey(MetaData::Of<Vulkan>());
         removed += map.RemoveKey(MetaData::Of<VulkanRenderable>());
         removed += map.RemoveKey(MetaData::Of<VulkanLight>());
         removed += map.RemoveKey(MetaData::Of<VulkanLayer>());

         THEN("The map should be correct") {
            REQUIRE(removed == 6);
            REQUIRE(map.GetCount() == 4);

            REQUIRE_THROWS(map[MetaData::Of<VulkanLayer>()] == "");
            REQUIRE_THROWS(map[MetaData::Of<VulkanRenderer>()] == "");
            REQUIRE_THROWS(map[MetaData::Of<VulkanCamera>()] == "");
            REQUIRE       (map[MetaData::Of<Platform>()] == "Platform");
            REQUIRE_THROWS(map[MetaData::Of<Vulkan>()] == "");
            REQUIRE       (map[MetaData::Of<Window>()] == "Window");
            REQUIRE_THROWS(map[MetaData::Of<VulkanLight>()] == "");
            REQUIRE       (map[MetaData::Of<Monitor>()] == "Monitor");
            REQUIRE_THROWS(map[MetaData::Of<VulkanRenderable>()] == "");
            REQUIRE       (map[MetaData::Of<Cursor>()] == "Cursor");
         }
      }
   }
}

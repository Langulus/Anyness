///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include <Anyness/Bytes.hpp>
#include <Anyness/Text.hpp>
#include "Common.hpp"


SCENARIO("Byte manipulation", "[bytes]") {
   static Allocator::State memoryState;

   GIVEN("An empty byte container") {
      Bytes data;

      WHEN("Capacity is reserved, via Allocate()") {
         IF_LANGULUS_MANAGED_MEMORY(Allocator::CollectGarbage());

         data.Reserve(500);
         auto memory = data.GetRaw();

         REQUIRE(not data);
         REQUIRE(data.GetCount() == 0);
         REQUIRE(data.GetReserved() >= 500);

         auto region = data.Extend(10);
         REQUIRE(data.GetCount() == 10);
         REQUIRE(data.GetReserved() >= 500);
         REQUIRE(data.GetRaw() == memory);
         REQUIRE(data.HasAuthority());
         REQUIRE(region.GetCount() == 10);
         REQUIRE(region.GetRaw() == memory);
      }
   }

   GIVEN("A filled byte container") {
      IF_LANGULUS_MANAGED_MEMORY(Allocator::CollectGarbage());

      const int randomStuff[] = { 1, 2, 3, 4, 5 };
      Bytes data {randomStuff};
      auto memory = data.GetRaw();

      REQUIRE(data.GetCount() == 5 * sizeof(int));
      REQUIRE(data.GetReserved() >= 5 * sizeof(int));
      REQUIRE(data.template IsExact<Byte>());
      REQUIRE(data.GetRaw() != nullptr);
      REQUIRE(data.HasAuthority());

      WHEN("Add more bytes") {
         const int moreRandomStuff[] = { 1, 2, 3 };
         data += Bytes {moreRandomStuff};

         REQUIRE(data.GetCount() == 8 * sizeof(int));
         REQUIRE(data.GetReserved() >= 8 * sizeof(int));
         IF_LANGULUS_MANAGED_MEMORY(REQUIRE(data.GetRaw() == memory));
         REQUIRE(data.HasAuthority());
         REQUIRE(data.Is<Byte>());
      }

      WHEN("More byte capacity is reserved") {
         data.Reserve(40);

         REQUIRE(data.GetCount() == 5 * sizeof(int));
         REQUIRE(data.GetReserved() >= 40);
         IF_LANGULUS_MANAGED_MEMORY(REQUIRE(data.GetRaw() == memory));
         REQUIRE(data.HasAuthority());
      }

      WHEN("More byte capacity is reserved, via Extend()") {
         auto region = data.Extend(10);

         REQUIRE(data.GetCount() == 5 * sizeof(int) + 10);
         REQUIRE(data.GetReserved() >= 5 * sizeof(int) + 10);
         IF_LANGULUS_MANAGED_MEMORY(REQUIRE(data.GetRaw() == memory));
         REQUIRE(data.HasAuthority());
         REQUIRE(region.GetCount() == 10);
         REQUIRE(region.GetRaw() == data.GetRaw() + 5 * sizeof(int));
      }

      WHEN("Less capacity is reserved") {
         data.Reserve(2);

         REQUIRE(data.GetCount() == 2);
         REQUIRE(data.GetReserved() >= 5);
         REQUIRE(data.GetRaw() == memory);
         REQUIRE(data.HasAuthority());
      }

      WHEN("Bytes are cleared") {
         data.Clear();

         REQUIRE(data.GetCount() == 0);
         REQUIRE(data.GetReserved() >= 5);
         REQUIRE(data.GetRaw() == memory);
         REQUIRE(data.HasAuthority());
         REQUIRE(data.Is<Byte>());
      }

      WHEN("Bytes are reset") {
         data.Reset();

         REQUIRE(data.GetCount() == 0);
         REQUIRE(data.GetReserved() == 0);
         REQUIRE(data.GetRaw() == nullptr);
         REQUIRE(data.Is<Byte>());
      }

      WHEN("Bytes are copied shallowly") {
         Bytes copy = data;

         REQUIRE(data.GetCount() == copy.GetCount());
         REQUIRE(data.GetReserved() == copy.GetReserved());
         REQUIRE(data.GetRaw() == copy.GetRaw());
         REQUIRE(data.GetType() == copy.GetType());
         REQUIRE(data.HasAuthority());
         REQUIRE(copy.HasAuthority());
         REQUIRE(copy.GetUses() == 2);
         REQUIRE(data.GetUses() == 2);
      }

      WHEN("Bytes are cloned") {
         Bytes copy = Clone(data);

         REQUIRE(data.GetCount() == copy.GetCount());
         REQUIRE(data.GetReserved() == copy.GetReserved());
         REQUIRE(data.GetRaw() != copy.GetRaw());
         REQUIRE(data.GetType() == copy.GetType());
         REQUIRE(data.HasAuthority());
         REQUIRE(copy.HasAuthority());
         REQUIRE(copy.GetUses() == 1);
         REQUIRE(data.GetUses() == 1);
      }

      WHEN("Bytes are reset, then allocated again") {
         const int randomStuff2[] = {4, 5, 6, 7, 8, 9};
         data.Reset();
         data += Bytes {randomStuff2};

         REQUIRE(data.GetCount() == sizeof(int) * 6);
         REQUIRE(data.GetReserved() >= sizeof(int) * 6);
         REQUIRE(data.HasAuthority());
         REQUIRE(data.Is<Byte>());
      }

      WHEN("Bytes are compared") {
         const int randomStuff2[] = {4, 5, 6, 7, 8, 9};

         REQUIRE(data == Bytes {randomStuff});
         REQUIRE(data != Bytes {randomStuff2});
      }
   }

   REQUIRE(memoryState.Assert());
}
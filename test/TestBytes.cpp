///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "Main.hpp"
#include <Anyness/Text.hpp>
#include <catch2/catch.hpp>

/// See https://github.com/catchorg/Catch2/blob/devel/docs/tostring.md        
CATCH_TRANSLATE_EXCEPTION(::Langulus::Exception const& ex) {
   const Text serialized {ex};
   return ::std::string {Token {serialized}};
}

SCENARIO("Byte manipulation", "[bytes]") {

   GIVEN("An empty byte container") {
      Bytes data;

      WHEN("Capacity is reserved, via Allocate()") {
         IF_LANGULUS_MANAGED_MEMORY(Fractalloc.CollectGarbage());

         data.Reserve(500);
         auto memory = data.GetRaw();

         REQUIRE(data.IsEmpty());
         REQUIRE(data.GetCount() == 0);
         REQUIRE(data.GetReserved() >= 500);

         auto region = data.Extend(10);
         THEN("The capacity and size change") {
            REQUIRE(data.GetCount() == 10);
            REQUIRE(data.GetReserved() >= 500);
            REQUIRE(data.GetRaw() == memory);
            REQUIRE(data.HasAuthority());
            REQUIRE(region.GetCount() == 10);
            REQUIRE(region.GetRaw() == memory);
         }
      }
   }

   GIVEN("A filled byte container") {
      IF_LANGULUS_MANAGED_MEMORY(Fractalloc.CollectGarbage());

      const int randomStuff[] = { 1, 2, 3, 4, 5 };
      Bytes data {randomStuff, sizeof(randomStuff)};
      auto memory = data.GetRaw();

      REQUIRE(data.GetCount() == 5 * sizeof(int));
      REQUIRE(data.GetReserved() >= 5 * sizeof(int));
      REQUIRE(data.template IsExact<Byte>());
      REQUIRE(data.GetRaw() != nullptr);
      REQUIRE(data.HasAuthority());

      WHEN("Add more bytes") {
         const int moreRandomStuff[] = { 1, 2, 3 };
         data += Bytes {moreRandomStuff, 3 * sizeof(int)};
         THEN("The size and capacity change, type will never change") {
            REQUIRE(data.GetCount() == 8 * sizeof(int));
            REQUIRE(data.GetReserved() >= 8 * sizeof(int));
            #if LANGULUS_FEATURE(MANAGED_MEMORY)
               REQUIRE(data.GetRaw() == memory);
            #endif
            REQUIRE(data.HasAuthority());
            REQUIRE(data.Is<Byte>());
         }
      }

      WHEN("More byte capacity is reserved") {
         data.Reserve(40);

         THEN("The capacity changes but not the size, memory will move in order to have jurisdiction") {
            REQUIRE(data.GetCount() == 5 * sizeof(int));
            REQUIRE(data.GetReserved() >= 40);
            #if LANGULUS_FEATURE(MANAGED_MEMORY)
               REQUIRE(data.GetRaw() == memory);
            #endif
            REQUIRE(data.HasAuthority());
         }
      }

      WHEN("More byte capacity is reserved, via Extend()") {
         auto region = data.Extend(10);

         THEN("The capacity and size change") {
            REQUIRE(data.GetCount() == 5 * sizeof(int) + 10);
            REQUIRE(data.GetReserved() >= 5 * sizeof(int) + 10);
            #if LANGULUS_FEATURE(MANAGED_MEMORY)
               REQUIRE(data.GetRaw() == memory);
            #endif
            REQUIRE(data.HasAuthority());
            REQUIRE(region.GetCount() == 10);
            REQUIRE(region.GetRaw() == data.GetRaw() + 5 * sizeof(int));
         }
      }

      WHEN("Less capacity is reserved") {
         data.Reserve(2);

         THEN("Capacity is not changed, but count is trimmed; memory will not move, and memory will still be outside jurisdiction") {
            REQUIRE(data.GetCount() == 2);
            REQUIRE(data.GetReserved() >= 5);
            REQUIRE(data.GetRaw() == memory);
            REQUIRE(data.HasAuthority());
         }
      }

      WHEN("Bytes are cleared") {
         data.Clear();

         THEN("Size goes to zero, capacity and type are unchanged") {
            REQUIRE(data.GetCount() == 0);
            REQUIRE(data.GetReserved() >= 5);
            REQUIRE(data.GetRaw() == memory);
            REQUIRE(data.HasAuthority());
            REQUIRE(data.Is<Byte>());
         }
      }

      WHEN("Bytes are reset") {
         data.Reset();

         THEN("Size and capacity goes to zero, type is unchanged, because it's a templated container") {
            REQUIRE(data.GetCount() == 0);
            REQUIRE(data.GetReserved() == 0);
            REQUIRE(data.GetRaw() == nullptr);
            REQUIRE(data.Is<Byte>());
         }
      }

      WHEN("Bytes are copied shallowly") {
         Bytes copy = data;

         THEN("Size and capacity goes to zero, type is unchanged, because it's a templated container") {
            REQUIRE(data.GetCount() == copy.GetCount());
            REQUIRE(data.GetReserved() == copy.GetReserved());
            REQUIRE(data.GetRaw() == copy.GetRaw());
            REQUIRE(data.GetType() == copy.GetType());
            REQUIRE(data.HasAuthority());
            REQUIRE(copy.HasAuthority());
            REQUIRE(copy.GetUses() == 2);
            REQUIRE(data.GetUses() == 2);
         }
      }

      WHEN("Bytes are cloned") {
         Bytes copy = data.Clone();

         THEN("Bytes get copied") {
            REQUIRE(data.GetCount() == copy.GetCount());
            REQUIRE(data.GetReserved() == copy.GetReserved());
            REQUIRE(data.GetRaw() != copy.GetRaw());
            REQUIRE(data.GetType() == copy.GetType());
            REQUIRE(data.HasAuthority());
            REQUIRE(copy.HasAuthority());
            REQUIRE(copy.GetUses() == 1);
            REQUIRE(data.GetUses() == 1);
         }
      }

      WHEN("Bytes are reset, then allocated again") {
         const int randomStuff2[] = {4, 5, 6, 7, 8, 9};
         data.Reset();
         data += Bytes {randomStuff2, sizeof(randomStuff2)};

         THEN("Block manager should reuse the memory") {
            REQUIRE(data.GetCount() == sizeof(int) * 6);
            REQUIRE(data.GetReserved() >= sizeof(int) * 6);
            REQUIRE(data.HasAuthority());
            REQUIRE(data.Is<Byte>());
         }
      }

      WHEN("Bytes are compared") {
         THEN("The results should match") {
            const int randomStuff2[] = {4, 5, 6, 7, 8, 9};
            REQUIRE(data == Bytes {randomStuff, sizeof(randomStuff)});
            REQUIRE(data != Bytes {randomStuff2, sizeof(randomStuff2)});
         }
      }
   }
}
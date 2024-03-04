///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include <Anyness/Text.hpp>
#include <Anyness/Ref.hpp>
#include "Common.hpp"


///                                                                           
TEMPLATE_TEST_CASE("Double-referenced shared pointer", "[TPointer]",
   Ref<int>, Ref<RT>, Ref<Any>
) {
   static Allocator::State memoryState;

   using T = TestType;
   using TT = TypeOf<T>;

   GIVEN("A templated shared pointer") {
      T pointer;
      T pointer2;

      REQUIRE_FALSE(pointer.Get());
      REQUIRE_FALSE(pointer);
      REQUIRE(pointer == pointer2);

      WHEN("Create an instance") {
         pointer.New(5);

         REQUIRE(*pointer == 5);
         REQUIRE(pointer.HasAuthority());
         REQUIRE(pointer.GetUses() == 1);
         if constexpr (CT::Referencable<TT>)
            REQUIRE(pointer->Reference(0) == 1);
      }

      WHEN("Create and copy an instance") {
         pointer.New(5);
         pointer2 = pointer;

         REQUIRE(pointer == pointer2);
         REQUIRE(*pointer == 5);
         REQUIRE(*pointer2 == 5);
         REQUIRE(pointer.HasAuthority());
         REQUIRE(pointer2.HasAuthority());
         REQUIRE(pointer.GetUses() == 2);
         REQUIRE(pointer2.GetUses() == 2);
         if constexpr (CT::Referencable<TT>)
            REQUIRE(pointer->Reference(0) == 2);
      }

      WHEN("Create and move an instance") {
         pointer.New(5);
         pointer2 = ::std::move(pointer);

         REQUIRE_FALSE(pointer);
         REQUIRE(pointer2);
         REQUIRE(*pointer2 == 5);
         REQUIRE_FALSE(pointer.HasAuthority());
         REQUIRE(pointer2.HasAuthority());
         REQUIRE(pointer.GetUses() == 0);
         REQUIRE(pointer2.GetUses() == 1);
         if constexpr (CT::Referencable<TT>)
            REQUIRE(pointer2->Reference(0) == 1);
      }

      WHEN("Overwrite an instance") {
         pointer.New(5);
         IF_LANGULUS_MANAGED_MEMORY(auto backup = pointer.Get());
         pointer2.New(6);
         pointer = pointer2;

         REQUIRE(pointer == pointer2);
         REQUIRE(*pointer == 6);
         REQUIRE(*pointer2 == 6);
         #if LANGULUS_FEATURE(MANAGED_MEMORY)
            REQUIRE(Allocator::CheckAuthority(pointer.GetType(), backup));
            REQUIRE_FALSE(Allocator::Find(pointer.GetType(), backup));
         #endif
         REQUIRE(pointer2.HasAuthority());
         REQUIRE(pointer.HasAuthority());
         REQUIRE(pointer.GetUses() == 2);
         if constexpr (CT::Referencable<TT>)
            REQUIRE(pointer->Reference(0) == 2);
      }

      auto raw = new Decay<TT> {3};
      const auto rawBackUp = raw;

      WHEN("Given an xvalue pointer created via `new` statement") {
         pointer = ::std::move(raw);

         REQUIRE(pointer == rawBackUp);
         REQUIRE(*pointer == *rawBackUp);
         REQUIRE(raw == rawBackUp);
         #if LANGULUS_FEATURE(NEWDELETE)
            REQUIRE(pointer.HasAuthority());
            REQUIRE(pointer.GetReferences() == 2);
         #else
            REQUIRE_FALSE(pointer.HasAuthority());
            if constexpr (CT::Referencable<TT>)
               REQUIRE(pointer->Reference(0) == 1);
         #endif
      }

      #if LANGULUS_FEATURE(NEWDELETE)
         WHEN("Given an immediate xvalue pointer created via `new` statement - a very bad practice, unless LANGULUS_FEATURE(NEWDELETE) is enabled!") {
            pointer = new Decay<TT> {3};

            #if LANGULUS_FEATURE(NEWDELETE)
               REQUIRE(pointer.HasAuthority());
               REQUIRE(pointer.GetReferences() == 2);
            #endif
         }

         WHEN("Given an xvalue pointer and then reset") {
            pointer = ::std::move(raw);
            auto unused = Allocator::Free(pointer.GetType(), raw, 1);
            pointer = nullptr;

            REQUIRE_FALSE(raw->HasAuthority());
            REQUIRE(Allocator::CheckAuthority(pointer.GetType(), raw));
            REQUIRE_FALSE(Allocator::Find(pointer.GetType(), raw));
            REQUIRE_FALSE(pointer.HasAuthority());
         }
      #endif

      WHEN("Given an lvalue pointer") {
         pointer = raw;

         REQUIRE(pointer == raw);
         REQUIRE(*pointer == *raw);
         #if LANGULUS_FEATURE(NEWDELETE)
            REQUIRE(pointer.HasAuthority());
            REQUIRE(pointer.GetReferences() == 2);
         #else
            REQUIRE_FALSE(pointer.HasAuthority());
            if constexpr (CT::Referencable<TT>)
               REQUIRE(pointer->Reference(0) == 1);
         #endif
      }

      #if not LANGULUS_FEATURE(NEWDELETE)
         if constexpr (CT::Referencable<TT>)
            raw->Reference(-1);
         delete raw;
      #endif
   }

   REQUIRE(memoryState.Assert());
}

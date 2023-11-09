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
#include <Anyness/Referenced.hpp>
#include "Common.hpp"


/// Simple type for testing Ref                                              
struct RT : Referenced {
   int data;

   RT(int a) : data{a}{}

   operator const int& () const noexcept {
      return data;
   }
};

///                                                                           
TEMPLATE_TEST_CASE("Shared pointer", "[TPointer]",
   Ptr<int>, Ptr<RT>, Ptr<Any>
) {
   using T = TestType;
   using TT = TypeOf<T>;

   GIVEN("A templated shared pointer") {
      IF_LANGULUS_MANAGED_MEMORY(Allocator::CollectGarbage());

      T pointer;
      T pointer2;

      REQUIRE_FALSE(pointer.Get());
      REQUIRE_FALSE(pointer);
      REQUIRE(pointer == pointer2);

      WHEN("Create an instance") {
         pointer.New(5);

         THEN("Should have exactly one reference and jurisdiction") {
            REQUIRE(**pointer == 5);
            REQUIRE(pointer.HasAuthority());
            REQUIRE(pointer.GetUses() == 1);
         }
      }

      WHEN("Create and copy an instance") {
         pointer.New(5);
         pointer2 = pointer;

         THEN("Should have exactly two references and jurisdiction") {
            REQUIRE(pointer == pointer2);
            REQUIRE(**pointer == 5);
            REQUIRE(**pointer2 == 5);
            REQUIRE(pointer.HasAuthority());
            REQUIRE(pointer2.HasAuthority());
            REQUIRE(pointer.GetUses() == 2);
            REQUIRE(pointer2.GetUses() == 2);
         }
      }

      WHEN("Create and move an instance") {
         pointer.New(5);
         pointer2 = ::std::move(pointer);

         THEN("Should have exactly one reference and jurisdiction") {
            REQUIRE_FALSE(pointer);
            REQUIRE(pointer2);
            REQUIRE(**pointer2 == 5);
            REQUIRE_FALSE(pointer.HasAuthority());
            REQUIRE(pointer2.HasAuthority());
            REQUIRE(pointer.GetUses() == 0);
            REQUIRE(pointer2.GetUses() == 1);
         }
      }

      WHEN("Overwrite an instance") {
         pointer.New(5);
         IF_LANGULUS_MANAGED_MEMORY(auto backup = pointer.Get());
         pointer2.New(6);
         pointer = pointer2;

         THEN("Old memory should be freed, but still be in jurisdiction") {
            REQUIRE(pointer == pointer2);
            REQUIRE(**pointer == 6);
            REQUIRE(**pointer2 == 6);
            #if LANGULUS_FEATURE(MANAGED_MEMORY)
               REQUIRE(Allocator::CheckAuthority(pointer.GetType(), backup));
               REQUIRE_FALSE(Allocator::Find(pointer.GetType(), backup));
            #endif
            REQUIRE(pointer2.HasAuthority());
            REQUIRE(pointer.HasAuthority());
            REQUIRE(pointer.GetUses() == 2);
         }
      }

      WHEN("Given an xvalue pointer created via `new` statement") {
         auto raw = new Decay<TT> {3};
         const auto rawBackUp = raw;
         pointer = ::std::move(raw);

         THEN("Should have exactly two references and jurisdiction, if NEWDELETE and MANAGED_MEMORY features are enabled") {
            REQUIRE(pointer == rawBackUp);
            REQUIRE(**pointer == *rawBackUp);
            REQUIRE(raw == rawBackUp);
            #if LANGULUS_FEATURE(NEWDELETE)
               REQUIRE(pointer.HasAuthority());
               REQUIRE(pointer.GetReferences() == 2);
            #endif
         }
      }

      WHEN("Given an immediate xvalue pointer created via `new` statement - a very bad practice!") {
         pointer = new Decay<TT> {3};

         THEN("Should have exactly two references and jurisdiction, if NEWDELETE and MANAGED_MEMORY features are enabled") {
         #if LANGULUS_FEATURE(NEWDELETE)
            REQUIRE(pointer.HasAuthority());
            REQUIRE(pointer.GetReferences() == 2);
         #endif
         }
      }

      #if LANGULUS_FEATURE(NEWDELETE)
         WHEN("Given an xvalue pointer and then reset") {
            Allocator::CollectGarbage();
            auto raw = new Decay<TT> {3};
            pointer = ::std::move(raw);
            auto unused = Allocator::Free(pointer.GetType(), raw, 1);
            pointer = nullptr;

            THEN("Should have released the resources") {
               REQUIRE_FALSE(raw->HasAuthority());
               REQUIRE(Allocator::CheckAuthority(pointer.GetType(), raw));
               REQUIRE_FALSE(Allocator::Find(pointer.GetType(), raw));
               REQUIRE_FALSE(pointer.HasAuthority());
            }
         }
      #endif

      WHEN("Given an lvalue pointer") {
         const auto raw = new Decay<TT> {4};
         pointer = raw;

         THEN("Should have exactly one reference and jurisdiction") {
            REQUIRE(pointer == raw);
            REQUIRE(**pointer == *raw);
            #if LANGULUS_FEATURE(NEWDELETE)
               REQUIRE(pointer.HasAuthority());
               REQUIRE(pointer.GetReferences() == 2);
            #endif
         }
      }
   }
}

///                                                                           
TEMPLATE_TEST_CASE("Double-referenced shared pointer", "[TPointer]",
   Ref<int>, Ref<RT>, Ref<Any>
) {
   using T = TestType;
   using TT = TypeOf<T>;

   GIVEN("A templated shared pointer") {
      IF_LANGULUS_MANAGED_MEMORY(Allocator::CollectGarbage());

      T pointer;
      T pointer2;

      REQUIRE_FALSE(pointer.Get());
      REQUIRE_FALSE(pointer);
      REQUIRE(pointer == pointer2);

      WHEN("Create an instance") {
         pointer.New(5);

         THEN("Should have exactly one reference and jurisdiction") {
            REQUIRE(**pointer == 5);
            REQUIRE(pointer.HasAuthority());
            REQUIRE(pointer.GetUses() == 1);
            if constexpr (CT::Referencable<TT>)
               REQUIRE(pointer->GetReferences() == 1);
         }
      }

      WHEN("Create and copy an instance") {
         pointer.New(5);
         pointer2 = pointer;

         THEN("Should have exactly two references and jurisdiction") {
            REQUIRE(pointer == pointer2);
            REQUIRE(**pointer == 5);
            REQUIRE(**pointer2 == 5);
            REQUIRE(pointer.HasAuthority());
            REQUIRE(pointer2.HasAuthority());
            REQUIRE(pointer.GetUses() == 2);
            REQUIRE(pointer2.GetUses() == 2);
            if constexpr (CT::Referencable<TT>)
               REQUIRE(pointer->GetReferences() == 2);
         }
      }

      WHEN("Create and move an instance") {
         pointer.New(5);
         pointer2 = ::std::move(pointer);

         THEN("Should have exactly one reference and jurisdiction") {
            REQUIRE_FALSE(pointer);
            REQUIRE(pointer2);
            REQUIRE(**pointer2 == 5);
            REQUIRE_FALSE(pointer.HasAuthority());
            REQUIRE(pointer2.HasAuthority());
            REQUIRE(pointer.GetUses() == 0);
            REQUIRE(pointer2.GetUses() == 1);
            if constexpr (CT::Referencable<TT>)
               REQUIRE(pointer2->GetReferences() == 1);
         }
      }

      WHEN("Overwrite an instance") {
         pointer.New(5);
         IF_LANGULUS_MANAGED_MEMORY(auto backup = pointer.Get());
         pointer2.New(6);
         pointer = pointer2;

         THEN("Old memory should be freed, but still be in jurisdiction") {
            REQUIRE(pointer == pointer2);
            REQUIRE(**pointer == 6);
            REQUIRE(**pointer2 == 6);
            #if LANGULUS_FEATURE(MANAGED_MEMORY)
               REQUIRE(Allocator::CheckAuthority(pointer.GetType(), backup));
               REQUIRE_FALSE(Allocator::Find(pointer.GetType(), backup));
            #endif
            REQUIRE(pointer2.HasAuthority());
            REQUIRE(pointer.HasAuthority());
            REQUIRE(pointer.GetUses() == 2);
            if constexpr (CT::Referencable<TT>)
               REQUIRE(pointer->GetReferences() == 2);
         }
      }

      WHEN("Given an xvalue pointer created via `new` statement") {
         auto raw = new Decay<TT> {3};
         const auto rawBackUp = raw;
         pointer = ::std::move(raw);

         THEN("Should have exactly two references and jurisdiction, if NEWDELETE and MANAGED_MEMORY features are enabled") {
            REQUIRE(pointer == rawBackUp);
            REQUIRE(**pointer == *rawBackUp);
            REQUIRE(raw == rawBackUp);
            #if LANGULUS_FEATURE(NEWDELETE)
               REQUIRE(pointer.HasAuthority());
               REQUIRE(pointer.GetReferences() == 2);
            #endif
         }
      }

      WHEN("Given an immediate xvalue pointer created via `new` statement - a very bad practice!") {
         pointer = new Decay<TT> {3};

         THEN("Should have exactly two references and jurisdiction, if NEWDELETE and MANAGED_MEMORY features are enabled") {
         #if LANGULUS_FEATURE(NEWDELETE)
            REQUIRE(pointer.HasAuthority());
            REQUIRE(pointer.GetReferences() == 2);
         #endif
         }
      }

      #if LANGULUS_FEATURE(NEWDELETE)
         WHEN("Given an xvalue pointer and then reset") {
            Allocator::CollectGarbage();
            auto raw = new Decay<TT> {3};
            pointer = ::std::move(raw);
            auto unused = Allocator::Free(pointer.GetType(), raw, 1);
            pointer = nullptr;

            THEN("Should have released the resources") {
               REQUIRE_FALSE(raw->HasAuthority());
               REQUIRE(Allocator::CheckAuthority(pointer.GetType(), raw));
               REQUIRE_FALSE(Allocator::Find(pointer.GetType(), raw));
               REQUIRE_FALSE(pointer.HasAuthority());
            }
         }
      #endif

      WHEN("Given an lvalue pointer") {
         const auto raw = new Decay<TT> {4};
         pointer = raw;

         THEN("Should have exactly one reference and jurisdiction") {
            REQUIRE(pointer == raw);
            REQUIRE(**pointer == *raw);
            #if LANGULUS_FEATURE(NEWDELETE)
               REQUIRE(pointer.HasAuthority());
               REQUIRE(pointer.GetReferences() == 2);
            #endif
         }
      }
   }
}

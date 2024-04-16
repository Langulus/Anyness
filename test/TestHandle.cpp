///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include <Anyness/Any.hpp>
#include "Common.hpp"


/// Create a dense or sparse container by providing simple arguments          
template<class T, class...FROM>
TAny<T> CreateManagedElements(FROM&&...from) {
   static_assert(CT::MakableFrom<Decay<T>, Decay<FROM>...>);
   TAny<Decay<T>> base {DecayCast(from)...};
   if constexpr (CT::Similar<T, Decay<T>>)
      return base;
   else {
      if constexpr (CT::Sparse<T> and CT::Dense<Deptr<T>>) {
         TAny<T> sparse;
         for (auto& item : base)
            sparse << &item;
         return sparse;
      }
      else LANGULUS_ERROR("TODO sparser T");
   }
}

///                                                                           
TEMPLATE_TEST_CASE("Handles from sequential containers", "[handle]",
   int, int*, RT, RT*
) {
   static Allocator::State memoryState;

   using T = TestType;
   using HE = Handle<T>;
   using HL = HandleLocal<T>;

   static_assert(not CT::Defaultable<HE>);
   static_assert(not CT::Defaultable<HL>);

   static constexpr bool SPARSE = CT::Sparse<T>;
   static constexpr bool REFERENCABLE = CT::Referencable<Deptr<T>>;


   GIVEN("A statically typed sequential container") {
      TAny<T> data = CreateManagedElements<T>(665, 666, 667);

      REQUIRE(data.GetCount() == 3);
      REQUIRE(DenseCast(data[0]) == 665);
      REQUIRE(DenseCast(data[1]) == 666);
      REQUIRE(DenseCast(data[2]) == 667);

      Handle<T> h0 = data.GetHandle(0);
      Handle<T> h1 = data.GetHandle(1);
      Handle<T> h2 = data.GetHandle(2);

      REQUIRE(h0.GetEntry());
      REQUIRE(h0.GetEntry() == h1.GetEntry());
      REQUIRE(h0.GetEntry() == h2.GetEntry());
      REQUIRE(h0.GetEntry() == h2.GetEntry());

      if constexpr (SPARSE)
         REQUIRE(h0.GetEntry()->GetUses() == 3);
      else
         REQUIRE(h0.GetEntry()->GetUses() == 1);

      if constexpr (REFERENCABLE) {
         REQUIRE(DenseCast(h0.Get()).Reference(0) == 1);
         REQUIRE(DenseCast(h1.Get()).Reference(0) == 1);
         REQUIRE(DenseCast(h2.Get()).Reference(0) == 1);

         REQUIRE(DenseCast(h0.Get()).destroyed == false);
         REQUIRE(DenseCast(h1.Get()).destroyed == false);
         REQUIRE(DenseCast(h2.Get()).destroyed == false);
      }

      WHEN("An element is taken out of the container and moved into a local handle") {
         const T h0p = h0.Get();
         const Allocation* const h0e = h0.GetEntry();

         HandleLocal<T> local = Move(h0);

         if constexpr (REFERENCABLE) {
            if constexpr (SPARSE) {
               REQUIRE(h0.Get() == nullptr);
               REQUIRE(h0.GetEntry() == nullptr);

               REQUIRE(local.Get() == h0p);
               REQUIRE(local.GetEntry() == h0e);
               REQUIRE(local.Get()->Reference(0) == 1);
               REQUIRE(local.GetEntry()->GetUses() == 2);
               REQUIRE(local.Get()->data == h0p->data);
               REQUIRE(local.Get()->destroyed == false);
               REQUIRE(local.Get()->moved_in == false);
               REQUIRE(local.Get()->moved_out == false);
            }
            else {
               REQUIRE(h0.Get().Reference(0) == 1);
               REQUIRE(h0.Get().destroyed == false);
               REQUIRE(h0.Get().moved_in == false);
               REQUIRE(h0.Get().moved_out == true);

               REQUIRE(local.Get().Reference(0) == 1);
               REQUIRE(local.Get().data == h0p.data);
               REQUIRE(local.GetEntry() == nullptr);
               REQUIRE(local.Get().destroyed == false);
               REQUIRE(local.Get().moved_in == true);
               REQUIRE(local.Get().moved_out == false);
            }
         }
         else {
            if constexpr (SPARSE) {
               REQUIRE(h0.Get() == nullptr);
               REQUIRE(h0.GetEntry() == nullptr);

               REQUIRE(local.Get() == h0p);
               REQUIRE(local.GetEntry() == h0e);
               REQUIRE(local.GetEntry()->GetUses() == 2);
            }
            else {
               REQUIRE(local.Get() == h0p);
               REQUIRE(local.GetEntry() == nullptr);
            }
         }
      }
   }

   REQUIRE(memoryState.Assert());
}

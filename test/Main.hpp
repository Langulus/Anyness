///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include <Anyness/Many.hpp>

using namespace Langulus;
using namespace Langulus::Anyness;

//#define LANGULUS_STD_BENCHMARK

//#ifdef LANGULUS_STD_BENCHMARK
//#define CATCH_CONFIG_ENABLE_BENCHMARKING
//#endif


/// Just a bank container, used to contain owned items                        
extern TMany<Many> BANK;

using uint = unsigned int;
template<class T>
using some = std::vector<T>;

template<class L, class R>
struct TypePair {
   using LHS = L;
   using RHS = R;
};

/// Used to configure a map test                                              
///   @param C - the map type we're testing                                   
///   @param K - the tested key type                                          
///   @param V - the tested value type                                        
///   @param MANAGED - true to contain memory allocated by our manager        
template<class C, class K, class V, bool MANAGED = false>
struct MapTest {
   using Container = C;
   using Key = K;
   using Value = V;
   static constexpr bool Managed = MANAGED;
};

/// Type for testing hashing consistency between two containers               
///   @tparam K - the left container                                          
///   @tparam V - the right container                                         
template<class K, class V>
struct HashTest {
   using Key = K;
   using Value = V;
};


/// Create a dense element, on the stack                                      
///   @tparam T - type of element we're creating                              
///   @param e - the data we'll use to initialize an instance of T            
///   @return the new instance of T                                           
template<CT::Dense T, bool = false>
T CreateElement(const auto& e) {
   T element;
   if constexpr (CT::Same<T, decltype(e)>)
      element = e;
   else if constexpr (not CT::Same<T, Block<>>)
      element = Decay<T> {e};
   else {
      element = Block<> {};
      element.Insert(e);
   }

   return element;
}

/// Create a sparse element, on the heap                                      
///   @tparam T - type of element we're creating                              
///   @tparam MANAGED - whether we'll have authority over the pointer or not  
///   @param e - the data we'll use to initialize an instance of T            
///   @return pointer to the new instance of T                                
template<CT::Sparse T, bool MANAGED = false>
T CreateElement(const auto& e) {
   void* element;

   if constexpr (not MANAGED) {
      // Create a pointer that is guaranteed to not be owned by the     
      // memory manager. Notice we don't use 'new' operator here,       
      // because it is weakly linked, and can be overriden to use our   
      // memory manager.                                                
      if constexpr (not CT::Same<T, Block<>>) {
         element = malloc(sizeof(Decay<T>));
         new (element) Decay<T> {e};
      }
      else {
         element = malloc(sizeof(Block<>));
         new (element) Block<> {};
         static_cast<Block<>*>(element)->Insert(e);
      }
   }
   else {
      // Create a pointer owned by the memory manager                   
      auto& container = BANK.Emplace(IndexBack);

      if constexpr (not CT::Same<T, Block<>>) {
         container << Decay<T> {e};
         element = container.GetRaw();
      }
      else {
         container << e;
         element = &container;
      }
   }

   return static_cast<T>(element);
}

/// Create a test pair                                                        
///   @tparam P - the pair type                                               
///   @tparam K - the pair key type                                           
///   @tparam V - the pair value type                                         
///   @tparam MANAGED - whether or not we have auhtority over the data        
///   @param key - the key initialization data                                
///   @param value - the value initialization data                            
///   @return the pair                                                        
template<class P, class K, class V, bool MANAGED = false>
P CreatePair(const auto& key, const auto& value) {
   return P {
      CreateElement<K, MANAGED>(key),
      CreateElement<V, MANAGED>(value)
   };
}

/// Destroy a test pair created via CreatePair                                
///   @tparam MANAGED - was it created by the memory manager?                 
///   @param pair - the pair to destroy                                       
template<bool MANAGED = false>
void DestroyPair(auto& pair) {
   using P = Deref<decltype(pair)>;

   if constexpr (not MANAGED) {
      if constexpr (requires { pair.mKey; }) {
         if constexpr (CT::Typed<P>) {
            // It's a statically typed langulus pair                    
            using K = typename P::Key;
            using V = typename P::Value;

            if constexpr (CT::Sparse<K>) {
               if constexpr (CT::Referencable<Deptr<K>>)
                  REQUIRE(pair.mKey->Reference(-1) == 0);
               if constexpr (CT::Destroyable<Decay<K>>)
                  pair.mKey->~Decay<K>();
               free(pair.mKey.Get());
            }

            if constexpr (CT::Sparse<V>) {
               if constexpr (CT::Referencable<Deptr<V>>)
                  REQUIRE(pair.mValue->Reference(-1) == 0);
               if constexpr (CT::Destroyable<Decay<V>>)
                  pair.mValue->~Decay<V>();
               free(pair.mValue.Get());
            }
         }
      }
      else if constexpr (requires { pair.first; }) {
         // It's an std::pair                                           
         using K = decltype(pair.first);
         using V = decltype(pair.second);

         if constexpr (CT::Sparse<K>) {
            if constexpr (CT::Referencable<Deptr<K>>)
               REQUIRE(pair.first->Reference(-1) == 0);
            if constexpr (CT::Destroyable<Decay<K>>)
               pair.first->~Decay<K>();
            free(pair.first);
         }

         if constexpr (CT::Sparse<V>) {
            if constexpr (CT::Referencable<Deptr<V>>)
               REQUIRE(pair.second->Reference(-1) == 0);
            if constexpr (CT::Destroyable<Decay<V>>)
               pair.second->~Decay<V>();
            free(pair.second);
         }
      }
      else LANGULUS_ERROR("What kind of pair is this? Are you making stuff up?");
   }
   else BANK.Reset();
}

/// Simple type for testing Referenced types                                  
struct RT : Referenced {
   int data;
   const char* t;
   bool destroyed = false;
   bool copied_in = false;
   bool moved_in = false;
   bool moved_out = false;

   RT() : data {0}, t {nullptr} {}
   RT(int a) : data {a}, t {nullptr} {}
   RT(const char* tt) : data(0), t {tt} {}
   RT(const RT& rhs) : data(rhs.data), t {rhs.t}, copied_in {true} { }
   RT(RT&& rhs) : data(rhs.data), t {rhs.t}, moved_in {true} { rhs.moved_out = true; }
   ~RT() {
      destroyed = true;
   }

   RT& operator = (const RT& rhs) {
      data = rhs.data;
      t = rhs.t;
      copied_in = true;
      moved_in = moved_out = false;
      return *this;
   }

   RT& operator = (RT&& rhs) {
      data = rhs.data;
      t = rhs.t;
      copied_in = false;
      moved_in = true;
      moved_out = false;
      rhs.copied_in = false;
      rhs.moved_in = false;
      rhs.moved_out = true;
      return *this;
   }

   operator const int& () const noexcept {
      return data;
   }
};
///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TUnorderedMap.hpp"

namespace Langulus::Anyness
{

   /// Usage styles for TFactory                                              
   enum class FactoryUsage {
      Default,		// Default factories aggregate duplicated items       
      Unique		// Unique factories never duplicate items (a set)     
   };

   
   ///                                                                        
   ///   Factory element                                                      
   ///                                                                        
   template<class FACTORY>
   struct ProducibleFrom {
      LANGULUS(PRODUCER) typename FACTORY::Producer;

      ProducibleFrom() = delete;
      ProducibleFrom(const ProducibleFrom&) = delete;
      ProducibleFrom(ProducibleFrom&&) = delete;

      /// The only allowed element constructor                                
      ///   @param factory - the factory who owns the T instance              
      ///   @param descriptor - the element descriptor, used for hashing      
      ProducibleFrom(FACTORY* factory, const Any& descriptor)
         : mFactory {factory}
         , mReferences {1}
         , mDescriptor {descriptor}
         , mHash {descriptor.GetHash()} {}

      ~ProducibleFrom() noexcept {
         // Very important, will be still in use after destruction      
         mReferences = 0;
      }

   protected:
      friend FACTORY;

      union {
         // When element is in use, this pointer points to the          
         // factory who produced, and owns the element                  
         FACTORY* mFactory;

         // When element is not in use, this pointer points to the      
         // next free entry in the factory                              
         typename FACTORY::MemberType* mNextFreeElement;
      };

      // Counts the uses of a factory element                           
      // If zero, element is unused, and mNextFreeElement is set        
      // Still in use after destruction of Element                      
      Count mReferences;

      // The descriptor used for hashing, and element identification    
      // Not valid if mReferences is zero                               
      Any mDescriptor;

      // Precomputed hash                                               
      Hash mHash;
   };

   template<class T>
   concept FactoryProducible = CT::Producible<T> && !CT::Abstract<T> && CT::Dense<T> && CT::Referencable<T>;


   ///                                                                        
   ///   Factory container                                                    
   ///                                                                        
   ///   Basically a templated container used to contain, produce, but most   
   /// importantly reuse memory. The factory can contain only reference-      
   /// counted types, because elements are forbidden to move, and are reused  
   /// in-place. Additionally, the factory also internally utilizes a hashmap 
   /// to quickly find relevant elements. Items are laid out serially, so     
   /// that iteration is as fast and cache-friendly as possible.              
   ///                                                                        
   ///	By specifying a FactoryUsage::Unique usage, you're essentially       
   /// making a set of the produced resources, never duplicating same         
   /// creations twice. It is highly recommended in such cases, to make the   
   /// produced item hashable and implement a satisfyingly fast compare       
   /// operators, to avoid	huge overheads.                                    
   ///                                                                        
   template<FactoryProducible T, FactoryUsage USAGE = FactoryUsage::Default>
   class TFactory {
   public:
      /// Makes the TFactory CT::Typed                                        
      using MemberType = T;
      using Producer = CT::ProducerOf<T>;

   protected:
      // Each factory is bound to a producer instance                   
      // Every produced T will also be bound to that instance           
      // If factory moved, all contents will be remapped to the new     
      // instance                                                       
      Producer* const mFactoryOwner;
      
      // Elements are allocated here, so they are cache-friendly and    
      // iterated fast, rarely ever moving                              
      TAny<T> mData;

      // The start of the reusable chain                                
      T* mReusable {};

      // A hash map for fast retrieval of elements                      
      TUnorderedMap<Hash, TAny<T*>> mHashmap;

   protected:
      void Destroy(T*);

   public:
      /// Factories can't be default-, move- or copy-constructed              
      /// We must guarantee that mFactoryOwner is always valid, and move is   
      /// allowed only via assignment, on a previously initialized factory    
      /// This is needed, because elements must be remapped to a new valid    
      /// owner upon move                                                     
      TFactory() = delete;
      TFactory(const TFactory&) = delete;
      TFactory(TFactory&&) = delete;

      TFactory(Producer*);
      TFactory& operator = (TFactory&&) noexcept;

   public:
      void Reset();

      template<class... A>
      NOD() T* Create(const Any&, A&&...);
   };

   template<FactoryProducible T>
   using TFactoryUnique = TFactory<T, FactoryUsage::Unique>;

} // namespace Langulus::Anyness

#include "TFactory.inl"
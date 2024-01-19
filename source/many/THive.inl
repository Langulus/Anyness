///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TFactory.hpp"

#define TEMPLATE()   template<class T, FactoryUsage USAGE>
#define FACTORY()    TFactory<T, USAGE>


namespace Langulus::Flow
{

   /// Move a produced item                                                   
   ///   @param other - the item to move                                      
   template<class T> LANGULUS(INLINED)
   ProducedFrom<T>::ProducedFrom(ProducedFrom&& other)
      : ProducedFrom {Move(other)} {}

   /// Semantic construction                                                  
   ///   @param other - semantic and element to initialize with               
   template<class T> template<template<class> class S>
   requires CT::Semantic<S<Neat>> LANGULUS(INLINED)
   ProducedFrom<T>::ProducedFrom(S<ProducedFrom<T>>&& other)
      // mProducer intentionally not overwritten                        
      : mDescriptor {S<Neat> {other->mDescriptor}} {}

   /// Construct a produced item                                              
   ///   @param producer - the item's producer                                
   ///   @param neat - the item's neat descriptor                             
   template<class T> LANGULUS(INLINED)
   ProducedFrom<T>::ProducedFrom(T* producer, const Neat& neat)
      : mDescriptor {neat}
      , mProducer {producer} {
      LANGULUS_ASSUME(DevAssumes, producer, "Invalid producer");
      // Remove parents, as they're ignored on hashing and comparison   
      // - they can create circular dependencies, that we best avoid    
      mDescriptor.RemoveTrait<Traits::Parent, true>();
   }

   /// Get the normalized descriptor of the produced item                     
   ///   @return the normalized descriptor                                    
   template<class T> LANGULUS(INLINED)
   const Neat& ProducedFrom<T>::GetNeat() const noexcept {
      return mDescriptor;
   }

   /// Get the hash of the normalized descriptor (cached and efficient)       
   ///   @return the hash                                                     
   template<class T> LANGULUS(INLINED)
   Hash ProducedFrom<T>::GetHash() const noexcept {
      return mDescriptor.GetHash();
   }

   /// Return the producer of the item (a.k.a. the owner of the factory)      
   ///   @return a pointer to the producer instance                           
   template<class T> LANGULUS(INLINED)
   T* ProducedFrom<T>::GetProducer() const noexcept {
      return mProducer;
   }


   /// Constructor for descriptor-constructible element                       
   ///   @param factory - the factory who owns the T instance                 
   ///   @param neat - element descriptor, used to construct the element      
   TEMPLATE() LANGULUS(INLINED)
   FACTORY()::Element::Element(TFactory* factory, const Neat& neat)
      : mFactory {factory}
      , mData {factory->mFactoryOwner, neat} {}

   /// Semantic construction                                                  
   ///   @param other - semantic and element to initialize with               
   TEMPLATE() template<template<class> class S>
   requires CT::SemanticMakableAlt<S<T>> LANGULUS(INLINED)
   FACTORY()::Element::Element(S<Element>&& other)
      : mFactory {other->mFactory}
      , mData {S<T> {other->mData}} {}


   /// Construction of a factory                                              
   ///   @param owner - the factory owner                                     
   TEMPLATE() LANGULUS(INLINED)
   FACTORY()::TFactory(Producer* owner)
      : mFactoryOwner {owner} {}

   /// Factory destructor                                                     
   /// Checks if all elements are referenced exactly once before destruction  
   /// if safe mode is enabled                                                
   TEMPLATE() LANGULUS(INLINED)
   FACTORY()::~TFactory() {
      Reset();
   }

   /// Move-assignment remaps all elements to the new instance owner          
   ///   @attention notice how mFactoryOwner never changes on both sides      
   ///   @param other - the factory to move                                   
   TEMPLATE() LANGULUS(INLINED)
   FACTORY()& FACTORY()::operator = (TFactory&& other) noexcept {
      mData = Move(other.mData);
      mHashmap = Move(other.mHashmap);
      mReusable = other.mReusable;
      mCount = other.mCount;
      other.mCount = 0;
      other.mReusable = nullptr;
      for (auto& item : mData)
         item.mFactory = this;
      return *this;
   }

   /// Reset the factory                                                      
   TEMPLATE() LANGULUS(INLINED)
   void FACTORY()::Reset() {
      if (not mData.IsAllocated())
         return;

      mHashmap.Reset();

      // Destroy only elements that have a single reference             
      // Some of the elements might be used in other modules, and their 
      // destruction, as well as the destruction of the mData.mEntry    
      // will commence automatically after their use have ceased        
      auto raw = mData.GetRaw();
      const auto rawEnd = mData.GetRawEnd();

      while (raw != rawEnd) {
         if (raw->mData.GetReferences() > 1) {
            // The element is probably used from another module         
            // This is not an error, we do not destroy the element      
            // We make sure, that this factory no longer owns it        
            Logger::Warning("Unable to destroy ", raw->mData, ", it has ",
               raw->mData.GetReferences(), " uses instead of 1");
            raw->mData.Free();
         }
         else {
            // Destroy the element, here was its last use               
            raw->~Element();
         }

         ++raw;
      }

      // Make sure no more destructors are called upon Any::Reset()     
      static_cast<Block&>(mData).mCount = 0;
      mData.Reset();
      mReusable = nullptr;
      mCount = 0;
   }

   /// Reset the factory                                                      
   TEMPLATE() LANGULUS(INLINED)
   bool FACTORY()::IsEmpty() const noexcept {
      return mCount == 0;
   }

   /// Explicit bool cast operator, for use in if statements                  
   ///   @return true if block contains at least one valid element            
   TEMPLATE() LANGULUS(INLINED)
   constexpr FACTORY()::operator bool() const noexcept {
      return not IsEmpty();
   }

#if LANGULUS(SAFE)
   /// Dump the factory to the log                                            
   TEMPLATE()
   void FACTORY()::Dump() const {
      const auto scope = Logger::Special("--------- FACTORY DUMP FOR ", 
         MetaOf<FACTORY()>(), " (", mData.GetUses(), " references): ",
         Logger::Tabs {}
      );

      Count counter {};
      auto raw = mData.GetRaw();
      const auto rawEnd = mData.GetRawEnd();
      while (raw != rawEnd) {
         if (not raw->mData.GetReferences())
            continue;

         Logger::Info(counter, "] ", raw->mData,
            ", ", raw->mData.GetReferences(), " references");
         ++raw;
      }
   }
#endif

   /// Find an element with the provided hash and descriptor                  
   ///   @param descriptor - the normalized descriptor for the element        
   ///   @return the found element, or nullptr if not found                   
   TEMPLATE() LANGULUS(INLINED)
   typename FACTORY()::Element* FACTORY()::Find(const Neat& descriptor) const {
      const auto hash = descriptor.GetHash();
      const auto found = mHashmap.Find(hash);
      if (found) {
         for (auto candidate : mHashmap.GetValue(found)) {
            if (candidate->mData.GetNeat() != descriptor)
               continue;

            // Found                                                    
            return candidate;
         }
      }
      
      // Not found                                                      
      return nullptr;
   }

   /// Create/Destroy element(s) inside the factory                           
   ///   @param verb - the creation verb                                      
   TEMPLATE()
   void FACTORY()::Create(Verb& verb) {
      verb.ForEachDeep(
         [&](const Construct& construct) {
            // For each construct...                                    
            if (not MetaOf<T>()->CastsTo(construct.GetType()))
               return;
            
            auto count = static_cast<int>(
               ::std::floor(construct.GetCharge().mMass * verb.GetMass())
            );

            try { CreateInner(verb, count, construct.GetDescriptor()); }
            catch (const Exception& e) {
               Logger::Error(
                  "Unable to ", MetaOf<FACTORY()>(), "::Create `", Logger::Push,
                  Logger::DarkYellow, construct.GetType(), Logger::Pop, '`'
               );
               Logger::Error("Due to exception: ", e);
               return;
            }
         },
         [&](const DMeta& type) {
            // For each type...                                         
            if (not type or not MetaOf<T>()->CastsTo(type))
               return;

            auto count = static_cast<int>(
               ::std::floor(verb.GetMass())
            );

            try { CreateInner(verb, count); }
            catch (const Exception& e) {
               Logger::Error(
                  "Unable to ", MetaOf<FACTORY()>(), "::Create `", Logger::Push,
                  Logger::DarkYellow, type, Logger::Pop, '`'
               );
               Logger::Error("Due to exception: ", e);
               return;
            }
         }
      );
   }

   /// Inner creation/destruction verb                                        
   ///   @param verb - [in/out] the creation/destruction verb                 
   ///   @param count - the number of items to create (or destroy if negative)
   ///   @param neat - element descriptor                                     
   TEMPLATE()
   void FACTORY()::CreateInner(Verb& verb, int count, const Neat& neat) {
      if (count > 0) {
         // Produce amount of compatible constructs                     
         if constexpr (IsUnique) {
            // Check if descriptor matches any of the available         
            const auto found = Find(neat);
            if (found) {
               // The unique construct was found, just return it.       
               // Mass will be ignored, it makes no sense to            
               // create multiple instances if unique                   
               verb << &found->mData;
               return;
            }

            // If reached, nothing was found                            
            // Produce exactly one element with this descriptor         
            // Mass will be ignored, it makes no sense to create        
            // multiple instances if unique                             
            verb << Produce(neat);
         }
         else {
            // Satisfy the required count                               
            while (count >= 1) {
               auto produced = Produce(neat);
               verb << produced;
               --count;
            }
         }
      }
      else if (count < 0) {
         // Destroy amount of compatible constructs                     
         if constexpr (IsUnique) {
            // Check if descriptor matches any of the available         
            const auto found = Find(neat);
            if (found) {
               // The unique construct was found, destroy it            
               // Mass is ignored, there should be exactly one          
               Destroy(found);
               return;
            }
         }
         else {
            // Destroy the required amount of matching items            
            do {
               const auto found = Find(neat);
               if (not found)
                  break;

               Destroy(found);
               ++count;
            }
            while (count < 0);
         }

         verb.Done();
      }
   }

   /// Select/Deselect element(s) inside the factory                          
   ///   @param verb - the selection verb                                     
   TEMPLATE()
   void FACTORY()::Select(Verb& verb) {
      // For each construct or meta compatible with the factory         
      verb.ForEachDeep(
         [&](const Construct& construct) {
            // For each construct...                                    
            if (not MetaDataOf<T>()->CastsTo(construct.GetType()))
               return;

            TODO();
         },
         [&](const DMeta& type) {
            // For each type...                                         
            if (not type or not MetaDataOf<T>()->CastsTo(type))
               return;

            TODO();
         }
      );
   }

   /// Produce a single T with the given descriptor                           
   ///   @param neat - element descriptor                                     
   ///   @return the produced instance                                        
   TEMPLATE()
   T* FACTORY()::Produce(const Neat& neat) {
      Element* result;

      if (mReusable) {
         // Reuse a slot                                                
         const auto memory = mReusable;
         mReusable = mReusable->mNextFreeElement;
         result = new (memory) Element {this, neat};
      }
      else {
         // Add new slot                                                
         mData.template Emplace<false>(IndexBack, this, neat);
         result = &mData.Last();
      }

      // Register new entry in the hashmap, for fast indexing           
      const auto hash = result->mData.GetHash();
      const auto found = mHashmap.FindIt(hash);
      if (found)
         *found.mValue << result;
      else
         mHashmap.Insert(hash, result);

      ++mCount;
      return &result->mData;
   }

   /// Destroys an element inside factory                                     
   ///   @attention assumes item is a valid pointer, owned by the factory     
   ///   @attention item pointer is no longer valid after this call           
   ///   @param item - element to destroy                                     
   TEMPLATE()
   void FACTORY()::Destroy(Element* item) {
      LANGULUS_ASSUME(DevAssumes, item,
         "Pointer is not valid");
      LANGULUS_ASSUME(DevAssumes, mData.Owns(item),
         "Pointer is not owned by factory");

      // Remove from hashmap                                            
      const auto hash = item->mData.GetHash();
      auto& list = mHashmap[hash];
      list.Remove(item);
      if (not list)
         mHashmap.RemoveKey(hash);

      // Destroy the element                                            
      item->~Element();
      item->mNextFreeElement = mReusable;
      mReusable = item;
      --mCount;
   }


   ///                                                                        
   ///   Iteration                                                            
   ///                                                                        

   /// Get iterator to first element                                          
   ///   @return an iterator to the first element, or end if empty            
   TEMPLATE() LANGULUS(INLINED)
   typename FACTORY()::Iterator FACTORY()::begin() noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid slot, or hit sentinel at the end              
      auto raw = mData.GetRaw();
      const auto rawEnd = mData.GetRawEnd();
      while (raw != rawEnd and 0 == raw->mData.GetReferences())
         ++raw;

      return {raw, rawEnd};
   }

   TEMPLATE() LANGULUS(INLINED)
   typename FACTORY()::ConstIterator FACTORY()::begin() const noexcept {
      return const_cast<FACTORY()*>(this)->begin();
   }

   /// Get iterator to end                                                    
   ///   @return an iterator to the end element                               
   TEMPLATE() LANGULUS(INLINED)
   typename FACTORY()::Iterator FACTORY()::end() noexcept {
      const auto ender = mData.GetRawEnd();
      return {ender, ender};
   }

   TEMPLATE() LANGULUS(INLINED)
   typename FACTORY()::ConstIterator FACTORY()::end() const noexcept {
      return const_cast<FACTORY()*>(this)->end();
   }

   /// Get iterator to the last element                                       
   ///   @return an iterator to the last element, or end if empty             
   TEMPLATE() LANGULUS(INLINED)
   typename FACTORY()::Iterator FACTORY()::last() noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid slot, or hit sentinel at the end              
      auto raw = mData.GetRawEnd() - 1;
      const auto rawEnd = mData.GetRaw() - 1;
      while (raw != rawEnd and 0 == raw->mData.GetReferences())
         --raw;

      return {raw != rawEnd ? raw : mData.GetRawEnd()};
   }

   TEMPLATE() LANGULUS(INLINED)
   typename FACTORY()::ConstIterator FACTORY()::last() const noexcept {
      return const_cast<FACTORY()*>(this)->last();
   }


   #define ITERATOR()   TFactory<T, USAGE>::template TIterator<MUTABLE>
   #define FACTORY_IT() FACTORY()::TIterator<MUTABLE>

   ///                                                                        
   ///   TFactory iterator                                                    
   ///                                                                        

   /// Construct an iterator                                                  
   ///   @param element - the current element                                 
   ///   @param sentinel - the sentinel (equivalent to factory::end())        
   TEMPLATE() template<bool MUTABLE> LANGULUS(INLINED)
   FACTORY_IT()::TIterator(const Element* element, const Element* sentinel) noexcept
      : mElement {element}
      , mSentinel {sentinel} {}

   /// Prefix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the modified iterator                                        
   TEMPLATE() template<bool MUTABLE> LANGULUS(INLINED)
   typename ITERATOR()& FACTORY_IT()::operator ++ () noexcept {
      ++mElement;
      // Skip all invalid entries, until a valid one/sentinel is hit    
      while (mElement != mSentinel and 0 == mElement->mData.GetReferences())
         ++mElement;
      return *this;
   }

   /// Suffix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the previous value of the iterator                           
   TEMPLATE() template<bool MUTABLE> LANGULUS(INLINED)
   typename ITERATOR() FACTORY_IT()::operator ++ (int) noexcept {
      const auto backup = *this;
      operator ++ ();
      return backup;
   }

   /// Compare iterators                                                      
   ///   @param rhs - the other iterator                                      
   ///   @return true if entries match                                        
   TEMPLATE() template<bool MUTABLE> LANGULUS(INLINED)
   bool FACTORY_IT()::operator == (const TIterator& rhs) const noexcept {
      return mElement == rhs.mElement;
   }
      
   /// Iterator access operator                                               
   ///   @return a reference to the element at the current iterator position  
   TEMPLATE() template<bool MUTABLE> LANGULUS(INLINED)
   T& FACTORY_IT()::operator * () const noexcept requires (MUTABLE) {
      return const_cast<T&>(mElement->mData);
   }

   /// Iterator access operator                                               
   ///   @return a reference to the element at the current iterator position  
   TEMPLATE() template<bool MUTABLE> LANGULUS(INLINED)
   const T& FACTORY_IT()::operator * () const noexcept requires (!MUTABLE) {
      return mElement->mData;
   }

   /// Iterator access operator                                               
   ///   @return a reference to the element at the current iterator position  
   TEMPLATE() template<bool MUTABLE> LANGULUS(INLINED)
   T& FACTORY_IT()::operator -> () const noexcept requires (MUTABLE) {
      return const_cast<T&>(mElement->mData);
   }

   /// Iterator access operator                                               
   ///   @return a reference to the element at the current iterator position  
   TEMPLATE() template<bool MUTABLE> LANGULUS(INLINED)
   const T& FACTORY_IT()::operator -> () const noexcept requires (!MUTABLE) {
      return mElement->mData;
   }

} // namespace Langulus::Flow

#undef TEMPLATE
#undef ITERATOR
#undef FACTORY
#undef FACTORY_IT

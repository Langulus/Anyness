///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "../Any.hpp"

#define VERBOSE_TAB(a) //ScopedTab tab; pcLogFuncVerbose << a << tab
#define VERBOSE(a) //Logger::Verbose() << a

namespace Langulus::Anyness
{

   /// Clone any data using RTTI                                              
   /// Nested for each deep inner block                                       
   ///   @param result - [out] the resulting container                        
   ///   @return the number of cloned elements                                
   Count Block::Clone(Block& result) const {
      // Always clone the state, but make it unconstrained              
      result.SetType<false>(mType);
      result.mState += GetUnconstrainedState();
      if (!IsAllocated())
         return 1;

      VERBOSE_TAB("Cloning " << mCount
         << " elements of type " << GetToken() 
         << " (" << GetStride() << " bytes each)");

      if (IsSparse()) {
         // Data is sparse - no escape from this scope                  
         if (!result.IsAllocated()) {
            // Allocate the array of pointers if not allocated yet      
            result.mState -= DataState::Static;
            result.mState -= DataState::Constant;
            result.AllocateFresh(result.RequestSize(mCount));
            result.mCount = mCount;
         }

         // Clone data behind each valid pointer                        
         for (Offset i = 0; i < mCount; ++i) {
            auto& ptrFrom = GetRawSparse()[i];
            auto& ptrTo = result.GetRawSparse()[i];
            if (!ptrFrom.mPointer) {
               // Pointer points nowhere, so just set to nullptr        
               ptrTo.mPointer = nullptr;
               ptrTo.mEntry = nullptr;
               continue;
            }

            // Pointer is pointing somewhere, so clone the data         
            const auto from = GetElementResolved(i);
            Block to;
            from.Clone(to);
            ptrTo.mPointer = to.mRaw;
            ptrTo.mEntry = to.mEntry;
         }

         VERBOSE("Cloned sparse data " << Logger::Red << "(slowest)");
         return mCount;
      }

      // If this is reached, then data is dense                         
      // Iterate each instance in memory                                
      if (!mType->mResolver) {
         // Type is not resolvable, so preallocate safely               
         if (mType->mCloner) {
            // Cloning by placement is available                        
            if (result.IsEmpty()) {
               result.AllocateFresh(result.RequestSize(mCount));
               result.mCount = mCount;
            }

            for (Offset index = 0; index < mCount; ++index) {
               const auto from = GetElement(index);
               auto to = result.GetElement(index);
               result.mType->mCloner(from.mRaw, to.mRaw);
            }

            VERBOSE("Cloned non-resolvable dense by move-placement new " 
               << Logger::DarkYellow << "(slow)");
         }
         else if (mType->mIsPOD) {
            // Just memcpy simple POD data                              
            if (result.IsEmpty()) {
               result.AllocateFresh(result.RequestSize(mCount));
               result.mCount = mCount;
            }
            
            CopyMemory(mRaw, result.mRaw, GetByteSize());

            VERBOSE("Cloned non-resolvable dense POD by memcpy " 
               << Logger::Green << "(fast)");
         }
         else LANGULUS_THROW(Clone,
            "Trying to clone unclonable complex type");
      }
      else {
         const bool preallocatedResult = !result.IsEmpty();
         for (Offset index = 0; index < mCount; ++index) {
            // Type is resolvable, so allocate and clone each resolved  
            const auto from = GetElementResolved(index);
            auto to = Any::FromMeta(from.GetType());

            // Check if a clone operation is available for element      
            if (mType->mCloner) {
               // Placement-clone                                       
               to.AllocateFresh(to.RequestSize(1));
               to.mCount = 1;
               
               from.mType->mCloner(from.mRaw, to.mRaw);
               VERBOSE("Cloned resolved dense by move-placement new" 
                  << Logger::Red << "(slowest)");
            }
            else if (from.mType->mIsPOD) {
               // Just memcpy simple CT::POD data                       
               to.AllocateFresh(to.RequestSize(1));
               to.mCount = 1;
               
               CopyMemory(from.mRaw, to.mRaw, from.GetByteSize());
               VERBOSE("Cloned resolved dense POD by memcpy " 
                  << Logger::DarkYellow << "(slow)");
            }
            else LANGULUS_THROW(Clone,
               "Trying to clone unclonable complex type (resolved)");

            // Commit the cloned, by shallowly copying it to result     
            if (preallocatedResult) {
               auto element = result.GetElementResolved(index);
               to.Copy(element); //TODO move instead?
            }
            else result.InsertBlock<IndexBack>(Abandon(to));
         }
      }

      return mCount;
   }

} // namespace Langulus::Anyness

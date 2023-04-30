///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Bytes.hpp"

namespace Langulus::Anyness
{

   /// Byte container copy-construction                                       
   /// Notice how container is explicitly cast to base class when forwarded   
   /// If that is not done, TAny will use the CT::Deep constructor instead    
   ///   @param other - container to reference                                
   LANGULUS(INLINED)
   Bytes::Bytes(const Bytes& other)
      : Bytes {Langulus::Copy(other)} {}

   /// Byte container move-construction                                       
   ///   @param other - container to move                                     
   LANGULUS(INLINED)
   Bytes::Bytes(Bytes&& other) noexcept
      : Bytes {Langulus::Move(other)} {}

   /// Byte container copy-construction from TAny<Byte> base                  
   ///   @param other - container to reference                                
   LANGULUS(INLINED)
   Bytes::Bytes(const TAny& other)
      : Bytes {Langulus::Copy(other)} {}

   /// Byte container mvoe-construction from TAny<Byte> base                  
   ///   @param other - container to move                                     
   LANGULUS(INLINED)
   Bytes::Bytes(TAny&& other) noexcept
      : Bytes {Langulus::Move(other)} {}

   template<CT::Semantic S>
   LANGULUS(INLINED)
   Bytes::Bytes(S&& other) requires Relevant<S>
      : TAny {other.template Forward<TAny<Byte>>()} {}

   /// Construct manually via raw constant memory pointer and size            
   ///   @param raw - raw memory to reference                                 
   ///   @param size - number of bytes inside 'raw'                           
   LANGULUS(INLINED)
   Bytes::Bytes(const void* raw, const Size& size)
      : Bytes {Langulus::Copy(raw), size} {}

   LANGULUS(INLINED)
   Bytes::Bytes(void* raw, const Size& size)
      : Bytes {Langulus::Copy(raw), size} {}

   template<CT::Semantic S>
   LANGULUS(INLINED)
   Bytes::Bytes(S&& raw, const Size& size) requires (CT::Sparse<TypeOf<S>>)
      : TAny {TAny::From(raw.Forward(), size)} { }

   /// Construct by interpreting anything POD as bytes                        
   ///   @param value - the data to interpret                                 
   template<CT::POD T>
   LANGULUS(INLINED)
   Bytes::Bytes(const T& value) requires CT::Dense<T>
      : Bytes {&value, sizeof(T)} { }

   /// Construct by interpreting a string literal                             
   ///   @param value - the string to interpret                               
   LANGULUS(INLINED)
   Bytes::Bytes(const Token& value)
      : Bytes {value.data(), value.size() * sizeof(Letter)} { }

   /// Construct by interpreting a meta definition                            
   ///   @param value - the meta to interpret                                 
   LANGULUS(INLINED)
   Bytes::Bytes(const RTTI::Meta* value)
      : Bytes {} {
      if (value) {
         *this += Bytes {Count {value->mToken.size()}};
         *this += Bytes {value->mToken};
      }
      else *this += Bytes {Count {0}};
   }

   /// Shallow copy assignment from immutable byte container                  
   ///   @param rhs - the byte container to shallow-copy                      
   ///   @return a reference to this container                                
   LANGULUS(INLINED)
   Bytes& Bytes::operator = (const Bytes& rhs) {
      return operator = (Langulus::Copy(rhs));
   }

   /// Move byte container                                                    
   ///   @param rhs - the container to move                                   
   ///   @return a reference to this container                                
   LANGULUS(INLINED)
   Bytes& Bytes::operator = (Bytes&& rhs) noexcept {
      return operator = (Langulus::Move(rhs));
   }

   /// Move byte container                                                    
   ///   @param rhs - the container to move                                   
   ///   @return a reference to this container                                
   template<CT::Semantic S>
   LANGULUS(INLINED)
   Bytes& Bytes::operator = (S&& rhs) requires Relevant<S> {
      TAny::operator = (rhs.template Forward<TAny>());
      return *this;
   }


   ///                                                                        
   ///   Concatenation                                                        
   ///                                                                        
   
   /// Copy-concatenate with another TAny                                     
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   LANGULUS(INLINED)
   Bytes Bytes::operator + (const Bytes& rhs) const {
      return Concatenate<Bytes>(Langulus::Copy(rhs));
   }

   /// Move-concatenate with another TAny                                     
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   LANGULUS(INLINED)
   Bytes Bytes::operator + (Bytes&& rhs) const {
      return Concatenate<Bytes>(Langulus::Move(rhs));
   }

   /// Move-concatenate with another TAny                                     
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   template<CT::Semantic S>
   LANGULUS(INLINED)
   Bytes Bytes::operator + (S&& rhs) const requires Relevant<S> {
      return Concatenate<Bytes>(rhs.Forward());
   }

   /// Destructive copy-concatenate with another TAny                         
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   LANGULUS(INLINED)
   Bytes& Bytes::operator += (const Bytes& rhs) {
      InsertBlock(rhs);
      return *this;
   }

   /// Destructive move-concatenate with any deep type                        
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   LANGULUS(INLINED)
   Bytes& Bytes::operator += (Bytes&& rhs) {
      InsertBlock(Forward<Bytes>(rhs));
      return *this;
   }

   /// Destructive move-concatenate with any deep type                        
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   template<CT::Semantic S>
   LANGULUS(INLINED)
   Bytes& Bytes::operator += (S&& rhs) requires Relevant<S> {
      InsertBlock(rhs.Forward());
      return *this;
   }
   
   /// Hash the byte sequence                                                 
   ///   @return a hash of the contained byte sequence                        
   LANGULUS(INLINED)
   Hash Bytes::GetHash() const {
      return HashBytes(GetRaw(), static_cast<int>(GetCount()));
   }

   /// Compare with another byte container                                    
   ///   @param other - the byte container to compare with                    
   ///   @return true if both containers are identical                        
   LANGULUS(INLINED)
   bool Bytes::operator == (const Bytes& other) const noexcept {
      return Compare(other);
   }

   /// Clone the byte container                                               
   ///   @return the cloned byte container                                    
   LANGULUS(INLINED)
   Bytes Bytes::Clone() const {
      Bytes result {Disown(*this)};
      if (mCount) {
         const auto request = RequestSize(mCount);
         result.mEntry = Fractalloc.Allocate(nullptr, request.mByteSize);
         LANGULUS_ASSERT(result.mEntry, Allocate, "Out of memory");
         result.mRaw = result.mEntry->GetBlockStart();
         result.mReserved = request.mElementCount;
         CopyMemory(result.mRaw, mRaw, mCount);
      }
      else {
         result.mEntry = nullptr;
         result.mRaw = nullptr;
         result.mReserved = 0;
      }
      
      return Abandon(result);
   }

   /// Pick a constant part of the byte array                                 
   ///   @param start - the starting byte offset                              
   ///   @param count - the number of bytes after 'start' to remain           
   ///   @return a new container that references the original memory          
   LANGULUS(INLINED)
   Bytes Bytes::Crop(const Offset& start, const Count& count) const {
      return TAny::Crop<Bytes>(start, count);
   }

   /// Pick a part of the byte array                                          
   ///   @param start - the starting byte offset                              
   ///   @param count - the number of bytes after 'start' to remain           
   ///   @return a new container that references the original memory          
   LANGULUS(INLINED)
   Bytes Bytes::Crop(const Offset& start, const Count& count) {
      return TAny::Crop<Bytes>(start, count);
   }

   /// Remove a region of bytes                                               
   /// Can't remove bytes from static containers                              
   ///   @param start - the starting offset                                   
   ///   @param end - the ending offset                                       
   ///   @return a reference to the byte container                            
   LANGULUS(INLINED)
   Bytes& Bytes::Remove(const Offset& start, const Offset& end) {
      if (IsEmpty() || IsStatic() || start >= end)
         return *this;
      
      const auto removed = end - start;
      if (end < mCount) {
         // Removing in the middle, so memory has to move               
         MoveMemory(mRaw + start, mRaw + end, mCount - removed);
      }

      mCount -= removed;
      return *this;
   }

   /// Extend the byte sequence, change count, and return the new range       
   /// Static byte containers can't be extended                               
   ///   @param count - the number of bytes to append                         
   ///   @return the extended part - you will not be allowed to resize it     
   LANGULUS(INLINED)
   Bytes Bytes::Extend(const Count& count) {
      return TAny::Extend<Bytes>(count);
   }

} // namespace Langulus::Anyness

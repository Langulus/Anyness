///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Bytes.hpp"
#include "TAny.inl"
#include "TAny-Iteration.inl"


namespace Langulus::Anyness
{

   /// Copy-constructor                                                       
   ///   @param other - container to reference                                
   LANGULUS(INLINED)
   Bytes::Bytes(const Bytes& other)
      : Bytes {Copy(other)} {}

   /// Move-constructor                                                       
   ///   @param other - container to move                                     
   LANGULUS(INLINED)
   Bytes::Bytes(Bytes&& other) noexcept
      : Bytes {Move(other)} {}

   /// Semantic constructor                                                   
   ///   @param other - the container/element and the semantic                
   template<template<class> class S> LANGULUS(INLINED)
   Bytes::Bytes(S<Bytes>&& other) requires CT::Semantic<S<Bytes>> {
      mType = MetaDataOf<Byte>();
      BlockTransfer<Base>(other.Forward());
   }

   /// Construct from anything else                                           
   ///   @param other - an array, range, or anything POD                      
   template<class T> LANGULUS(INLINED)
   Bytes::Bytes(T&& other) requires (CT::Inner::UnfoldMakableFrom<Byte, T>
                                 or (CT::POD<T> and CT::Dense<T>)) {
      UnfoldInsert(IndexFront, Forward<T>(other));
   }

   /// Serialize a meta definition                                            
   ///   @param meta - the definition to serialize                            
   LANGULUS(INLINED)
   Bytes::Bytes(const CT::Meta auto& meta) {
      if (meta) {
         *this += Bytes {Count {meta->mToken.size()}};
         *this += Bytes {meta->mToken};
      }
      else *this += Bytes {Count {0}};
   }

   /// Construct semantically via raw memory pointer, semantic, and size      
   ///   @param raw - raw memory and semantic to use                          
   ///   @param size - number of bytes inside 'raw'                           
   LANGULUS(INLINED)
   Bytes::Bytes(auto&& raw, const Size& size)
      : TAny {TAny::From(Forward<decltype(raw)>(raw), size)} { }

   /// Shallow copy assignment from immutable byte container                  
   ///   @param rhs - the byte container to shallow-copy                      
   ///   @return a reference to this container                                
   LANGULUS(INLINED)
   Bytes& Bytes::operator = (const Bytes& rhs) {
      return operator = (Copy(rhs));
   }

   /// Move byte container                                                    
   ///   @param rhs - the container to move                                   
   ///   @return a reference to this container                                
   LANGULUS(INLINED)
   Bytes& Bytes::operator = (Bytes&& rhs) {
      return operator = (Move(rhs));
   }

   /// Shallow-copy disowned runtime container without referencing contents   
   /// This is a bit slower, because checks type compatibility at runtime     
   ///   @param other - the container to shallow-copy                         
   ///   @return a reference to this container                                
   template<template<class> class S> LANGULUS(INLINED)
   Bytes& Bytes::operator = (S<Bytes>&& other) requires CT::Semantic<S<Bytes>> {
      if (static_cast<const Block*>(this)
       == static_cast<const Block*>(&*other))
            return *this;

      Free<Bytes>();
      new (this) Bytes {other.Forward()};
      return *this;
   }
   
   /// Insert an element, or an array of elements                             
   ///   @param index - the index at which to insert                          
   ///   @param item - the argument to unfold and insert, can be semantic     
   ///   @return the number of inserted elements after unfolding              
   Count Bytes::UnfoldInsert(CT::Index auto index, auto&& item) {
      using S = SemanticOf<decltype(item)>;
      using T = TypeOf<S>;
      
      if constexpr (CT::Array<T>) {
         using DT = Deext<T>;

         if constexpr (CT::POD<DT> and CT::Dense<DT>) {
            // Insert as byte array                                     
            auto data = Block::From(
               reinterpret_cast<const Byte*>(DesemCast(item)),
               sizeof(T)
            );
            InsertContiguousInner<Bytes, void, true, Byte>(index, Copy(data));
            return sizeof(T);
         }
         else {
            // Unfold and serialize elements, one by one                
            Count inserted = 0;
            for (auto& element : DesemCast(item))
               inserted += UnfoldInsert(index + inserted, Copy(element));
            return inserted;
         }
      }
      else if constexpr (CT::POD<T> and CT::Dense<T>) {
         // Insert as byte array                                        
         auto data = Block::From(
            reinterpret_cast<const Byte*>(&DesemCast(item)),
            sizeof(T)
         );
         InsertContiguousInner<Bytes, void, true, Byte>(index, Copy(data));
         return sizeof(T);
      }
      else LANGULUS_ERROR("Unable to insert as bytes");
   }

   ///                                                                        
   ///   Concatenation                                                        
   ///                                                                        
   
   /// Copy-concatenate with another TAny                                     
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   LANGULUS(INLINED)
   Bytes Bytes::operator + (const Bytes& rhs) const {
      return operator + (Copy(rhs));
   }

   /// Move-concatenate with another TAny                                     
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   LANGULUS(INLINED)
   Bytes Bytes::operator + (Bytes&& rhs) const {
      return operator + (Move(rhs));
   }

   /// Move-concatenate with another TAny                                     
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   template<template<class> class S> LANGULUS(INLINED)
   Bytes Bytes::operator + (S<Bytes>&& rhs) const requires CT::Semantic<S<Bytes>> {
      return ConcatBlock<Bytes>(rhs.Forward());
   }

   /// Destructive copy-concatenate with another TAny                         
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   LANGULUS(INLINED)
   Bytes& Bytes::operator += (const Bytes& rhs) {
      return operator += (Copy(rhs));
   }

   /// Destructive move-concatenate with any deep type                        
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   LANGULUS(INLINED)
   Bytes& Bytes::operator += (Bytes&& rhs) {
      return operator += (Move(rhs));
   }

   /// Destructive move-concatenate with any deep type                        
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   template<template<class> class S> LANGULUS(INLINED)
   Bytes& Bytes::operator += (S<Bytes>&& rhs) requires CT::Semantic<S<Bytes>> {
      InsertBlock<Bytes, void, true>(IndexBack, rhs.Forward());
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
         const auto request = RequestSize<Bytes>(mCount);
         result.mEntry = Allocator::Allocate(nullptr, request.mByteSize);
         LANGULUS_ASSERT(result.mEntry, Allocate, "Out of memory");
         result.mRaw = const_cast<Byte*>(result.mEntry->GetBlockStart());
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
      return Block::Crop<Bytes>(start, count);
   }

   /// Pick a part of the byte array                                          
   ///   @param start - the starting byte offset                              
   ///   @param count - the number of bytes after 'start' to remain           
   ///   @return a new container that references the original memory          
   LANGULUS(INLINED)
   Bytes Bytes::Crop(const Offset& start, const Count& count) {
      return Block::Crop<Bytes>(start, count);
   }

   /// Extend the byte sequence, change count, and return the new range       
   /// Static byte containers can't be extended                               
   ///   @param count - the number of bytes to append                         
   ///   @return the extended part - you will not be allowed to resize it     
   LANGULUS(INLINED)
   Bytes Bytes::Extend(const Count& count) {
      return TAny::Extend<Bytes>(count);
   }

   /// Default header constructor                                             
   LANGULUS(INLINED)
   Bytes::Header::Header() noexcept {
      mAtomSize = sizeof(Size);

      // First bit of the flag means the file was written by a big      
      // endian machine                                                 
      mFlags = Default;
      if constexpr (BigEndianMachine)
         mFlags = BigEndian;
      mUnused = 0;
   }

   LANGULUS(INLINED)
   bool Bytes::Header::operator == (const Header& rhs) const noexcept {
      return mAtomSize == rhs.mAtomSize and mFlags == rhs.mFlags;
   }
   

#if LANGULUS_FEATURE(MANAGED_REFLECTION)

   ///                                                                        
   LANGULUS(INLINED)
   void Bytes::RequestMoreBytes(Offset read, Size byteCount, const Loader& loader) const {
      if (read >= GetCount() or GetCount() - read < byteCount) {
         if (not loader)
            LANGULUS_THROW(Access, "Deserializer has no loader");
         loader(const_cast<Bytes&>(*this), byteCount - (GetCount() - read));
      }
   }

   /// Read an atom-sized unsigned integer, based on the provided header      
   ///   @param source - the serialized byte source                           
   ///   @param result - [out] the resulting deserialized number              
   ///   @param read - offset to apply to serialized byte array               
   ///   @param header - environment header                                   
   ///   @param loader - loader for streaming                                 
   ///   @return the number of read bytes from byte container                 
   inline Size Bytes::DeserializeAtom(Offset& result, Offset read, const Header& header, const Loader& loader) const {
      if (header.mAtomSize == 4) {
         // We're deserializing data, that was serialized on a 32-bit   
         // architecture                                                
         uint32_t count4 = 0;
         RequestMoreBytes(read, 4, loader);
         ::std::memcpy(&count4, At(read), 4);
         read += 4;
         result = static_cast<Offset>(count4);
      }
      else if (header.mAtomSize == 8) {
         // We're deserializing data, that was serialized on a 64-bit   
         // architecture                                                
         uint64_t count8 = 0;
         RequestMoreBytes(read, 8, loader);
         ::std::memcpy(&count8, At(read), 8);
         read += 8;
         if (count8 > std::numeric_limits<Offset>::max()) {
            LANGULUS_THROW(Convert,
               "Deserialized atom contains a value "
               "too powerful for your architecture"
            );
         }
         result = static_cast<Offset>(count8);
      }
      else {
         LANGULUS_THROW(Convert,
            "An unknown atomic size was deserialized "
            "from source - is the source corrupted?"
         );
      }

      return read;
   }

   /// A snippet for conveniently deserializing a meta from binary            
   ///   @tparam META - type of meta we're deserializing (deducible)          
   ///   @param result - [out] the deserialized meta goes here                
   ///   @param read - byte offset inside 'from'                              
   ///   @param header - environment header                                   
   ///   @param loader - loader for streaming                                 
   ///   @return number of read bytes                                         
   template<class META>
   Size Bytes::DeserializeMeta(META& result, Offset read, const Header& header, const Loader& loader) const {
      Count count = 0;
      read = DeserializeAtom(count, read, header, loader);
      if (count) {
         RequestMoreBytes(read, count, loader);
         const Token token {GetRawAs<Letter>() + read, count};
         if constexpr (CT::Same<META, DMeta>)
            result = RTTI::GetMetaData(token);
         else if constexpr (CT::Same<META, VMeta>)
            result = RTTI::GetMetaVerb(token);
         else if constexpr (CT::Same<META, TMeta>)
            result = RTTI::GetMetaTrait(token);
         else if constexpr (CT::Same<META, CMeta>)
            result = RTTI::GetMetaConstant(token);
         else
            LANGULUS_ERROR("Unsupported meta deserialization");
         return read + count;
      }

      result = {};
      return read;
   }

#endif

} // namespace Langulus::Anyness

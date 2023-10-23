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

   /// Byte container copy-construction                                       
   ///   @param other - container to reference                                
   LANGULUS(INLINED)
   Bytes::Bytes(const Bytes& other)
      : Bytes {Copy(other)} {}

   /// Byte container move-construction                                       
   ///   @param other - container to move                                     
   LANGULUS(INLINED)
   Bytes::Bytes(Bytes&& other) noexcept
      : Bytes {Move(other)} {}

   /// Copy-construction from any container/element                           
   ///   @param other - container/element to shallow-copy                     
   LANGULUS(INLINED)
   Bytes::Bytes(const CT::NotSemantic auto& other)
      : Bytes {Copy(other)} {}

   LANGULUS(INLINED)
   Bytes::Bytes(CT::NotSemantic auto& other)
      : Bytes {Copy(other)} {}

   /// Move-construction from any container/element                           
   ///   @param other - container/element to move                             
   LANGULUS(INLINED)
   Bytes::Bytes(CT::NotSemantic auto&& other)
      : Bytes {Move(other)} {}

   /// Semantic construction from any container/element                       
   ///   @param other - the container/element and the semantic                
   LANGULUS(INLINED)
   Bytes::Bytes(CT::Semantic auto&& other) : Bytes {} {
      using S = Decay<decltype(other)>;
      using T = TypeOf<S>;

      if constexpr (CT::Array<T>) {
         // Integration with bounded arrays                             
         static_assert(CT::POD<::std::remove_extent_t<T>>,
            "Bounded array should be made of CT::POD elements");

         new (this) Bytes {Base::From(
            S::Nest(reinterpret_cast<const Byte*>(*other)),
            sizeof(T)
         )};
      }
      else if constexpr (CT::DerivedFrom<T, Base>) {
         // Transfer any TAny<Byte> based container                     
         mType = MetaData::Of<Byte>();
         BlockTransfer<Base>(other.Forward());
      }
      else if constexpr (CT::Exact<T, Token>) {
         // Integration with std::string_view                           
         // Copies the string as a byte sequence                        
         if (other->empty())
            return;

         new (this) Bytes {Base::From(
            S::Nest(reinterpret_cast<const Byte*>(other->data())),
            other->size()
         )};
      }
      else if constexpr (CT::Meta<T>) {
         // Serialize any meta definition                               
         if (*other) {
            *this += Bytes {Count {(*other)->mToken.size()}};
            *this += Bytes {(*other)->mToken};
         }
         else *this += Bytes {Count {0}};
      }
      else if constexpr (CT::POD<T> && CT::Dense<T>) {
         // Copy/Move/Abandon/Disown/Clone anything dense and POD       
         new (this) Bytes {Base::From(
            S::Nest(reinterpret_cast<const Byte*>(&(*other))),
            sizeof(T)
         )};
      }
      else LANGULUS_ERROR("Bad semantic construction");
   }

   /// Construct manually via raw constant memory pointer and size            
   ///   @param raw - raw memory to reference                                 
   ///   @param size - number of bytes inside 'raw'                           
   LANGULUS(INLINED)
   Bytes::Bytes(const void* raw, const Size& size)
      : Bytes {Copy(static_cast<const Byte*>(raw)), size} {}

   /// Construct manually via raw mutable memory pointer and size             
   ///   @param raw - raw memory to reference                                 
   ///   @param size - number of bytes inside 'raw'                           
   LANGULUS(INLINED)
   Bytes::Bytes(void* raw, const Size& size)
      : Bytes {Copy(static_cast<Byte*>(raw)), size} {}

   /// Construct semantically via raw memory pointer, semantic, and size      
   ///   @param raw - raw memory and semantic to use                          
   ///   @param size - number of bytes inside 'raw'                           
   template<CT::Semantic S>
   LANGULUS(INLINED)
   Bytes::Bytes(S&& raw, const Size& size) requires (CT::Sparse<TypeOf<S>>)
      : TAny {TAny::From(raw.Forward(), size)} { }

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
   
   /// Copy-assign an unknown container                                       
   /// This is a bit slower, because it checks type compatibility at runtime  
   ///   @param other - the container to shallow-copy                         
   ///   @return a reference to this container                                
   LANGULUS(INLINED)
   Bytes& Bytes::operator = (const CT::NotSemantic auto& other) {
      return operator = (Copy(other));
   }
   
   LANGULUS(INLINED)
   Bytes& Bytes::operator = (CT::NotSemantic auto& other) {
      return operator = (Copy(other));
   }

   /// Move-assign an unknown container                                       
   /// This is a bit slower, because it checks type compatibility at runtime  
   ///   @param other - the container to move                                 
   ///   @return a reference to this container                                
   LANGULUS(INLINED)
   Bytes& Bytes::operator = (CT::NotSemantic auto&& other) {
      return operator = (Move(other));
   }

   /// Shallow-copy disowned runtime container without referencing contents   
   /// This is a bit slower, because checks type compatibility at runtime     
   ///   @param other - the container to shallow-copy                         
   ///   @return a reference to this container                                
   LANGULUS(INLINED)
   Bytes& Bytes::operator = (CT::Semantic auto&& other) {
      using S = Decay<decltype(other)>;
      using T = TypeOf<S>;

      if constexpr (CT::DerivedFrom<T, Base>) {
         if (static_cast<const Block*>(this)
            == static_cast<const Block*>(&*other))
            return *this;
      }

      Free();
      new (this) Bytes {other.Forward()};
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
      return Concatenate<Bytes>(Copy(static_cast<const Block&>(rhs)));
   }

   /// Move-concatenate with another TAny                                     
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   LANGULUS(INLINED)
   Bytes Bytes::operator + (Bytes&& rhs) const {
      return Concatenate<Bytes>(Move(Forward<Block>(rhs)));
   }

   /// Move-concatenate with another TAny                                     
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   LANGULUS(INLINED)
   Bytes Bytes::operator + (CT::Semantic auto&& rhs) const {
      using S = Decay<decltype(rhs)>;

      if constexpr (CT::DerivedFrom<TypeOf<S>, Base>)
         return Concatenate<Bytes>(rhs.template Forward<Block>());
      else
         LANGULUS_ERROR("Bad semantic concatenation");
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
   LANGULUS(INLINED)
   Bytes& Bytes::operator += (CT::Semantic auto&& rhs) {
      using S = Decay<decltype(rhs)>;

      if constexpr (CT::DerivedFrom<TypeOf<S>, Base>) {
         InsertBlock(rhs.Forward());
         return *this;
      }
      else LANGULUS_ERROR("Bad semantic concatenation");
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
      if (IsEmpty() or IsStatic() or start >= end)
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
   ///   @param source - the bytes to deserialize                             
   ///   @param result - [out] the deserialized meta goes here                
   ///   @param read - byte offset inside 'from'                              
   ///   @param header - environment header                                   
   ///   @param loader - loader for streaming                                 
   ///   @return number of read bytes                                         
   template<class META>
   Size Bytes::DeserializeMeta(META const*& result, Offset read, const Header& header, const Loader& loader) const {
      Count count = 0;
      read = DeserializeAtom(count, read, header, loader);
      if (count) {
         RequestMoreBytes(read, count, loader);
         const Token token {GetRawAs<Letter>() + read, count};
         if constexpr (CT::Same<META, MetaData>)
            result = RTTI::GetMetaData(token);
         else if constexpr (CT::Same<META, RTTI::MetaVerb>)
            result = RTTI::GetMetaVerb(token);
         else if constexpr (CT::Same<META, RTTI::MetaTrait>)
            result = RTTI::GetMetaTrait(token);
         else if constexpr (CT::Same<META, RTTI::MetaConst>)
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

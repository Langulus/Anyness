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

   /// Semantic bytes constructor                                             
   ///   @param other - the text container to use semantically                
   template<class T> requires CT::Bytes<Desem<T>> LANGULUS(INLINED)
   Bytes::Bytes(T&& other)
      : Base {SemanticOf<T>(other).template Forward<Base>()} {}

   /// Serialize a single POD item                                            
   ///   @param item - the item to serialize                                  
   LANGULUS(INLINED)
   Bytes::Bytes(const CT::BinablePOD auto& item) {
      using T = Deref<decltype(item)>;
      Block::AllocateFresh<Bytes>(Block::RequestSize<Bytes>(sizeof(T)));
      if constexpr (CT::Array<T>)
         CopyMemory(mRaw, reinterpret_cast<const Byte*>(item), sizeof(T));
      else
         CopyMemory(mRaw, reinterpret_cast<const Byte*>(&item), sizeof(T));
      mCount += sizeof(T);
   }
 
   /// Serialize a meta definition                                            
   ///   @param meta - the definition to serialize                            
   LANGULUS(INLINED)
   Bytes::Bytes(const CT::Meta auto& meta) {
      const auto tokensize = meta->mToken.size();
      const auto count = sizeof(Count) + tokensize;
      Block::AllocateFresh<Bytes>(Block::RequestSize<Bytes>(count));
      CopyMemory(GetRaw(), &tokensize, sizeof(tokensize));
      if (tokensize)
         CopyMemory(GetRaw() + sizeof(Count), meta->mToken.data(), tokensize);
      mCount = count;
   }
 
   /// Compose bytes by an arbitrary amount of binable arguments              
   ///   @param t1 - the first argument                                       
   ///   @param t2 - the second argument                                      
   ///   @param tn... - the rest of the arguments (optional)                  
   template<class T1, class T2, class...TN>
   requires CT::Inner::Binable<T1, T2, TN...> LANGULUS(INLINED)
   Bytes::Bytes(T1&& t1, T2&& t2, TN&&...tn) {
      UnfoldInsert(Forward<T1>(t1));
      UnfoldInsert(Forward<T2>(t2));
      ((UnfoldInsert(Forward<TN>(tn))), ...);
   }
   
   /// Semantic construction from count-terminated array                      
   ///   @param text - text memory to wrap                                    
   ///   @param count - number of characters inside text                      
   template<class T> requires (CT::Sparse<Desem<T>> and CT::Byte<Desem<T>>)
   LANGULUS(INLINED) Bytes Bytes::From(T&& text, Count count) {
      auto block = Base::From(Forward<T>(text), count);
      return Abandon(reinterpret_cast<Bytes&>(block));
   }

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

   /// Assign any Bytes block                                                 
   ///   @param rhs - the block to assign                                     
   template<class T> requires CT::Bytes<Desem<T>>
   LANGULUS(INLINED) Bytes& Bytes::operator = (T&& rhs) {
      Base::operator = (Forward<T>(rhs));
      return *this;
   }
   
   /// Hash the byte sequence                                                 
   ///   @return a hash of the contained byte sequence                        
   LANGULUS(INLINED)
   Hash Bytes::GetHash() const {
      return HashBytes(GetRaw(), static_cast<int>(GetCount()));
   }
   
   /// Compare with another block of bytes                                    
   /// The base Block::operator == can't be used, because Bytes arent CT::Deep
   ///   @param rhs - the block to compare with                               
   ///   @return true if blocks are the same                                  
   LANGULUS(INLINED)
   bool Bytes::operator == (const CT::Block auto& rhs) const noexcept {
      if (mCount != rhs.mCount)
         return false;
      else if (IsEmpty())
         return true;

      using T = Deref<decltype(rhs)>;
      if constexpr (CT::Bytes<T> or (CT::Typed<T> and CT::Byte<TypeOf<T>>))
         return 0 == ::std::memcmp(mRaw, rhs.mRaw, mCount);
      else if constexpr (not CT::Typed<T>) {
         if (rhs.GetType()->template IsSimilar<Byte>())
            return 0 == ::std::memcmp(mRaw, rhs.mRaw, mCount);
         else
            return false;
      }
      else return false;
   }

   /// Compare with a single character                                        
   ///   @param rhs - the character to compare with                           
   ///   @return true if this container contains this exact character         
   LANGULUS(INLINED)
   bool Bytes::operator == (const CT::BinablePOD auto& rhs) const noexcept {
      using T = Deref<decltype(rhs)>;
      if (mCount != sizeof(T))
         return false;

      if constexpr (CT::Array<T>)
         return 0 == ::std::memcmp(mRaw, rhs, sizeof(T));
      else
         return 0 == ::std::memcmp(mRaw, &rhs, sizeof(T));
   }

   /// Pick a part of the byte array                                          
   ///   @param start - the starting byte offset                              
   ///   @param count - the number of bytes after 'start' to remain           
   ///   @return a new container that references the original memory          
   LANGULUS(INLINED)
   Bytes Bytes::Crop(Offset start, Count count) {
      return Block::Crop<Bytes>(start, count);
   }

   LANGULUS(INLINED)
   Bytes Bytes::Crop(Offset start, Count count) const {
      return Block::Crop<Bytes>(start, count);
   }

   /// Concatenate two byte containers                                        
   ///   @param rhs - right hand side                                         
   ///   @return the concatenated byte container                              
   template<class T> requires CT::Binable<Desem<T>> LANGULUS(INLINED)
   Bytes Bytes::operator + (T&& rhs) const {
      return ConcatInner<Bytes>(Forward<T>(rhs));
   }

   /// Concatenate (destructively) byte containers                            
   ///   @param rhs - right hand side                                         
   ///   @return a reference to this container                                
   template<class T> requires CT::Binable<Desem<T>> LANGULUS(INLINED)
   Bytes& Bytes::operator += (T&& rhs) {
      return ConcatRelativeInner<Bytes>(Forward<T>(rhs));
   }

   /// Inner concatenation function, used in all Byte derivatives             
   ///   @param rhs - right hand side                                         
   ///   @return the concatenated byte container                              
   template<CT::Bytes THIS, class T>
   THIS Bytes::ConcatInner(T&& rhs) const {
      using S = SemanticOf<T>;
      using B = TypeOf<S>;

      if constexpr (CT::Block<B>) {
         if constexpr (CT::Typed<B>) {
            if constexpr (CT::Similar<Byte, TypeOf<B>>) {
               // We can concat directly                                
               return Block::ConcatBlock<THIS>(S::Nest(rhs));
            }
            else LANGULUS_ERROR("Can't concatenate with this container");
         }
         else {
            // Type-erased concat                                       
            return Block::ConcatBlock<THIS>(S::Nest(rhs));
         }
      }
      else {
         // RHS isn't Block, try to convert it to Bytes, and nest       
         return ConcatInner<THIS>(static_cast<THIS>(DesemCast(rhs)));
      }
   }

   /// Inner concatenation function, used in all Text derivatives             
   ///   @param rhs - right hand side                                         
   ///   @return a reference to this container                                
   template<CT::Bytes THIS, class T>
   THIS& Bytes::ConcatRelativeInner(T&& rhs) {
      using S = SemanticOf<T>;
      using B = TypeOf<S>;

      if constexpr (CT::Block<B>) {
         if constexpr (CT::Typed<B>) {
            if constexpr (CT::Similar<Byte, TypeOf<B>>) {
               // We can concat directly                                
               Block::InsertBlock<THIS, void>(IndexBack, S::Nest(rhs));
            }
            else LANGULUS_ERROR("Can't concatenate with this container");
         }
         else {
            // Type-erased concat                                       
            Block::InsertBlock<THIS, void>(IndexBack, S::Nest(rhs));
         }
      }
      else {
         // RHS isn't Block, try to convert it to Text, and nest        
         return ConcatRelativeInner<THIS>(static_cast<THIS>(DesemCast(rhs)));
      }

      return static_cast<THIS&>(*this);
   }

   /// Extend the byte sequence, change count, and return the new range       
   ///   @attention if you extend static container, it will diverge           
   ///   @param count - the number of bytes to append                         
   ///   @return the extended part                                            
   LANGULUS(INLINED)
   Bytes Bytes::Extend(Count count) {
      return Block::Extend<Bytes>(count);
   }
   
   /// Insert an element, or an array of elements                             
   ///   @param item - the argument to unfold and insert, can be semantic     
   void Bytes::UnfoldInsert(auto&& item) {
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
            Block::InsertBlockInner<Bytes, void, true>(IndexBack, Copy(data));
         }
         else {
            // Unfold and serialize elements, one by one                
            for (auto& element : DesemCast(item))
               UnfoldInsert(S::Nest(element));
         }
      }
      else if constexpr (CT::Block<T>) {
         if constexpr (CT::Bytes<T>) {
            // Insert another byte block's contents                     
            Block::AllocateMore<Bytes>(mCount + DesemCast(item).GetCount());
            CopyMemory(GetRaw() + mCount, DesemCast(item).GetRaw());
            mCount += DesemCast(item).GetCount();
         }
         else if constexpr (CT::Typed<T>) {
            // Nest-insert typed block's elements one by one            
            for (auto& element : DesemCast(item))
               UnfoldInsert(S::Nest(element));
         }
         else {
            // Check a type-erased block                                
            if (DesemCast(item).GetType()->template IsSimilar<Byte>()) {
               // Insert another byte block's contents directly         
               Block::AllocateMore<Bytes>(mCount + DesemCast(item).GetCount());
               CopyMemory(GetRaw() + mCount, DesemCast(item).template GetRaw<Bytes>());
               mCount += DesemCast(item).GetCount();
            }
            else for (auto subblock : DesemCast(item))
               UnfoldInsert(S::Nest(subblock));
         }
      }
      else if constexpr (CT::POD<T> and CT::Dense<T>) {
         // Insert as byte array                                        
         auto data = Block::From(
            reinterpret_cast<const Byte*>(&DesemCast(item)),
            sizeof(T)
         );
         InsertBlockInner<Bytes, void, true>(IndexBack, Copy(data));
      }
      else LANGULUS_ERROR("Unable to insert as bytes");
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
   
   template<class T>
   T Bytes::Deserialize() const {
      TODO();
      return {};
   }

/*#if LANGULUS_FEATURE(MANAGED_REFLECTION)

   ///                                                                        
   LANGULUS(INLINED)
   void Bytes::RequestMoreBytes(
      Offset read, Size byteCount, const Loader& loader
   ) const {
      if (read >= GetCount() or GetCount() - read < byteCount) {
         if (not loader)
            LANGULUS_THROW(Access, "Deserializer has no loader");
         loader(const_cast<Bytes&>(*this), byteCount - (GetCount() - read));
      }
   }

   /// Read an atom-sized unsigned integer, based on the provided header      
   ///   @param result - [out] the resulting deserialized number              
   ///   @param read - offset to apply to serialized byte array               
   ///   @param header - environment header                                   
   ///   @param loader - loader for streaming                                 
   ///   @return the number of read bytes from byte container                 
   inline Size Bytes::DeserializeAtom(
      Offset& result, Offset read, const Header& header, const Loader& loader
   ) const {
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
   Size Bytes::DeserializeMeta(
      META& result, Offset read, const Header& header, const Loader& loader
   ) const {
      Count count = 0;
      read = DeserializeAtom(count, read, header, loader);
      if (count) {
         RequestMoreBytes(read, count, loader);
         const Token token {GetRawAs<Letter, Bytes>() + read, count};
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

#endif*/

} // namespace Langulus::Anyness

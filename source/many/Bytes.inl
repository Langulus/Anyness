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


namespace Langulus::Anyness
{

   /// Refer-constructor                                                      
   ///   @param other - container to reference                                
   LANGULUS(INLINED)
   Bytes::Bytes(const Bytes& other)
      : Bytes {Refer(other)} {}

   /// Move-constructor                                                       
   ///   @param other - container to move                                     
   LANGULUS(INLINED)
   Bytes::Bytes(Bytes&& other) noexcept
      : Bytes {Move(other)} {}

   /// Semantic bytes constructor                                             
   ///   @param other - the text container to use semantically                
   template<class T> requires CT::Bytes<Desem<T>> LANGULUS(INLINED)
   Bytes::Bytes(T&& other)
      : Base {SemanticOf<decltype(other)>(other).template Forward<Base>()} {}

   /// Serialize a single POD item                                            
   ///   @param item - the item to serialize                                  
   LANGULUS(INLINED)
   Bytes::Bytes(const CT::BinablePOD auto& item) {
      using T = Deref<decltype(item)>;
      constexpr auto size = sizeof(T);

      Block::AllocateFresh<Bytes>(Block::RequestSize<Bytes>(size));
      if constexpr (CT::Array<T>)
         CopyMemory(mRaw, reinterpret_cast<const Byte*>( item), size);
      else
         CopyMemory(mRaw, reinterpret_cast<const Byte*>(&item), size);
      mCount += size;
   }
 
   /// Serialize a meta definition                                            
   ///   @param meta - the definition to serialize                            
   LANGULUS(INLINED)
   Bytes::Bytes(const CT::Meta auto& meta) {
      constexpr auto atom = sizeof(Count);

      if (meta) {
         const auto token = meta->mToken;
         const Count tokensize = static_cast<Count>(token.size());
         const Count count = atom + tokensize;
         Block::AllocateFresh<Bytes>(Block::RequestSize<Bytes>(count));
         mCount = count;
         CopyMemory(GetRaw(),
            reinterpret_cast<const Byte*>(&tokensize), atom);
         CopyMemory(GetRaw() + atom,
            reinterpret_cast<const Byte*>(token.data()), tokensize);
      }
      else {
         const Count tokensize = 0;
         Block::AllocateFresh<Bytes>(Block::RequestSize<Bytes>(atom));
         mCount = atom;
         CopyMemory(GetRaw(),
            reinterpret_cast<const Byte*>(&tokensize), atom);
      }
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

   /// Refer-assignment                                                       
   ///   @param rhs - the byte container to refer to                          
   ///   @return a reference to this container                                
   LANGULUS(INLINED)
   Bytes& Bytes::operator = (const Bytes& rhs) {
      return operator = (Refer(rhs));
   }

   /// Move-assignment                                                        
   ///   @param rhs - the byte container to move                              
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
         return 0 == ::std::memcmp(mRaw,  rhs, sizeof(T));
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
      using S = SemanticOf<decltype(rhs)>;
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
      using S = SemanticOf<decltype(rhs)>;
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
            Block::InsertBlockInner<Bytes, void, true>(IndexBack, Refer(data));
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
         InsertBlockInner<Bytes, void, true>(IndexBack, Refer(data));
      }
      else LANGULUS_ERROR("Unable to insert as bytes");
   }
   
   /// Deserialize a byte container to a desired type                         
   ///   @tparam result - [out] data/container to deserialize into            
   ///   @return the number of parsed bytes                                   
   Count Bytes::Deserialize(CT::Data auto& result) const {
      Header header;
      return Block::DeserializeBinary<Bytes, void>(result, header);
   }

} // namespace Langulus::Anyness

///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TAny.hpp"

namespace Langulus::Anyness
{

   ///                                                                        
   ///   Byte sequence container                                              
   ///                                                                        
   /// Convenient wrapper for count-terminated raw byte sequences             
   /// Can represent any POD type as a sequence of bytes                      
   ///                                                                        
   class Bytes : public TAny<Byte> {
      LANGULUS(DEEP) false;		
      LANGULUS_BASES(TAny<Byte>);

   public:
      constexpr Bytes() = default;
      Bytes(const Bytes&);
      Bytes(Bytes&&) noexcept;

      Bytes(const TAny&);
      Bytes(TAny&&) noexcept;

      template<CT::Deep T>
      Bytes(const T&) = delete;

      Bytes(const void*, const Size&);
      Bytes(void*, const Size&);
      
      template<CT::Semantic S>
      constexpr Bytes(S&&) noexcept requires (CT::DerivedFrom<TypeOf<S>, TAny<Byte>>);

      template<CT::POD T>
      explicit Bytes(const T&) requires CT::Dense<T>;

      explicit Bytes(const Token&);
      explicit Bytes(const RTTI::Meta*);

      Bytes& operator = (const Bytes&);
      Bytes& operator = (Bytes&&) noexcept;

      template<CT::Semantic S>
      Bytes& operator = (S&&) requires (CT::Exact<TypeOf<S>, Bytes>);
      
   public:
      NOD() Bytes Clone() const;
      NOD() Bytes Crop(const Offset&, const Count&) const;
      NOD() Bytes Crop(const Offset&, const Count&);
      Bytes& Remove(const Offset&, const Count&);
      Bytes Extend(const Count&);
      Hash GetHash() const;

      bool operator == (const Bytes&) const noexcept;

      ///                                                                     
      ///   Concatenation                                                     
      ///                                                                     
      NOD() Bytes operator + (const Bytes&) const;
      NOD() Bytes operator + (Bytes&&) const;
      template<CT::Semantic S>
      NOD() Bytes operator + (S&&) const requires (CT::Exact<TypeOf<S>, Bytes>);

      Bytes& operator += (const Bytes&);
      Bytes& operator += (Bytes&&);
      template<CT::Semantic S>
      Bytes& operator += (S&&) requires (CT::Exact<TypeOf<S>, Bytes>);
   };

} // namespace Langulus::Anyness

#include "Bytes.inl"

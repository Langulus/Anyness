///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
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

   private:
      using Base = TAny<Byte>;

   public:
      constexpr Bytes() = default;

      Bytes(const Bytes&);
      Bytes(Bytes&&) noexcept;
      template<template<class> class S>
      Bytes(S<Bytes>&&) requires CT::Semantic<S<Bytes>>;
      template<class T>
      Bytes(T&&) requires (CT::Inner::UnfoldMakableFrom<Byte, T>
                       or (CT::POD<T> and CT::Dense<T>));
      Bytes(const CT::Meta auto&);
      Bytes(auto&&, const Size&);

      Bytes& operator = (const Bytes&);
      Bytes& operator = (Bytes&&);
      template<template<class> class S>
      Bytes& operator = (S<Bytes>&&) requires CT::Semantic<S<Bytes>>;
      
   public:
      NOD() Bytes Clone() const;
      NOD() Bytes Crop(const Offset&, const Count&) const;
      NOD() Bytes Crop(const Offset&, const Count&);

      Bytes Extend(const Count&);
      Hash GetHash() const;

      bool operator == (const Bytes&) const noexcept;

      ///                                                                     
      ///   Concatenation                                                     
      ///                                                                     
      NOD() Bytes operator + (const Bytes&) const;
      NOD() Bytes operator + (Bytes&&) const;
      template<template<class> class S>
      NOD() Bytes operator + (S<Bytes>&&) const requires CT::Semantic<S<Bytes>>;

      Bytes& operator += (const Bytes&);
      Bytes& operator += (Bytes&&);
      template<template<class> class S>
      Bytes& operator += (S<Bytes>&&) requires CT::Semantic<S<Bytes>>;

   protected:
      Count UnfoldInsert(CT::Index auto, auto&&);

   public:
      ///                                                                     
      ///   Deserialization                                                   
      ///                                                                     
      #pragma pack(push, 1)
      struct Header {
         ::std::uint8_t mAtomSize;
         ::std::uint8_t mFlags;
         ::std::uint16_t mUnused;

      public:
         Header() noexcept;

         enum {Default, BigEndian};

         bool operator == (const Header&) const noexcept;
      };
      #pragma pack(pop)

      using Loader = void(*)(Bytes&, Size);

      // Intentionally undefined, because it requires Langulus::Flow    
      // and relies on Verbs::Interpret                                 
      #if LANGULUS_FEATURE(MANAGED_REFLECTION)
         template<bool HEADER, CT::Block TO>
         Size Deserialize(TO& result, const Header& header = {}, Offset readOffset = 0, const Loader& loader = {}) const;

      protected:
         void RequestMoreBytes(Offset, Size, const Loader&) const;

         NOD() Size DeserializeAtom(Offset&, Offset, const Header&, const Loader&) const;

         template<class META>
         NOD() Size DeserializeMeta(META&, Offset, const Header&, const Loader&) const;
      #endif
   };

} // namespace Langulus::Anyness

namespace Langulus::CT
{

   /// Concept for differentiating managed Anyness Text types                 
   template<class T>
   concept Bytes = DerivedFrom<T, Anyness::Bytes>;

} // namespace Langulus::CT
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

      //template<CT::Semantic S>
      //static constexpr bool Relevant = CT::DerivedFrom<TypeOf<S>, Base>;

   public:
      constexpr Bytes() = default;

      Bytes(const Bytes&);
      Bytes(Bytes&&) noexcept;

      Bytes(const CT::NotSemantic auto&);
      Bytes(CT::NotSemantic auto&);
      Bytes(CT::NotSemantic auto&&);
      Bytes(CT::Semantic auto&&);

      Bytes(const void*, const Size&);
      Bytes(void*, const Size&);
      template<CT::Semantic S>
      Bytes(S&&, const Size&) requires (CT::Sparse<TypeOf<S>>);

      Bytes& operator = (const Bytes&);
      Bytes& operator = (Bytes&&);

      Bytes& operator = (const CT::NotSemantic auto&);
      Bytes& operator = (CT::NotSemantic auto&);
      Bytes& operator = (CT::NotSemantic auto&&);
      Bytes& operator = (CT::Semantic auto&&);
      
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
      NOD() Bytes operator + (CT::Semantic auto&&) const;

      Bytes& operator += (const Bytes&);
      Bytes& operator += (Bytes&&);
      Bytes& operator += (CT::Semantic auto&&);

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
         NOD() Size DeserializeMeta(META const*&, Offset, const Header&, const Loader&) const;
      #endif
   };

} // namespace Langulus::Anyness

namespace Langulus::CT
{

   /// Concept for differentiating managed Anyness Text types                 
   template<class T>
   concept Bytes = DerivedFrom<T, ::Langulus::Anyness::Bytes>;

} // namespace Langulus::CT
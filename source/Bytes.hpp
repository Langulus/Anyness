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
      LANGULUS(POD) false;
      LANGULUS_BASES(TAny<Byte>);

   private:
      template<CT::Semantic S>
      static constexpr bool Relevant = CT::DerivedFrom<TypeOf<S>, TAny<Byte>>;

   public:
      constexpr Bytes() = default;

      Bytes(const Bytes&);
      Bytes(Bytes&&) noexcept;
      Bytes(const TAny&);
      Bytes(TAny&&) noexcept;

      // Constructing from other containers is disabled                 
      Bytes(const CT::Deep auto&) = delete;

      template<CT::Semantic S>
      Bytes(S&&) requires Relevant<S>;

      template<CT::POD T>
      explicit Bytes(const T&) requires CT::Dense<T>;
      explicit Bytes(const Token&);
      explicit Bytes(const RTTI::Meta*);

      Bytes(const void*, const Size&);
      Bytes(void*, const Size&);
      template<CT::Semantic S>
      Bytes(S&&, const Size&) requires (CT::Sparse<TypeOf<S>>);

      Bytes& operator = (const Bytes&);
      Bytes& operator = (Bytes&&) noexcept;

      template<CT::Semantic S>
      Bytes& operator = (S&&) requires Relevant<S>;
      
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
      NOD() Bytes operator + (S&&) const requires Relevant<S>;

      Bytes& operator += (const Bytes&);
      Bytes& operator += (Bytes&&);
      template<CT::Semantic S>
      Bytes& operator += (S&&) requires Relevant<S>;

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

#include "Bytes.inl"

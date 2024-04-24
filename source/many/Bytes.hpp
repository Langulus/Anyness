///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TMany.hpp"


namespace Langulus::CT
{

   /// Any dense value or array of values, that isn't CT::Block               
   template<class...T>
   concept BinablePOD = ((POD<Deext<T>> and Dense<Deext<T>>
          and not Block<Deext<T>> and not Meta<Deext<T>>
      ) and ...);

   namespace Inner
   {
   
      /// Do types have an explicit or implicit cast operator to Bytes        
      template<class...T>
      concept BinableByOperator = requires (T&...a) {
         ((a.operator ::Langulus::Anyness::Bytes()), ...); };

      /// Does Bytes has an explicit/implicit constructor that accepts T      
      template<class...T>
      concept BinableByConstructor = requires (T&...a) {
         ((::Langulus::Anyness::Bytes {a}), ...); };

      /// Used internally in Bytes, to sum up all types a variadic Bytes      
      /// constructor can accept                                              
      template<class...T>
      concept Binable = ((DerivedFrom<T, Anyness::Bytes>
           or BinablePOD<T>
           or Meta<T>
           or Inner::BinableByOperator<T>) and ...);

   } // namespace Langulus::CT::Inner

   /// A binable type is one that has either an implicit or explicit          
   /// cast operator to Bytes type, or can be used to explicitly initialize a 
   /// Bytes container                                                        
   template<class...T>
   concept Binable = ((Inner::BinableByOperator<T>
        or Inner::BinableByConstructor<T>) and ...);
   
   /// Concept for differentiating managed Anyness::Bytes                     
   template<class...T>
   concept Bytes = (DerivedFrom<T, Anyness::Bytes> and ...);

} // namespace Langulus::CT

namespace Langulus::Anyness
{
   
   ///                                                                        
   ///   Byte sequence container                                              
   ///                                                                        
   ///   Convenient wrapper for count-terminated raw byte sequences. Can      
   /// represent any POD type as a sequence of bytes. Also used as a binary   
   /// serializer.                                                            
   ///                                                                        
   struct Bytes : Block<Byte> {
      using Base = Block<Byte>;

      LANGULUS(DEEP) false;
      LANGULUS_BASES(Base);
      LANGULUS(FILES) "lgls";

      /// The presence of this structure makes Bytes a serializer             
      struct SerializationRules {

      };

      ///                                                                     
      ///   Construction                                                      
      ///                                                                     
      constexpr Bytes() = default;
      Bytes(const Bytes&);
      Bytes(Bytes&&) noexcept;

      template<class T> requires CT::Bytes<Desem<T>>
      Bytes(T&&);

      explicit Bytes(const CT::BinablePOD auto&);
      Bytes(const CT::Meta auto&);

      template<class T1, class T2, class...TN>
      requires CT::Inner::Binable<T1, T2, TN...>
      Bytes(T1&&, T2&&, TN&&...);

      template<class T> requires (CT::Sparse<Desem<T>> and CT::Byte<Decay<Desem<T>>>)
      static Bytes From(T&&, Count);

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      Bytes& operator = (const Bytes&);
      Bytes& operator = (Bytes&&);

      template<class T> requires CT::Bytes<Desem<T>>
      Bytes& operator = (T&&);
      
      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      Hash GetHash() const;

      ///                                                                     
      ///   Indexing                                                          
      ///                                                                     
      NOD() Bytes Crop(Offset, Count) const;
      NOD() Bytes Crop(Offset, Count);

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      bool operator == (const CT::Block auto&) const noexcept;
      bool operator == (const CT::BinablePOD auto&) const noexcept;

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Bytes Extend(Count);

      ///                                                                     
      ///   Concatenation                                                     
      ///                                                                     
      template<class T> requires CT::Binable<Desem<T>>
      NOD() Bytes operator + (T&&) const;

      template<class T> requires CT::Binable<Desem<T>>
      Bytes& operator += (T&&);

   protected:
      template<CT::Bytes THIS, class T>
      THIS ConcatInner(T&&) const;
      template<CT::Bytes THIS, class T>
      THIS& ConcatRelativeInner(T&&);

      void UnfoldInsert(auto&&);

   public:
      ///                                                                     
      ///   Deserialization                                                   
      ///                                                                     
      NOD() Count Deserialize(CT::Data auto&) const;
   };

} // namespace Langulus::Anyness

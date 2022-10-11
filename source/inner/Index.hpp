///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Config.hpp"

namespace Langulus::Anyness
{

   ///                                                                        
   ///   A multipurpose index, used to access common elements in containers   
   ///                                                                        
   struct Index {
      LANGULUS(POD) true;
      LANGULUS(NULLIFIABLE) true;
      LANGULUS(SUFFIX) "i";
      LANGULUS(INFO) "Used to safely access elements inside containers";

   public:
      using Type = ::std::ptrdiff_t;

      /// These are defines useful for special indices                        
      static constexpr Type MaxIndex = ::std::numeric_limits<Type>::max();
      static constexpr Type MinIndex = ::std::numeric_limits<Type>::min();

   protected:
      enum SpecialIndices : Type {
         // All, Many, and Single must be compared in separate context  
         All = MinIndex,
         Many,
         Single,

         // Back, Middle, Front, and None must be compared separately   
         None,
         Front,
         Middle,
         Back,

         // These can't be compared                                     
         Mode,
         Biggest,
         Smallest,
         Auto,
         Random,

         // This signifies the end of the special indices               
         Counter,

         // These must be wrapped before compared                       
         Last = -1,

         // These fit into the non-special category                     
         First = 0
      };

      LANGULUS_NAMED_VALUES(SpecialIndices) {
         {"All", SpecialIndices::All},
         {"Many", SpecialIndices::Many},
         {"Single", SpecialIndices::Single},
         {"None", SpecialIndices::None},
         {"Front", SpecialIndices::Front},
         {"Middle", SpecialIndices::Middle},
         {"Back", SpecialIndices::Back},
         {"Mode", SpecialIndices::Mode},
         {"Biggest", SpecialIndices::Biggest},
         {"Smallest", SpecialIndices::Smallest},
         {"Auto", SpecialIndices::Auto},
         {"Random", SpecialIndices::Random}
      };

      #if LANGULUS_DEBUG()
         union {
            // Named index (useful for debugging)                       
            SpecialIndices mNamedIndex {SpecialIndices::None};
            // Raw index                                                
            Type mIndex;
         };
      #else
         Type mIndex {SpecialIndices::None};
      #endif

   public:
      constexpr Index() noexcept = default;
      constexpr Index(const Index&) noexcept = default;
      constexpr Index(const SpecialIndices&) noexcept;
      template<CT::SignedInteger T>
      constexpr Index(const T&) noexcept;
      template<CT::UnsignedInteger T>
      constexpr Index(const T&);

   public:
      NOD() constexpr Index Constrained(Count) const noexcept;
      NOD() Offset GetOffset() const;

      constexpr void Constrain(Count) noexcept;
      constexpr void Concat(const Index&) noexcept;

      NOD() constexpr bool IsValid() const noexcept;
      NOD() constexpr bool IsInvalid() const noexcept;
      NOD() constexpr bool IsSpecial() const noexcept;
      NOD() constexpr bool IsReverse() const noexcept;
      NOD() constexpr bool IsArithmetic() const noexcept;

      NOD() explicit constexpr operator bool() const noexcept;
      NOD() explicit constexpr operator const Type& () const noexcept;

      constexpr void operator ++ () noexcept;
      constexpr void operator -- () noexcept;
      constexpr void operator += (const Index&) noexcept;
      constexpr void operator -= (const Index&) noexcept;
      constexpr void operator *= (const Index&) noexcept;
      constexpr void operator /= (const Index&) noexcept;

      NOD() constexpr Index operator + (const Index&) const noexcept;
      NOD() constexpr Index operator - (const Index&) const noexcept;
      NOD() constexpr Index operator * (const Index&) const noexcept;
      NOD() constexpr Index operator / (const Index&) const noexcept;
      NOD() constexpr Index operator - () const noexcept;

      NOD() constexpr bool operator == (const Index&) const noexcept;
      NOD() constexpr bool operator != (const Index&) const noexcept;

      NOD() constexpr bool operator < (const Index&) const noexcept;
      NOD() constexpr bool operator > (const Index&) const noexcept;
      NOD() constexpr bool operator <= (const Index&) const noexcept;
      NOD() constexpr bool operator >= (const Index&) const noexcept;
   };

} // namespace Langulus::Anyness

namespace Langulus::CT
{

   /// Generalized index concept                                              
   template<class T>
   concept Index = Integer<T> || Same<T, Anyness::Index>;

} // namespace Langulus::CT

#include "Index.inl"

namespace Langulus::Anyness
{

   constexpr Index IndexAll {Index::MinIndex};
   constexpr Index IndexMany {Index::MinIndex + 1};
   constexpr Index IndexSingle {Index::MinIndex + 2};
   constexpr Index IndexNone {Index::MinIndex + 3};
   constexpr Index IndexFront {Index::MinIndex + 4};
   constexpr Index IndexMiddle {Index::MinIndex + 5};
   constexpr Index IndexBack {Index::MinIndex + 6};
   constexpr Index IndexMode {Index::MinIndex + 7};
   constexpr Index IndexBiggest {Index::MinIndex + 8};
   constexpr Index IndexSmallest {Index::MinIndex + 9};
   constexpr Index IndexAuto {Index::MinIndex + 10};
   constexpr Index IndexRandom {Index::MinIndex + 11};
   constexpr Index IndexFirst {0};
   constexpr Index IndexLast {-1};

} // namespace Langulus::Anyness

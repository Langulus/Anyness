///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Config.hpp"
#include <RTTI/MetaData.hpp>

namespace Langulus::Anyness
{

   ///                                                                        
   ///   Data state flags                                                     
   ///                                                                        
   struct DataState {
      LANGULUS(POD) true;
      LANGULUS(NULLIFIABLE) true;

      enum Enum {
         // Default data state                                          
         // Default state is inclusive, mutable, nonmissing             
         // nonstatic, nonencrypted, noncompressed, and dense           
         Default = 0,

         // Tags data as missing                                        
         // Missing data is considered only a hint, that is used to     
         // direct data expansion, when combined with Future/Past       
         Missing = 1,

         // The data is compressed, and it is your respondibility to    
         // decompress it before using it                               
         Compressed = 2,

         // The data is encrypted, and it is your respondibility to     
         // decrypt it with the correct key before using it             
         Encrypted = 4,

         // Enables inhibition (or so called exclusive (OR) container)  
         // An OR container means that data can be used in multiple     
         // parallel ways. Verbs in such containers, for example, are   
         // considered branched. Beware, using OR containers might      
         // cause	huge overhead, because in some contexts a full        
         // stack cloning might occur                                   
         Or = 8,

         // Future missing symbol, when missing                         
         Future = 16,

         // If state is only Missing, then it is missing past           
         Past = Missing,

         // Data won't move, reallocate, deallocate, etc.               
         // Used to constrain the memory manipulations. That way you    
         // can reference static or unmovable memory as any other.      
         // Data can still change in-place, unless constant.            
         // Reflected members are interfaced in that way                
         Static = 32,

         // Data won't move, reallocate, deallocate, or even change     
         // Used to constrain the memory manipulations for safety       
         // That way you can interface constant memory as any other     
         Constant = 64,

         // Data won't ever change type - useful for templated packs    
         // Used to constrain the memory manipulations for safety       
         Typed = 128,

         // Data is fully constrained                                   
         // Useful set of states to interface a constant member         
         Constrained = Static | Constant | Typed,

         // Useful set of states to interface a mutable member          
         Member = Static | Typed,
         ConstantMember = Constrained,
         TypedConstant = Constant | Typed,
         MissingFuture = Missing | Future,
         MissingPast = Missing | Past
      };

      using Type = TypeOf<Enum>;

      Type mState {Default};

   public:
      constexpr DataState() noexcept = default;
      constexpr DataState(const Type&) noexcept;

      explicit constexpr operator bool() const noexcept;
      constexpr bool operator == (const DataState&) const noexcept = default;
      
      NOD() constexpr DataState operator + (const DataState&) const noexcept;
      NOD() constexpr DataState operator - (const DataState&) const noexcept;
      constexpr DataState& operator += (const DataState&) noexcept;
      constexpr DataState& operator -= (const DataState&) noexcept;
      
      NOD() constexpr bool operator & (const DataState&) const noexcept;
      NOD() constexpr bool operator % (const DataState&) const noexcept;
      
      NOD() constexpr bool IsDefault() const noexcept;
      NOD() constexpr bool IsMissing() const noexcept;
      NOD() constexpr bool IsCompressed() const noexcept;
      NOD() constexpr bool IsEncrypted() const noexcept;
      NOD() constexpr bool IsOr() const noexcept;
      NOD() constexpr bool IsNow() const noexcept;
      NOD() constexpr bool IsFuture() const noexcept;
      NOD() constexpr bool IsPast() const noexcept;
      NOD() constexpr bool IsStatic() const noexcept;
      NOD() constexpr bool IsConstant() const noexcept;
      NOD() constexpr bool IsTyped() const noexcept;
      NOD() constexpr bool IsConstrained() const noexcept;
      
      constexpr void Reset() noexcept;
   };
   
} // namespace Langulus::Anyness

#include "DataState.inl"

///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "many/TMany.hpp"


namespace Langulus::Anyness
{

   template<class...T>
   concept DenseTypedBlock = CT::Block<T...>
       and CT::Dense<T...> and CT::Typed<T...>;


   ///                                                                        
   ///   Interface for editing containers of any kind                         
   ///                                                                        
   ///   Allows you to select regions and do operations on to them,           
   /// while keeping track of selections and synchronizing changes between    
   /// them. The rationale is, that when modifying a TMany, for example,      
   /// picking and changing a selection will result in a copy the contents.   
   /// That is sometimes not desirable. This interface ensures, that you can  
   /// modify selections safely without branching out - changes will be       
   /// synchronized with the original upon destruction of this interface.     
   ///   The interface is designed to immitate the usual keyboard editing of  
   /// a text file - you can mark text, replace it, insert of the right or    
   /// left of selection, backspace, delete, etc.                             
   ///                                                                        
   template<DenseTypedBlock T>
   class Edit {
      LANGULUS(TYPED) TypeOf<T>;
      LANGULUS(UNINSERTABLE) true;

      // What are we editing?                                           
      T& mSource;
      // Start of the selection                                         
      Offset mStart = 0;
      // End of the selection                                           
      Offset mEnd = 0;

   public:
      Edit() = delete;
      Edit(const Edit&) = delete;
      Edit(Edit&&) noexcept = default;
      Edit(T*, Offset = 0, Offset = 0) noexcept;
      Edit(T&, Offset = 0, Offset = 0) noexcept;

      Edit& Select(const T&);
      Edit& Select(Offset, Offset);
      Edit& Select(Offset);

      NOD() const T& GetSource() const noexcept;
      NOD() Offset GetStart() const noexcept;
      NOD() Offset GetEnd() const noexcept;
      NOD() Count  GetLength() const noexcept;

      NOD() auto& operator[] (Offset) const noexcept;
      NOD() auto& operator[] (Offset) noexcept;

      Edit& operator << (const T&);
      Edit& operator >> (const T&);
      Edit& Replace(const T&);

      Edit& operator << (const TypeOf<T>&);
      Edit& operator >> (const TypeOf<T>&);
      Edit& Replace(const TypeOf<T>&);

      Edit& Delete();
      Edit& Backspace();
   };

   /// Deduction guides                                                       
   template<DenseTypedBlock T>
   Edit(T&) -> Edit<T>;

   template<DenseTypedBlock T>
   Edit(T*) -> Edit<T>;

} // namespace Langulus::Anyness

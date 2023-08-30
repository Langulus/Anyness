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

   template<class T>
   concept DenseTypedBlock = CT::Block<T> && CT::Dense<T> && CT::Typed<T>;

   ///                                                                        
   /// Interface for editing containers of any kind                           
   /// Allows you to select regions and do operations relative to them, while 
   /// keeping track of selections and synchronizing changes between them     
   ///                                                                        
   template<DenseTypedBlock T>
   class TEdit {
      LANGULUS(TYPED) TypeOf<T>;
      LANGULUS(UNINSERTABLE) true;

      // What are we editing?                                           
      T& mSource;
      // Start of the selection                                         
      Offset mStart {};
      // End of the selection                                           
      Offset mEnd {};

   public:
      TEdit() = delete;
      TEdit(const TEdit&) = delete;
      TEdit(TEdit&&) noexcept = default;
      TEdit(T&, Offset = 0, Offset = 0) noexcept;

      TEdit& Select(const T&);
      TEdit& Select(Offset, Offset);
      TEdit& Select(Offset);

      NOD() const T& GetSource() const noexcept;
      NOD() Offset GetStart() const noexcept;
      NOD() Offset GetEnd() const noexcept;
      NOD() Count GetLength() const noexcept;

      NOD() auto& operator[] (Offset) const noexcept;
      NOD() auto& operator[] (Offset) noexcept;

      TEdit& operator << (const T&);
      TEdit& operator >> (const T&);
      TEdit& Replace(const T&);

      TEdit& operator << (const TypeOf<T>&);
      TEdit& operator >> (const TypeOf<T>&);
      TEdit& Replace(const TypeOf<T>&);

      TEdit& Delete();
      TEdit& Backspace();
   };

   template<CT::Sparse T>
   auto Edit(T what) noexcept {
      return TEdit<Decay<T>> {DenseCast(what)};
   }

   template<CT::Dense T>
   auto Edit(T& what) noexcept {
      return TEdit<Decay<T>> {what};
   }

} // namespace Langulus::Anyness

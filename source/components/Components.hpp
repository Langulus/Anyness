///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../DataState.hpp"
#include "../Compare.hpp"
#include "../Index.hpp"
#include "../Iterator.hpp"
#include "../one/Handle.hpp"
#include "../one/Own.hpp"


///                                                                           
/// Components are simple structures that when combined, define the size of   
/// containers and their behavior. Any change in these structures' layout     
/// will need to be paired with a major version change.                       
///                                                                           
namespace Langulus::Anyness::Component
{

   ///                                                                        
   /// The data component has a type-erased pointer to some data              
   ///                                                                        
   struct Data {
      union {
         // Display as a string for ease of debugging                   
         DEBUGGERY(char* mStartChar);
         // Raw pointer to first element inside the memory block        
         Byte*  mStart {};
         // The pointer can point to another pointer                    
         Byte** mStartSparse;
      };
   };
   
   ///                                                                        
   /// The source component keeps a pointer to an allocation                  
   ///                                                                        
   struct Source {
      // Pointer to an allocation. If this entry is zero, then data     
      // is static, and we have no authority over resizing it           
      Allocation* mSource {};
   };

   ///                                                                        
   /// The range component allows us to represent contiguous memory           
   /// If N is CountMax, then the memory is resizable at runtime.             
   ///                                                                        
   template<Count N>
   struct Range {
      static_assert(N > 0, "N must be greater than zero");
      static constexpr Count CellCount = N;
   };

   template<>
   struct Range<CountMax> {
      // Pointer to the first uninitialized element                     
      Byte* mNext;
      union {
         // Pointer to the end                                          
         Byte* mEnd;
         // Also, this is where usually sources of pointers start       
         Allocation** mSparseSources;
      }
   };

   ///                                                                        
   /// The meta component contains RTTI type information. If T is void, the   
   /// container is considered type-erased - its type may change at runtime.  
   ///                                                                        
   template<class T>
   struct Meta {
      using CellType = T;
   };

   template<>
   struct Meta<void> {
      // The contained type                                             
      DMeta mType;
   };

   ///                                                                        
   /// The ownership component makes sure that contained data and sources     
   /// are properly referenced when transferred between containers.           
   ///                                                                        
   struct Ownership {};

   ///                                                                        
   /// The hashed component makes sure that hashing is cached, so that it     
   /// isn't recomputed every time.                                           
   ///                                                                        
   struct Hashed {
      // The cached hash                                                
      Hash mHash {};
   };

   ///                                                                        
   /// The Small Value Optimization component allows for part of the layout   
   /// to be reused to allocate small data on the stack, instead of on the    
   /// heap.                                                                  
   ///                                                                        
   struct SVO {};

   ///                                                                        
   /// The table component allows for cells to be reused                      
   ///                                                                        
   struct Table {
      // Pointer to the start of the table array                        
      Byte* mTable;
   };

   ///                                                                        
   /// The ordered component generates indices for sorting                    
   ///                                                                        
   struct Ordered {

   };

   ///                                                                        
   /// The missing component allows for a container to be marked missing      
   ///                                                                        
   struct Missing {
   
   };
   
   ///                                                                        
   /// The 'or' component allows container to be marked as disjunctive        
   ///                                                                        
   struct Or {
   
   };
   
   ///                                                                        
   /// The compress component allows contained memory to be compressed        
   ///                                                                        
   struct Compress {
   
   };
   
   ///                                                                        
   /// The encrypt component allows contained memory to be encrypted          
   ///                                                                        
   struct Encrypt {
   
   };
   
   ///                                                                        
   /// The constant component allows contained memory to be marked as         
   /// constant, to prevent any change at runtime.                            
   ///                                                                        
   struct Constant {
   
   };

} // namespace Langulus::Anyness::Layout
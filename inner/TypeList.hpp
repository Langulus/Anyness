///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 - 2022 Dimo Markov <langulusteam@gmail.com>					
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "Integration.hpp"

namespace Langulus::Anyness
{

   ///                                                                        
   ///   Compile-time type list                                               
   ///                                                                        
   template<class...T>
   class TTypeList {
   public:
      static constexpr Count GetCount() noexcept {
         return sizeof...(T);
      }

   private:
      template<class HEAD, class...>
      struct HeadWrapper { using type = HEAD; };
      template<class...ALL>
      using Head = typename HeadWrapper<ALL...>::type;

      template<class HEAD, class...TAIL>
      struct TailWrapper { using type = TTypeList<TAIL...>; };
      template<class...ALL>
      using Tail = typename TailWrapper<ALL...>::type;

      template<Offset N>
      struct AtWrapper {
         using type = Conditional<N == 0, Head<T...>, typename Tail<T...>::template At<N - 1>>;
      };

   public:
      template<Offset INDEX>
      using At = typename AtWrapper<INDEX>::type;

      template<class... MORE>
      static constexpr auto Push(TTypeList<MORE...>) -> TTypeList<T..., MORE...>;
   };


   ///                                                                        
   ///   Compile-time type list generator													
   ///                                                                        
   /// Let's say you have the following class:                                
   /// template<class T> class Vector {}                                      
   ///                                                                        
   /// You can create a vector class type generator like this:                
   /// TYPE_GENERATOR(UniqueName, Vector<T>);                                 
   ///                                                                        
   /// Then, you can generate types by:                                       
   /// using VectorTypes = GENERATE_TYPELIST(UniqueName, int, float)          
   ///                                                                        
   /// At this point VectorTypes will contain:                                
   /// TTypeList<TVector<int>, TVector<float>>                                
   ///                                                                        
#define TYPE_GENERATOR(name, format) \
      template<Count SIZE> \
      struct name { \
         template<class... T> \
         static constexpr auto ForEach(TTypeList<T...>) -> TTypeList<format...>; \
      }

   #define GENERATE_TYPELIST(a, ...) \
      decltype(a::ForEach(TTypeList<__VA_ARGS__>{}))


} // namespace Langulus::Anyness

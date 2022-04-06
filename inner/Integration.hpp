#pragma once

#ifndef LANGULUS_INTEGRATION
	/// Building as standalone, which is indicated by the lack of					
	/// -DLANGULUS_INTEGRATION																	
	/// We have to define missing stuff here												
	#include <type_traits>
	#include <typeinfo>
	
	#define LANGULUS(a) LANGULUS_##a()
	#define LANGULUS_MODULE(a) LANGULUS(MODULE_##a)
	#define LANGULUS_MODULE_Anyness()
	
	#define LANGULUS_DEEP() public: static constexpr bool Deep = true
	#define NOD() [[nodiscard]]
	
	namespace Langulus
	{
		
		using Count = ::std::size_t;
		
		///																							
		/// Concepts																				
		///																							
		
		/// A reflected type is a type that has a public Reflection field			
		/// This field is automatically added when using LANGULUS(REFLECT) macro
		/// inside the type you want to reflect											
		template<class T>
		concept Reflected = requires { T::Reflection; };
		
		/// A reflected data type is any type that is not void, and is either	
		/// manually reflected, or an implicitly reflected fundamental type		
		template<class T>
		concept ReflectedData = !::std::is_void_v<T> && (Reflected<T> || ::std::is_fundamental_v<T>);
	
		/// A deep type is any type with a static member T::Deep set to true		
		/// If no such member exists, the type is assumed NOT deep by default	
		/// Deep types are considered iteratable, and verbs are executed in each
		/// of their elements, instead on the container itself						
		template<class T>
		concept Deep = T::Deep == true;
		
		/// When building standalone, we shall use CTTI only							
		/// Types are identified by their hash code										
		struct DataID {
			::std::size_t mID;
			
			template<class T>
			static constexpr DataID Of;
		};
		
		template<class T>
		constexpr DataID DataID::Of = [](){ return typeid(T).hash_code(); };
		
		using DMeta = const DataID&;
		
	} // namespace Langulus
#else
	#if defined(LANGULUS_INCLUDES_RTTI)
		/// RTTI module is available - use it												
		#include <Langulus.RTTI.hpp>
	#elif defined(LANGULUS_INCLUDES_CTTI)
		/// CTTI module is available - use it												
		#include <Langulus.CTTI.hpp>
	#else
		#error Langulus integration doesn't provide neither RTTI, nor CTTI
	#endif
#endif

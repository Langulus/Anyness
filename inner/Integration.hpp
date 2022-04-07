#pragma once

#ifndef LANGULUS_INTEGRATION

	/// Building as standalone, which is indicated by the lack of					
	/// -DLANGULUS_INTEGRATION																	
	/// We have to define missing stuff here												
	#include <type_traits>
	#include <typeinfo>
	#include <cstddef>
	#include <functional>
	#include <span>
	
	#define LANGULUS(a) LANGULUS_##a()
	#define LANGULUS_MODULE(a) LANGULUS(MODULE_##a)
	#define LANGULUS_MODULE_Anyness()
	#define LANGULUS_DISABLED() 0
	#define LANGULUS_ENABLED() 1
	
	#define LANGULUS_DEEP() public: static constexpr bool Deep = true
	#define NOD() [[nodiscard]]
	
	#if defined(DEBUG) || !defined(NDEBUG) || defined(_DEBUG) || defined(CB_DEBUG) || defined(QT_QML_DEBUG)
		#define LANGULUS_DEBUG() LANGULUS_ENABLED()
	#else
		#define LANGULUS_DEBUG() LANGULUS_DISABLED()
	#endif

	namespace Langulus
	{
		
		///																							
		/// Fundamental types																	
		///																							
		using Byte = ::std::byte;
		using Count = ::std::size_t;
		using RefCount = ::std::ptrdiff_t;
		using Stride = ::std::size_t;
		using Offset = ::std::size_t;
		using Hash = ::std::size_t;
		template<class T>
		using TFunctor = ::std::function<T>;
		using Token = ::std::span<char>;


		///																							
		/// Concepts																				
		///																							
		/// Remove a reference from type														
		template<class T>
		using Deref = ::std::remove_reference_t<T>;

		/// Remove a pointer from type														
		template<class T>
		using Deptr = ::std::remove_pointer_t<T>;

		/// Remove a const/volatile from a type											
		template<class T>
		using Decvq = ::std::remove_cv_t<T>;

		/// Remove an array extent from a type												
		template<class T>
		using Deext = ::std::remove_extent_t<T>;

		/// Strip a typename to its root type, removing qualifiers and ptrs		
		/// Note that this strips only 1D array, one reference, one pointer...	
		/// You can chain multiple pcDecay<pcDecay<T>> if not sure					
		template<class T>
		using Decay = Decvq<Deptr<Deext<Deref<T>>>>;

		/// True if two decayed types match													
		template<class T1, class T2>
		concept Same = ::std::same_as<Decay<T1>, Decay<T2>>;

		/// True if two decayed types don't match											
		template<class T1, class T2>
		concept NotSame = !::std::same_as<Decay<T1>, Decay<T2>>;

		/// Boolean concept																		
		template<class T>
		concept Boolean = Same<T, bool>;

		/// True if T is an array (has an extent with [])								
		/// Sometimes a reference hides the pointer/extent, hence the deref		
		template<class T>
		constexpr bool Array = ::std::is_array_v<Deref<T>>;

		/// True if T is a pointer (or has an extent with [])							
		/// Sometimes a reference hides the pointer/extent, hence the deref		
		template<class T>
		concept Sparse = ::std::is_pointer_v<Deref<T>> || Array<T>;

		/// True if T is not a pointer (and has no extent with[])					
		template<class T>
		concept Dense = !Sparse<T>;

		/// A reflected type is a type that has a public Reflection field			
		/// This field is automatically added when using LANGULUS(REFLECT) macro
		/// inside the type you want to reflect											
		template<class T>
		concept Reflected = requires { Decay<T>::Reflection; };
		
		/// A reflected data type is any type that is not void, and is either	
		/// manually reflected, or an implicitly reflected fundamental type		
		template<class T>
		concept ReflectedData = !::std::is_void_v<Decay<T>> && (Reflected<T> || ::std::is_fundamental_v<Decay<T>>);
	
		/// A deep type is any type with a static member T::Deep set to true		
		/// If no such member exists, the type is assumed NOT deep by default	
		/// Deep types are considered iteratable, and verbs are executed in each
		/// of their elements, instead on the container itself						
		template<class T>
		concept Deep = T::Deep == true;

		/// Sortable concept																		
		/// Any class with an adequate <, >, <=, >=, or combined <=> operator	
		template<class T, class U = T>
		concept Sortable = requires(Decay<T> t, Decay<U> u) {
			{ t < u } -> Boolean;
			{ t > u } -> Boolean;
		};

	} // namespace Langulus

	#include "Reflection.hpp"
	#include "Index.hpp"

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

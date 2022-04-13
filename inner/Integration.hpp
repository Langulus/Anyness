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
	#include <string_view>
	#include <limits>
	
	#define LANGULUS(a) LANGULUS_##a()
	#define LANGULUS_MODULE(a) LANGULUS(MODULE_##a)
	#define LANGULUS_MODULE_Anyness()
	#define LANGULUS_DISABLED() 0
	#define LANGULUS_ENABLED() 1
	
	#define NOD() [[nodiscard]]
	#define LANGULUS_SAFE() LANGULUS_ENABLED()
	#define LANGULUS_PARANOID() LANGULUS_DISABLED()
	#define LANGULUS_ALIGN() ::std::size_t{16}

	#if defined(DEBUG) || !defined(NDEBUG) || defined(_DEBUG) || defined(CB_DEBUG) || defined(QT_QML_DEBUG)
		#define LANGULUS_DEBUG() LANGULUS_ENABLED()
		#define DEBUGGERY(a) a
	#else
		#define LANGULUS_DEBUG() LANGULUS_DISABLED()
		#define DEBUGGERY(a)
	#endif

	#if LANGULUS_SAFE()
		#define SAFETY(a) a
	#else
		#define SAFETY(a)
	#endif

	/// Trigger a static assert (without condition)										
	/// This form is required in order of it to work in 'if constexpr - else'	
	/// https://stackoverflow.com/questions/38304847									
	#define LANGULUS_ASSERT(text) []<bool flag = false>() { static_assert(flag, "FAILED ASSERTION: " text); }()
	#define TODO() LANGULUS_ASSERT("TODO")

	namespace Langulus
	{
		
		///																							
		/// Fundamental types																	
		///																							
		using Byte = ::std::byte;
		using Count = ::std::size_t;
		using Stride = ::std::size_t;
		using Offset = ::std::size_t;
		using Hash = ::std::size_t;
		template<class T>
		using TFunctor = ::std::function<T>;
		using Token = ::std::u8string_view;
		using Pointer = ::std::uintptr_t;


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

		/// Get the extent of an array, or 1 if dense, or 0 if sparse				
		template<class T>
		constexpr Count ExtentOf = Array<T> ? ::std::extent_v<Deref<T>> : (Dense<T> ? 1 : 0);

		/// Sortable concept																		
		/// Any class with an adequate <, >, or combined <=> operator				
		template<class T, class U = T>
		concept Sortable = requires(Decay<T> t, Decay<U> u) {
			{ t < u } -> Boolean;
			{ t > u } -> Boolean;
		};

		/// Character concept																	
		template<class T>
		concept Character = Same<T, char8_t> || Same<T, char16_t> || Same<T, char32_t> || Same<T, wchar_t>;

		/// Integer number concept (either sparse or dense)							
		/// Excludes boolean types																
		template<class... T>
		concept Integer = (... && (::std::is_integral_v<Decay<T>> && !Boolean<T> && !Character<T>));

		/// Real number concept (either sparse or dense)								
		template<class... T>
		concept Real = (... && (::std::is_floating_point_v<Decay<T>>));

		/// Built-in number concept (either sparse or dense)							
		template<class T>
		concept BuiltinNumber = Integer<T> || Real<T>;

		/// Number concept (either sparse or dense)										
		template<class T>
		concept Number = BuiltinNumber<T>;

		/// Check if type is signed (either sparse or dense)							
		/// Doesn't apply to only number, but anything where T(-1) < T(0)			
		template<class T>
		concept Signed = ::std::is_signed_v<Decay<T>>;

		/// Check if type is unsigned (either sparse or dense)						
		template<class T>
		concept Unsigned = !Signed<T>;

		/// Check if type is statically convertible to another						
		/// Types are not decayed and are as provided to preserve operators		
		/// Concept apply if you have an explicit/implicit cast operator 			
		/// `FROM::operator TO() const`, or if you have explicit/implicit			
		/// constructor TO::TO(const FROM&)													
		template<class FROM, class TO>
		concept StaticallyConvertible = ::std::convertible_to<FROM, TO>;

		/// Pick between two types, based on a condition								
		template<bool CONDITION, class TRUETYPE, class FALSETYPE>
		using Conditional = ::std::conditional_t<CONDITION, TRUETYPE, FALSETYPE>;

		/// Make a type constant reference or constant pointer						
		template<class T>
		using MakeConst = Conditional<Dense<T>, const Decay<T>&, const Decay<T>*>;

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

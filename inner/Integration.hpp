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
	#include <concepts>
	
	/// All non-argument macros should use this facility								
	/// https://www.fluentcpp.com/2019/05/28/better-macros-better-flags/			
	#define LANGULUS(a) LANGULUS_##a()
	#define LANGULUS_MODULE(a) LANGULUS(MODULE_##a)
	#define LANGULUS_MODULE_Anyness()
	#define LANGULUS_DISABLED() 0
	#define LANGULUS_ENABLED() 1
	#define LANGULUS_FPU() double
	
	#define NOD() [[nodiscard]]
	#define LANGULUS_SAFE() LANGULUS_DISABLED()
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
		#define SAFETY_NOEXCEPT()
	#else
		#define SAFETY(a)
		#define SAFETY_NOEXCEPT() noexcept
	#endif

	#ifdef _MSC_VER
		#define LANGULUS_NOINLINE() __declspec(noinline)
	#else
		#define LANGULUS_NOINLINE() __attribute__((noinline))
	#endif

	#ifdef _MSC_VER
		#define LANGULUS_LIKELY(condition) condition
		#define LANGULUS_UNLIKELY(condition) condition
	#else
		#define LANGULUS_LIKELY(condition) __builtin_expect(condition, 1)
		#define LANGULUS_UNLIKELY(condition) __builtin_expect(condition, 0)
	#endif

	#if INTPTR_MAX == INT64_MAX
		#define LANGULUS_BITNESS() 64
	#elif INTPTR_MAX == INT32_MAX
		#define LANGULUS_BITNESS() 32
	#else
		#error Unknown pointer size
	#endif

	#define LANGULUS_FEATURE(a) LANGULUS_FEATURE_##a()

	/// Replace the default new-delete operators with one that use Allocator	
	/// No overhead, no dependencies															
	#define LANGULUS_FEATURE_NEWDELETE() LANGULUS_DISABLED()

	/// Use the utfcpp library to convert between utf types							
	/// No overhead, requires utfcpp to be linked										
	#define LANGULUS_FEATURE_UTFCPP() LANGULUS_DISABLED()

	/// Enable memory compression via the use of zlib									
	/// Gives a tiny runtime overhead, requires zlib to be linked					
	#define LANGULUS_FEATURE_ZLIB() LANGULUS_DISABLED()

	/// Enable memory encryption and decryption											
	/// Gives a tiny runtime overhead, no dependencies									
	#define LANGULUS_FEATURE_ENCRYPTION() LANGULUS_DISABLED()

	/// Memory allocations will be pooled, authority will be tracked,				
	/// memory will be reused whenever possible, and you can also tweak			
	/// runtime allocation strategies on per-type basis								
	/// Significantly improves performance, no dependencies							
	#define LANGULUS_FEATURE_MANAGED_MEMORY() LANGULUS_DISABLED()

	/// Reflections will be registered in a centralized location, allowing for	
	/// runtime type modification. Meta primitives will always be in the same	
	/// place in memory regardless of translation unit, which significantly		
	/// speeds up meta definition comparisons.											
	/// Naming collisions will be detected upon type registration					
	/// Gives a significant overhead on program launch, no dependencies			
	#define LANGULUS_FEATURE_MANAGED_REFLECTION() LANGULUS_DISABLED()

	/// Trigger a static assert (without condition)										
	/// This form is required in order of it to work in 'if constexpr - else'	
	/// https://stackoverflow.com/questions/38304847									
	#define LANGULUS_ASSERT(text) []<bool flag = false>() { static_assert(flag, "FAILED ASSERTION: " text); }()
	#define TODO() LANGULUS_ASSERT("TODO")

	namespace Langulus
	{

		namespace Anyness
		{
			class Block;
		}

		namespace Flow
		{
			class Verb;
		}

		///																							
		/// Fundamental types																	
		///																							
		using Byte = ::std::byte;
		using Count = ::std::size_t;
		using Size = ::std::size_t;
		using Offset = ::std::size_t;
		using Hash = ::std::size_t;
		template<class T>
		using TFunctor = ::std::function<T>;
		using Token = ::std::u8string_view;
		using Pointer = ::std::uintptr_t;
		using Real = LANGULUS(FPU);


		/// Check endianness at compile-time												
		/// True if the architecture uses big/mixed endianness						
		constexpr bool BigEndianMachine = ::std::endian::native == ::std::endian::big;
		constexpr bool LittleEndianMachine = ::std::endian::native == ::std::endian::little;

		
		/// Count leading/trailing bits                                         
		#ifdef _MSC_VER
			#include <intrin.h>
			#if LANGULUS(BITNESS) == 32
				#pragma intrinsic(_BitScanForward)
				#pragma intrinsic(_BitScanReverse)
			#elif LANGULUS(BITNESS) == 64
				#pragma intrinsic(_BitScanForward64)
				#pragma intrinsic(_BitScanReverse64)
			#else
				#error Not implemented
			#endif

			constexpr int CountTrailingZeroes(size_t mask) {
				unsigned long index;
				#if LANGULUS(BITNESS) == 32
					return _BitScanForward(&index, mask)
						? static_cast<int>(index)
						: LANGULUS(BITNESS);
				#elif LANGULUS(BITNESS) == 64
					return _BitScanForward64(&index, mask)
						? static_cast<int>(index)
						: LANGULUS(BITNESS);
				#else
					#error Not implemented
				#endif
			}

			constexpr int CountLeadingZeroes(size_t mask) {
				unsigned long index;
				#if LANGULUS(BITNESS) == 32
					return _BitScanReverse(&index, mask)
						? static_cast<int>(index)
						: LANGULUS(BITNESS);
				#elif LANGULUS(BITNESS) == 64
					return _BitScanReverse64(&index, mask)
						? static_cast<int>(index)
						: LANGULUS(BITNESS);
				#else
					#error Not implemented
				#endif
			}
		#else
			constexpr int CountTrailingZeroes(size_t mask) {
				unsigned long index;
				#if LANGULUS(BITNESS) == 32
					return mask ? __builtin_ctzl(mask) : LANGULUS(BITNESS);
				#elif LANGULUS(BITNESS) == 64
					return mask ? __builtin_ctzll(mask) : LANGULUS(BITNESS);
				#else
					#error Not implemented
				#endif
			}

			constexpr int CountLeadingZeroes(size_t mask) {
				unsigned long index;
				#if LANGULUS(BITNESS) == 32
					return mask ? __builtin_clzl(mask) : LANGULUS(BITNESS);
				#elif LANGULUS(BITNESS) == 64
					return mask ? __builtin_clzll(mask) : LANGULUS(BITNESS);
				#else
					#error Not implemented
				#endif
			}
		#endif

		/// Same as ::std::declval, but conveniently named								
		template<class T>
		::std::add_rvalue_reference_t<T> Uneval() noexcept {
			LANGULUS_ASSERT("Calling Uneval is ill-formed");
		}

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
		concept IsSame = ::std::same_as<Decay<T1>, Decay<T2>>;

		/// Boolean concept																		
		template<class T>
		concept IsBool = IsSame<T, bool>;

		/// True if T is an array (has an extent with [])								
		/// Sometimes a reference hides the pointer/extent, hence the deref		
		template<class T>
		constexpr bool IsArray = ::std::is_bounded_array_v<Deref<T>>;

		/// True if T is a pointer (or has an extent with [])							
		/// Sometimes a reference hides the pointer/extent, hence the deref		
		template<class T>
		concept IsSparse = ::std::is_pointer_v<Deref<T>> || IsArray<T>;

		/// True if T is not a pointer (and has no extent with[])					
		template<class T>
		concept IsDense = not IsSparse<T>;

		/// Get the extent of an array, or 1 if dense, or 0 if sparse				
		template<class T>
		constexpr Count ExtentOf = IsArray<T> ? ::std::extent_v<Deref<T>> : (IsDense<T> ? 1 : 0);

		/// IsSortable concept																	
		/// Any class with an adequate <, >, or combined <=> operator				
		template<class T, class U = T>
		concept IsSortable = requires(Decay<T> t, Decay<U> u) {
			{ t < u } -> IsBool;
			{ t > u } -> IsBool;
		};
		
		/// Equality comparable concept														
		/// Any class with an adequate == operator										
		template<class T, class U = T>
		concept IsComparable = ::std::equality_comparable_with<Decay<T>, Decay<U>>;
		
		/// IsConvertible concept																
		/// Checks if a static_cast is possible between the provided types		
		template<class FROM, class TO>
		concept IsConvertible = requires(FROM from, TO to) {
			to = static_cast<TO>(from);
		};

		/// IsCharacter concept																	
		/// Notice how char is not here, as it is considered a number				
		template<class T>
		concept IsCharacter = IsSame<T, char8_t> || IsSame<T, char16_t> || IsSame<T, char32_t> || IsSame<T, wchar_t>;

		/// IsInteger number concept (either sparse or dense)							
		/// Excludes boolean types																
		template<class... T>
		concept IsInteger = (... && (::std::integral<Decay<T>> && not IsBool<T> && not IsCharacter<T>));

		/// IsReal number concept (either sparse or dense)								
		template<class... T>
		concept IsReal = (... && (::std::floating_point<Decay<T>>));

		/// Built-in number concept (either sparse or dense)							
		template<class T>
		concept IsBuiltinNumber = IsInteger<T> || IsReal<T>;

		/// IsNumber concept (either sparse or dense)									
		template<class T>
		concept IsNumber = IsBuiltinNumber<T>;

		/// Check if type is signed (either sparse or dense)							
		/// Doesn't apply to only number, but anything where T(-1) < T(0)			
		template<class T>
		concept IsSigned = ::std::is_signed_v<Decay<T>>;

		/// Check if type is unsigned (either sparse or dense)						
		template<class T>
		concept IsUnsigned = not IsSigned<T>;

		/// Check if type is any signed integer (either sparse or dense)			
		template<class T>
		concept IsSignedInteger = IsInteger<T> && IsSigned<T>;

		/// Check if type is any unsigned integer (either sparse or dense)		
		template<class T>
		concept IsUnsignedInteger = IsInteger<T> && IsUnsigned<T>;

		/// Pick between two types, based on a condition								
		template<bool CONDITION, class TRUETYPE, class FALSETYPE>
		using Conditional = ::std::conditional_t<CONDITION, TRUETYPE, FALSETYPE>;

		/// Make a type constant reference or constant pointer						
		template<class T>
		using MakeConst = Conditional<IsDense<T>, const Decay<T>&, const Decay<T>*>;

		/// Check if type is cv-qualified													
		template<class T>
		concept IsConstant = ::std::is_const_v<Deptr<Deref<T>>>;

		/// Check if type is not cv-qualified												
		template<class T>
		concept IsMutable = not IsConstant<T>;

		/// Check if T is abstract (has at least one pure virtual function)		
		template<class T>
		concept IsAbstract = ::std::is_abstract_v<Decay<T>> || Decay<T>::CTTI_Abstract == true;

		/// Check if T is a fundamental type (either sparse or dense)				
		template<class... T>
		concept IsFundamental = (::std::is_fundamental_v<Decay<T>> && ...);

		/// Check if T is default-constructible											
		template<class... T>
		concept IsDefaultConstructible = (::std::default_initializable<Decay<T>> && ...);

		/// Check if T is copy-constructible												
		template<class... T>
		concept IsCopyConstructible = (::std::copy_constructible<Decay<T>> && ...);
		
		/// Check if T is copy-constructible												
		template<class... T>
		concept IsMoveConstructible = (::std::move_constructible<Decay<T>> && ...);
		
		/// Check if T is destructible														
		template<class... T>
		concept IsDestructible = (::std::destructible<Decay<T>> && ...);
		
		/// Check if Decay<T> is clonable													
		template<class... T>
		concept IsClonable = (requires (Decay<T> a) { 
			{a = a.Clone()};
		} && ...);
		
		/// Check if T is referencable														
		template<class T>
		concept IsReferencable = requires (const Decay<T> a) { 
			{a.Keep()};
			{a.Free()} -> IsSame<Count>;
		};
		
		/// Check if T is copy-assignable													
		template<class T>
		concept IsCopyable = ::std::copyable<Decay<T>>;
		
		/// Check if T is move-assignable													
		template<class T>
		concept IsMovable = ::std::movable<Decay<T>>;
		
		/// Check if T is move-assignable													
		template<class T>
		concept IsResolvable = requires (Decay<T> a) {
			{a.GetBlock()} -> IsSame<Anyness::Block>;
		};
		
		/// Check if T is move-assignable													
		template<class T>
		concept IsHashable = requires (Decay<T> a) {
			{a.GetHash()} -> IsSame<Hash>;
		};
		
		/// Check if T is move-assignable													
		template<class T>
		concept IsDispatcher = requires (Decay<T> a, Flow::Verb& b) {
			{a.Do(b)};
		};
		
		/// Check if T inherits BASE															
		template<class T, class BASE>
		concept Inherits = ::std::derived_from<Decay<T>, Decay<BASE>>;

		/// Check if type has no reference/pointer/extent, etc.						
		template<class T>
		concept IsDecayed = IsDense<T> && !::std::is_reference_v<T> && IsMutable<T>;

	} // namespace Langulus

#else

	#if defined(LANGULUS_INCLUDES_RTTI)
		/// RTTI module is available - use it												
		#include <Langulus.RTTI.hpp>
	#elif defined(LANGULUS_INCLUDES_CTTI)
		/// CTTI module is available - use it												
		#include <Langulus.CTTI.hpp>
	#else
		#error "Langulus integration doesn't provide neither RTTI, nor CTTI"
	#endif

#endif

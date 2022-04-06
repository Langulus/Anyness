#pragma once
#include "Any.hpp"

namespace Langulus::Anyness
{

	///																								
	/// A helper structure for pairing keys and values of any type					
	///																								
	template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
	struct TPair {
	public:
		using KeyType = KEY;
		using ValueType = VALUE;

		TPair() = delete;
		TPair(const TPair&) = default;
		TPair(TPair&&) noexcept = default;
		TPair(KEY key, VALUE value)
			: Key{ key }
			, Value{ value } {}

	public:
		KEY Key;
		VALUE Value;
	};


	///																								
	///	DATA CONTAINER SPECIALIZATION FOR KEY-VALUE PAIRS							
	///																								
	class LANGULUS_MODULE(Anyness) Map : public Any {
		REFLECT(Map);
	public:
		Map() noexcept = default;
		Map(const Map&);
		Map(Map&&) noexcept;
		Map(const Block&, const Block&);

		ME& operator = (const ME&);
		ME& operator = (ME&&) noexcept;

	public:
		NOD() static Text GetMapToken(DMeta, DMeta);

		NOD() const Any& Keys() const noexcept;
		NOD() Any& Keys() noexcept;
		NOD() const Any& Values() const noexcept;
		NOD() Any& Values() noexcept;

		NOD() Map Clone() const;

		NOD() static Map From(DMeta, DMeta, const DState& = {}) noexcept;

		template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
		NOD() static Map From(const DState& = {}) noexcept;

		void Clear();
		void Reset();

		NOD() inline DMeta KeyMeta() const;
		NOD() inline DMeta ValueMeta() const;

		template<RTTI::ReflectedData KEY>
		NOD() Index FindKey(const KEY&) const;

		template<RTTI::ReflectedData VALUE>
		NOD() Index FindValue(const VALUE&) const;

		template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
		NOD() auto GetPair(const Index&);

		template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
		NOD() auto GetPair(const Index&) const;

		template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
		NOD() auto GetPair(pcptr);

		template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
		NOD() auto GetPair(pcptr) const;

		template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
		NOD() bool IsMapInsertable();

		template<RTTI::ReflectedData KEY>
		NOD() decltype(auto) GetKey(const Index&) const;
		template<RTTI::ReflectedData KEY>
		NOD() decltype(auto) GetKey(const Index&);
		template<RTTI::ReflectedData KEY>
		NOD() decltype(auto) GetKey(const pcptr) const;
		template<RTTI::ReflectedData KEY>
		NOD() decltype(auto) GetKey(const pcptr);

		template<RTTI::ReflectedData VALUE>
		NOD() decltype(auto) GetValue(const Index&) const;
		template<RTTI::ReflectedData VALUE>
		NOD() decltype(auto) GetValue(const Index&);
		template<RTTI::ReflectedData VALUE>
		NOD() decltype(auto) GetValue(const pcptr) const;
		template<RTTI::ReflectedData VALUE>
		NOD() decltype(auto) GetValue(const pcptr);

		template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
		pcptr Emplace(TPair<KEY, VALUE>&&, const Index& = uiBack);

		template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
		pcptr Insert(const TPair<KEY, VALUE>*, pcptr = 1, const Index& = uiBack);

		template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
		Map& operator << (const TPair<KEY, VALUE>&);

		template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
		Map& operator << (TPair<KEY, VALUE>&&);

		template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
		Map& operator >> (const TPair<KEY, VALUE>&);

		template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
		Map& operator >> (TPair<KEY, VALUE>&&);

		template<class FUNCTION>
		pcptr ForEachPair(FUNCTION&&);

		template<class FUNCTION>
		pcptr ForEachPairRev(FUNCTION&&);

		template<class FUNCTION>
		pcptr ForEachPair(FUNCTION&&) const;

		template<class FUNCTION>
		pcptr ForEachPairRev(FUNCTION&&) const;

	protected:
		template<class RETURN, RTTI::ReflectedData ALT_KEY, RTTI::ReflectedData ALT_VALUE, bool REVERSE>
		pcptr ForEachPairInner(TFunctor<RETURN(ALT_KEY, ALT_VALUE)>&&);

		template<class RETURN, RTTI::ReflectedData ALT_KEY, RTTI::ReflectedData ALT_VALUE, bool REVERSE>
		pcptr ForEachPairInner(TFunctor<RETURN(ALT_KEY, ALT_VALUE)>&&) const;

		/// This function declaration is used to decompose a lambda					
		/// You can use it to extract the argument type of the lambda, using		
		/// decltype on the return type. Useful for template deduction in the	
		/// ForEach functions above, purely for convenience							
		template<class RETURN, class FUNCTION, RTTI::ReflectedData ALT_KEY, RTTI::ReflectedData ALT_VALUE>
		TPair<ALT_KEY, ALT_VALUE> GetLambdaArguments(RETURN(FUNCTION::*)(ALT_KEY, ALT_VALUE) const) const;

	protected:
		Any mKeys;
	};

} // namespace Langulus::Anyness

#include "Map.inl"

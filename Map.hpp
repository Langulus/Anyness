#pragma once
#include "Any.hpp"

namespace Langulus::Anyness
{

	///																								
	/// A helper structure for pairing keys and values of any type					
	///																								
	template<ReflectedData KEY, ReflectedData VALUE>
	struct TPair {
	public:
		KEY Key;
		VALUE Value;

	public:
		using KeyType = KEY;
		using ValueType = VALUE;

		TPair() = delete;
		TPair(const TPair&) = default;
		TPair(TPair&&) noexcept = default;
		TPair(KEY key, VALUE value)
			: Key {key}
			, Value {value} {}
	};


	///																								
	///	DATA CONTAINER SPECIALIZATION FOR KEY-VALUE PAIRS							
	///																								
	class Map : public Any {
	public:
		Map() noexcept = default;
		Map(const Map&);
		Map(Map&&) noexcept;
		Map(const Block&, const Block&);

		Map& operator = (const Map&);
		Map& operator = (Map&&) noexcept;

	public:
		NOD() static Map From(DMeta, DMeta, const DataState& = {}) noexcept;

		NOD() static Text GetMapToken(DMeta, DMeta);
		NOD() const Any& Keys() const noexcept;
		NOD() Any& Keys() noexcept;
		NOD() const Any& Values() const noexcept;
		NOD() Any& Values() noexcept;
		NOD() Map Clone() const;

		template<ReflectedData KEY, ReflectedData VALUE>
		NOD() static Map From(const DataState& = {}) noexcept;

		void Clear();
		void Reset();

		NOD() inline DMeta KeyType() const;
		NOD() inline DMeta ValueType() const;

		template<ReflectedData KEY>
		NOD() Index FindKey(const KEY&) const;

		template<ReflectedData VALUE>
		NOD() Index FindValue(const VALUE&) const;

		template<ReflectedData KEY, ReflectedData VALUE>
		NOD() auto GetPair(const Index&);

		template<ReflectedData KEY, ReflectedData VALUE>
		NOD() auto GetPair(const Index&) const;

		template<ReflectedData KEY, ReflectedData VALUE>
		NOD() auto GetPair(Offset);

		template<ReflectedData KEY, ReflectedData VALUE>
		NOD() auto GetPair(Offset) const;

		template<ReflectedData KEY, ReflectedData VALUE>
		NOD() bool IsMapInsertable();

		template<ReflectedData KEY>
		NOD() decltype(auto) GetKey(const Index&) const;
		template<ReflectedData KEY>
		NOD() decltype(auto) GetKey(const Index&);
		template<ReflectedData KEY>
		NOD() decltype(auto) GetKey(Offset) const;
		template<ReflectedData KEY>
		NOD() decltype(auto) GetKey(Offset);

		template<ReflectedData VALUE>
		NOD() decltype(auto) GetValue(const Index&) const;
		template<ReflectedData VALUE>
		NOD() decltype(auto) GetValue(const Index&);
		template<ReflectedData VALUE>
		NOD() decltype(auto) GetValue(Offset) const;
		template<ReflectedData VALUE>
		NOD() decltype(auto) GetValue(Offset);

		template<ReflectedData KEY, ReflectedData VALUE>
		Count Emplace(TPair<KEY, VALUE>&&, const Index& = Index::Back);

		template<ReflectedData KEY, ReflectedData VALUE>
		Count Insert(const TPair<KEY, VALUE>*, Count = 1, const Index& = Index::Back);

		template<ReflectedData KEY, ReflectedData VALUE>
		Map& operator << (const TPair<KEY, VALUE>&);

		template<ReflectedData KEY, ReflectedData VALUE>
		Map& operator << (TPair<KEY, VALUE>&&);

		template<ReflectedData KEY, ReflectedData VALUE>
		Map& operator >> (const TPair<KEY, VALUE>&);

		template<ReflectedData KEY, ReflectedData VALUE>
		Map& operator >> (TPair<KEY, VALUE>&&);

		template<class FUNCTION>
		Count ForEachPair(FUNCTION&&);

		template<class FUNCTION>
		Count ForEachPairRev(FUNCTION&&);

		template<class FUNCTION>
		Count ForEachPair(FUNCTION&&) const;

		template<class FUNCTION>
		Count ForEachPairRev(FUNCTION&&) const;

	protected:
		template<class RETURN, ReflectedData ALT_KEY, ReflectedData ALT_VALUE, bool REVERSE>
		Count ForEachPairInner(TFunctor<RETURN(ALT_KEY, ALT_VALUE)>&&);

		template<class RETURN, ReflectedData ALT_KEY, ReflectedData ALT_VALUE, bool REVERSE>
		Count ForEachPairInner(TFunctor<RETURN(ALT_KEY, ALT_VALUE)>&&) const;

		/// This function declaration is used to decompose a lambda					
		/// You can use it to extract the argument type of the lambda, using		
		/// decltype on the return type. Useful for template deduction in the	
		/// ForEach functions above, purely for convenience							
		template<class RETURN, class FUNCTION, ReflectedData ALT_KEY, ReflectedData ALT_VALUE>
		TPair<ALT_KEY, ALT_VALUE> GetLambdaArguments(RETURN(FUNCTION::*)(ALT_KEY, ALT_VALUE) const) const;

	protected:
		Any mKeys;
	};

} // namespace Langulus::Anyness

#include "Map.inl"

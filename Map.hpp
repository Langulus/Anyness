#pragma once
#include "Any.hpp"

namespace Langulus::Anyness
{
	
	namespace Inner
	{
		/// An abstract pair																		
		struct APair {
			//TODO make abstract, forbid containment
		};
	}
	
	
	///																								
	/// A helper structure for pairing keys and values of any type					
	///																								
	template<ReflectedData K, ReflectedData V>
	struct TPair : public Inner::APair {
		//TODO forbid containment, it's just an intermediate type
		// use std::pair instead for that
		using Key = K;
		using Value = V;
		
		Key mKey;
		Value mValue;

		TPair() = delete;
		constexpr TPair(const TPair&) = default;
		constexpr TPair(TPair&&) noexcept = default;
		constexpr TPair(Key key, Value value)
			: mKey {key}
			, mValue {value} {}
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

		template<ReflectedData>
		NOD() decltype(auto) GetKey(const Index&) const;
		template<ReflectedData>
		NOD() decltype(auto) GetKey(const Index&);
		template<ReflectedData>
		NOD() decltype(auto) GetKey(Offset) const;
		template<ReflectedData>
		NOD() decltype(auto) GetKey(Offset);

		template<ReflectedData>
		NOD() decltype(auto) GetValue(const Index&) const;
		template<ReflectedData>
		NOD() decltype(auto) GetValue(const Index&);
		template<ReflectedData>
		NOD() decltype(auto) GetValue(Offset) const;
		template<ReflectedData>
		NOD() decltype(auto) GetValue(Offset);

		template<ReflectedData K, ReflectedData V>
		Count Emplace(TPair<K, V>&&, const Index& = Index::Back);

		template<ReflectedData K, ReflectedData V>
		Count Insert(const TPair<K, V>*, const Count& = 1, const Index& = Index::Back);

		template<ReflectedData K, ReflectedData V>
		Map& operator << (const TPair<K, V>&);

		template<ReflectedData K, ReflectedData V>
		Map& operator << (TPair<K, V>&&);

		template<ReflectedData K, ReflectedData V>
		Map& operator >> (const TPair<K, V>&);

		template<ReflectedData K, ReflectedData V>
		Map& operator >> (TPair<K, V>&&);

		template<class F>
		Count ForEachPair(F&&);

		template<class F>
		Count ForEachPairRev(F&&);

		template<class F>
		Count ForEachPair(F&&) const;

		template<class F>
		Count ForEachPairRev(F&&) const;

	protected:
		template<class R, ReflectedData ALT_KEY, ReflectedData ALT_VALUE, bool REVERSE>
		Count ForEachPairInner(TFunctor<R(ALT_KEY, ALT_VALUE)>&&);

		template<class R, ReflectedData ALT_KEY, ReflectedData ALT_VALUE, bool REVERSE>
		Count ForEachPairInner(TFunctor<R(ALT_KEY, ALT_VALUE)>&&) const;

		/// This function declaration is used to decompose a lambda					
		/// You can use it to extract the argument type of the lambda, using		
		/// decltype on the return type. Useful for template deduction in the	
		/// ForEach functions above, purely for convenience							
		template<class R, class FUNCTION, ReflectedData ALT_KEY, ReflectedData ALT_VALUE>
		TPair<ALT_KEY, ALT_VALUE> GetLambdaArguments(R(FUNCTION::*)(ALT_KEY, ALT_VALUE) const) const;

	protected:
		Any mKeys;
	};

} // namespace Langulus::Anyness

#include "Map.inl"

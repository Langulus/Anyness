#pragma once
#include "Any.hpp"
#include "TPair.hpp"

namespace Langulus::Anyness
{

	///																								
	///	DATA CONTAINER SPECIALIZATION FOR KEY-VALUE PAIRS							
	///																								
	class Map {
	protected:
		Any mKeys;
		Any mValues;

	public:
		constexpr Map() noexcept = default;
		
		Map(const Map&);
		Map(Map&&) noexcept;
		Map(const Block&, const Block&);

		Map& operator = (const Map&);
		Map& operator = (Map&&) noexcept;

	public:
		NOD() static Map From(DMeta, DMeta, const DataState& = {}) noexcept;

		NOD() static Text GetMapToken(DMeta, DMeta);
		NOD() const Any& GetKeys() const noexcept;
		NOD() Any& GetKeys() noexcept;
		NOD() const Any& GetValues() const noexcept;
		NOD() Any& GetValues() noexcept;
		NOD() constexpr const Count& GetCount() const noexcept;
		NOD() constexpr bool IsEmpty() const noexcept;
		NOD() constexpr Byte* GetRaw() noexcept;
		NOD() constexpr const Byte* GetRaw() const noexcept;
		NOD() constexpr Byte* GetRawEnd() noexcept;
		NOD() constexpr const Byte* GetRawEnd() const noexcept;

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
	};

} // namespace Langulus::Anyness

#include "Map.inl"

#pragma once
#include "Map.hpp"

namespace Langulus::Anyness
{

	///																								
	///	DATA CONTAINER SPECIALIZATION FOR KEY-VALUE PAIRS							
	///																								
	template<ReflectedData K, ReflectedData V> 
	class TMap : public Map {
	public:
		using Pair = TPair<K, V>;
		using PairRef = TPair<K&, V&>;
		using PairConstRef = TPair<const K&, const V&>;
		using PairPtr = TPair<K*, V*>;
		using PairConstPtr = TPair<const K*, const V*>;
		using Key = K;
		using KeyList = TAny<K>;
		using Value = V;
		using ValueList = TAny<V>;

		TMap();
		TMap(const TMap&);
		TMap(TMap&&) noexcept;
		TMap(const DataState&, Key*, Value*, const Count&);

		TMap& operator = (const TMap&);
		TMap& operator = (TMap&&) noexcept;

	public:
		RANGED_FOR_INTEGRATION(TMap, V)

		void Clear();
		void Reset();

		NOD() TMap Clone() const;

		NOD() decltype(auto) Keys() const noexcept;
		NOD() decltype(auto) Keys() noexcept;

		NOD() decltype(auto) Values() const noexcept;
		NOD() decltype(auto) Values() noexcept;
		
		NOD() Index FindKey(const Key&) const;
		NOD() Index FindValue(const Value&) const;
		
		NOD() auto GetPair(const Index&);
		NOD() auto GetPair(const Index&) const;

		NOD() auto GetPair(const Offset&) noexcept;
		NOD() auto GetPair(const Offset&) const noexcept;

		NOD() decltype(auto) operator [] (const Key&);
		NOD() decltype(auto) operator [] (const Key&) const;

		template<ReflectedData = K>
		NOD() decltype(auto) GetKey(const Index&) const;
		template<ReflectedData = K>
		NOD() decltype(auto) GetKey(const Index&);
		template<ReflectedData = K>
		NOD() decltype(auto) GetKey(const Offset&) const noexcept;
		template<ReflectedData = K>
		NOD() decltype(auto) GetKey(const Offset&) noexcept;

		template<ReflectedData = V>
		NOD() decltype(auto) GetValue(const Index&) const;
		template<ReflectedData = V>
		NOD() decltype(auto) GetValue(const Index&);
		template<ReflectedData = V>
		NOD() decltype(auto) GetValue(const Offset&) const noexcept;
		template<ReflectedData = V>
		NOD() decltype(auto) GetValue(const Offset&) noexcept;

		Count RemoveKey(const Key&);
		Count RemoveValue(const Value&);

		template<class ALT_V>
		Count Merge(const K&, const ALT_V&)
			requires CopyConstructible<K> && CopyConstructible<V>;

		Count Merge(const TMap&)
			requires CopyConstructible<K> && CopyConstructible<V>;

		Count Emplace(Pair&&, const Index& = Index::Back);
		Count Insert(const Pair*, const Count& = 1, const Index& = Index::Back);

		TMap& operator << (Pair&&);
		TMap& operator >> (Pair&&);

		Count Add(K&&, V&&, const Index& = Index::Back)
			requires MoveConstructible<K> && MoveConstructible<V>;
		
		Count Add(const K&, V&&, const Index& = Index::Back)
			requires CopyConstructible<K> && MoveConstructible<V>;
		
		Count Add(K&&, const V&, const Index& = Index::Back) 
			requires MoveConstructible<K> && CopyConstructible<V>;
		
		Count Add(const K&, const V&, const Index& = Index::Back) 
			requires CopyConstructible<K> && CopyConstructible<V>;
		
		Count Add(K&, V&, const Index& = Index::Back)
			requires CopyConstructible<K> && CopyConstructible<V>;

		void Sort(const Index&);

		template<Function F>
		Count ForEach(F&&);
		template<Function F>
		Count ForEach(F&&) const;

		template<Function F>
		Count ForEachRev(F&&);
		template<Function F>
		Count ForEachRev(F&&) const;

	private:
		template<class RETURN, ReflectedData ALT_KEY, ReflectedData ALT_VALUE, bool REVERSE>
		Count ForEachInner(TFunctor<RETURN(ALT_KEY, ALT_VALUE)>&&);

		template<class RETURN, ReflectedData ALT_KEY, ReflectedData ALT_VALUE, bool REVERSE>
		Count ForEachInner(TFunctor<RETURN(ALT_KEY, ALT_VALUE)>&&) const;
	};

} // namespace Langulus::Anyness

#include "TMap.inl"

#pragma once
#include "Map.hpp"
#include "Trait.hpp"

namespace Langulus::Anyness
{

	///																								
	///	DATA CONTAINER SPECIALIZATION FOR KEY-VALUE PAIRS							
	///																								
	template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE> 
	class TMap : public Map {
		REFLECT_MANUALLY(TMap) {
			auto keyType = DataID::Reflect<KEY>();
			auto valueType = DataID::Reflect<VALUE>();

			static Text name, info;
			if (name.IsEmpty()) {
				name += Map::GetMapToken(keyType, valueType);
				name = name + "," + name + "Ptr," + name + "ConstPtr";
				info += "a container that maps ";
				info += keyType->GetToken();
				info += " to ";
				info += valueType->GetToken();
			}

			auto reflection = RTTI::ReflectData::From<ME>(name, info);
			reflection.mIsDeep = true;
			reflection.template SetBases<ME>(
				REFLECT_BASE(Map));
			return reflection;
		}

	public:
		using Pair = TPair<KEY, VALUE>;

		TMap();
		TMap(const TMap&);
		TMap(TMap&&) noexcept;
		TMap(const DState&, KEY*, VALUE*, pcptr);

		ME& operator = (const ME&);
		ME& operator = (ME&&) noexcept;

	public:
		/// Range-based for-statement integration											
		PC_RANGED_FOR_INTEGRATION(VALUE, Values().GetRaw(), Values().GetCount())

		void Clear();
		void Reset();

		NOD() ME Clone() const;

		NOD() const TAny<KEY>& Keys() const noexcept;
		NOD() TAny<KEY>& Keys() noexcept;

		NOD() const TAny<VALUE>& Values() const noexcept;
		NOD() TAny<VALUE>& Values() noexcept;
		
		NOD() Index FindKey(const KEY&) const;
		NOD() Index FindValue(const VALUE&) const;
		
		NOD() TPair<KEY*, VALUE*> GetPair(const Index&);
		NOD() TPair<const KEY*, const VALUE*> GetPair(const Index&) const;

		NOD() TPair<KEY*, VALUE*> GetPair(pcptr);
		NOD() TPair<const KEY*, const VALUE*> GetPair(pcptr) const;

		NOD() auto& operator [] (const KEY&);
		NOD() auto& operator [] (const KEY&) const;

		template<RTTI::ReflectedData K = KEY>
		NOD() decltype(auto) GetKey(const Index&) const;
		template<RTTI::ReflectedData K = KEY>
		NOD() decltype(auto) GetKey(Index);
		template<RTTI::ReflectedData K = KEY>
		NOD() decltype(auto) GetKey(pcptr) const;
		template<RTTI::ReflectedData K = KEY>
		NOD() decltype(auto) GetKey(pcptr);

		template<RTTI::ReflectedData V = VALUE>
		NOD() decltype(auto) GetValue(const Index&) const;
		template<RTTI::ReflectedData V = VALUE>
		NOD() decltype(auto) GetValue(Index);
		template<RTTI::ReflectedData V = VALUE>
		NOD() decltype(auto) GetValue(pcptr) const;
		template<RTTI::ReflectedData V = VALUE>
		NOD() decltype(auto) GetValue(pcptr);

		pcptr RemoveKey(const KEY&);
		pcptr RemoveValue(const VALUE&);

		template<class MERGED_VALUE>
		pcptr Merge(const KEY&, const MERGED_VALUE&) requires CopyConstructible<KEY> && CopyConstructible<VALUE>;

		pcptr Merge(const ME&) requires CopyConstructible<KEY> && CopyConstructible<VALUE>;

		pcptr Emplace(Pair&&, const Index& = uiBack);
		pcptr Insert(const Pair*, const pcptr = 1, const Index& = uiBack);

		ME& operator << (Pair&&);
		ME& operator >> (Pair&&);

		pcptr Add(KEY&&, VALUE&&, const Index& = uiBack) requires MoveConstructible<KEY> && MoveConstructible<VALUE>;
		pcptr Add(const KEY&, VALUE&&, const Index& = uiBack) requires CopyConstructible<KEY> && MoveConstructible<VALUE>;
		pcptr Add(KEY&&, const VALUE&, const Index& = uiBack) requires MoveConstructible<KEY> && CopyConstructible<VALUE>;
		pcptr Add(const KEY&, const VALUE&, const Index& = uiBack) requires CopyConstructible<KEY> && CopyConstructible<VALUE>;
		pcptr Add(KEY&, VALUE&, const Index& = uiBack) requires CopyConstructible<KEY> && CopyConstructible<VALUE>;

		void Sort(const Index&);

		template<class FUNCTION>
		pcptr ForEach(FUNCTION&&);

		template<class FUNCTION>
		pcptr ForEachRev(FUNCTION&&);

		template<class FUNCTION>
		pcptr ForEach(FUNCTION&&) const;

		template<class FUNCTION>
		pcptr ForEachRev(FUNCTION&&) const;

	private:
		template<class RETURN, RTTI::ReflectedData ALT_KEY, RTTI::ReflectedData ALT_VALUE, bool REVERSE>
		pcptr ForEachInner(TFunctor<RETURN(ALT_KEY, ALT_VALUE)>&&);

		template<class RETURN, RTTI::ReflectedData ALT_KEY, RTTI::ReflectedData ALT_VALUE, bool REVERSE>
		pcptr ForEachInner(TFunctor<RETURN(ALT_KEY, ALT_VALUE)>&&) const;
	};

} // namespace Langulus::Anyness

#include "TMap.inl"
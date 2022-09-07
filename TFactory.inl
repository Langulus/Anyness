///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "TFactory.hpp"

#define TEMPLATE() template<FactoryProducible T, FactoryUsage USAGE>
#define FACTORY() TFactory<T, USAGE>

namespace Langulus::Anyness
{

	/// Construction																				
	///	@param owner - the factory owner													
	TEMPLATE()
	FACTORY()::TFactory(Producer* owner)
		: mFactoryOwner {owner} {}

	/// Move-assignment remaps all elements to the new instance owner				
	///	@attention notice how mFactoryOwner never changes on both sides		
	///	@param other - the factory to move												
	TEMPLATE()
	FACTORY()& FACTORY()::operator = (TFactory&& other) noexcept {
		mData = Move(other.mData);
		mHashmap = Move(other.mHashmap);
		mReusable = other.mReusable;
		other.mReusable = nullptr;
		for (auto& item : mData)
			item.mFactory = this;
		return *this;
	}

	/// Reset the factory																		
	TEMPLATE()
	void FACTORY()::Reset() {
		mData.Reset();
		mHashmap.Reset();
		mReusable = nullptr;
	}

	/// Destroys an element inside factory													
	///	@attention will throw if element has more than a single use				
	///	@attention provided pointer is invalid after this function completes	
	///				  if pointer is owned by the factory								
	///	@param item - pointer of the element to destroy								
	TEMPLATE()
	void FACTORY()::Destroy(T* item) {
		if (!mData.Owns(item) || item->GetReferences() == 0)
			return;

		if (item->GetReferences() > 1)
			Throw<Except::Destruct>(
				"Can't destroy element in use at TFactory::Destroy");

		// Remove from hashmap															
		auto& list = mHashmap[item->mHash];
		list.Remove(item);
		if (list.IsEmpty())
			mHashmap.RemoveKey(item->mHash);

		// Destroy the item																
		item->~T();
		item->mNextFreeElement = mReusable;
		mReusable = item;
	}

	/// Create an element inside the factory, using the provided arguments		
	///	@param descriptor - the element descriptor, usually from a construct	
	///	@param arguments - additional arguments for element constructor		
	///	@return the new element, or an existent element if unique usage		
	TEMPLATE()
	template<class... A>
	T* FACTORY()::Create(const Any& descriptor, A&&... arguments) {
		if constexpr (USAGE == FactoryUsage::Unique) {
			// Check if new descriptor matches any of the available			
			const auto hash = descriptor.GetHash();
			const auto found = mHashmap.FindKeyIndex(hash);
			if (found) {
				for (auto candidate : mHashmap.GetValue(found)) {
					if (candidate->mDescriptor != descriptor) //TODO orderless comparison instead
						continue;
					return candidate;
				}
			}
		}

		if (mReusable) {
			// Reuse a slot																
			auto memory = mReusable;
			mReusable = mReusable->mNextFreeElement;
			auto result = new (memory) T {
				this, descriptor, Forward<A>(arguments)...
			};
			mHashmap[result->mHash] << result;
			return result;
		}

		// If this is reached, then a reallocation is required				
		mData.Emplace(this, descriptor, Forward<A>(arguments)...);
		mHashmap[mData.Last().mHash] << &mData.Last();
		return &mData.Last();
	}

} // namespace Langulus::Anyness

#undef TEMPLATE
#undef FACTORY
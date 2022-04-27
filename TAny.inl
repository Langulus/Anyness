#pragma once
#include "TAny.hpp"

#define TEMPLATE() template<ReflectedData T>

namespace Langulus::Anyness
{

	/// Default construction																	
	/// TAny is type-constrained and always has a type									
	TEMPLATE()
	TAny<T>::TAny()
		: Any {Block {DataState::Typed, MetaData::Of<T>()}} { }

	/// Shallow-copy construction (const)													
	///	@param other - the TAny to reference											
	TEMPLATE()
	TAny<T>::TAny(const TAny& other)
		: Any {static_cast<const Any&>(other)} { }

	/// Shallow-copy construction																
	///	@param other - the TAny to reference											
	TEMPLATE()
	TAny<T>::TAny(TAny& other)
		: Any {static_cast<Any&>(other)} { }

	/// Move construction																		
	///	@param other - the TAny to move													
	TEMPLATE()
	TAny<T>::TAny(TAny&& other) noexcept
		: Any {Forward<Any>(other)} { }

	/// Shallow copy construction from Any, that checks type							
	/// Any can contain anything, so there's a bit of type-checking overhead	
	///	@param other - the anyness to reference										
	TEMPLATE()
	TAny<T>::TAny(Any& other) {
		if (!InterpretsAs(other.GetType())) {
			throw Except::Copy(Logger::Error()
				<< "Bad shallow-copy-construction for TAny: from "
				<< GetToken() << " to " << other.GetToken());
		}

		CopyProperties<false>(other);
		Keep();
	}

	/// Shallow copy construction from Any, that checks type							
	/// Any can contain anything, so there's a bit of type-checking overhead	
	///	@param other - the anyness to reference										
	TEMPLATE()
	TAny<T>::TAny(const Any& other)
		: TAny{const_cast<Any&>(other)} {
		MakeConstant();
	}

	/// Move-construction from Any, that checks type									
	/// Any can contain anything, so there's a bit of type-checking overhead	
	///	@param other - the container to move											
	TEMPLATE()
	TAny<T>::TAny(Any&& other) {
		if (!InterpretsAs(other.GetType())) {
			throw Except::Copy(Logger::Error()
				<< "Bad move-construction for TAny: from "
				<< GetToken() << " to " << other.GetToken());
		}

		CopyProperties<false>(other);
		other.ResetMemory();
		other.ResetState();
	}

	/// Shallow copy construction from blocks (const)									
	/// Block can contain anything, so there's a bit of type-checking overhead	
	///	@param copy - the block to reference											
	TEMPLATE()
	TAny<T>::TAny(const Block& copy)
		: TAny {Any {copy}} { }
	
	/// Shallow copy construction from blocks												
	/// Block can contain anything, so there's a bit of type-checking overhead	
	///	@param copy - the block to reference											
	TEMPLATE()
	TAny<T>::TAny(Block& copy)
		: TAny {Any {copy}} { }

	/// Move construction - moves block and references content						
	/// Since we are not aware if that block is referenced, we reference it		
	/// We do not reset the other block to avoid memory leaks						
	///	@param other - the block to move													
	TEMPLATE()
	TAny<T>::TAny(Block&& copy)
		: TAny {Any {Forward<Block>(copy)}} { }

	/// Copy other but do not reference it, because it is disowned					
	///	@param other - the block to copy													
	TEMPLATE()
	TAny<T>::TAny(const Disowned<TAny>& other) noexcept
		: Any {other.Forward<Any>()} { }	
	
	/// Move other, but do not bother cleaning it up, because it is disowned	
	///	@param other - the block to move													
	TEMPLATE()
	TAny<T>::TAny(Abandoned<TAny>&& other) noexcept
		: Any {other.Forward<Any>()} { }	
	
	/// Construct by moving a dense value of non-block type							
	///	@param initial - the dense value to forward and emplace					
	TEMPLATE()
	TAny<T>::TAny(T&& initial) requires (not Anyness::IsDeep<T>)
		: Any {Forward<T>(initial)} { }

	/// Construct by copying/referencing value of non-block type					
	///	@param initial - the dense value to shallow-copy							
	TEMPLATE()
	TAny<T>::TAny(const T& initial) requires (not Anyness::IsDeep<T>)
		: Any {initial} { }

	/// Construct by copying/referencing value of non-block type					
	///	@param other - the dense value to shallow-copy								
	TEMPLATE()
	TAny<T>::TAny(T& other) requires (not Anyness::IsDeep<T>)
		: TAny {const_cast<const T&>(other)} { }

	/// Construct manually from an array													
	///	@param raw - raw memory to reference											
	///	@param count - number of items inside 'raw'									
	TEMPLATE()
	TAny<T>::TAny(const T* raw, const Count& count)
		: Any {
			Block {
				DataState::Constrained, MetaData::Of<T>(), count, 
				reinterpret_cast<const Byte*>(raw)
			}
		} {
		// Data is not owned by us, it may be on the stack						
		// We should monopolize the memory to avoid segfaults, in the		
		// case of the byte container being initialized with temporary		
		// data on the stack																
		TakeAuthority();
	}

	/// Shallow constant copy operator														
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (const TAny<T>& other) {
		other.Keep();
		Free();
		CopyProperties<true>(other);
		MakeConstant();
		return *this;
	}

	/// Shallow mutable copy operator														
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (TAny<T>& other) {
		other.Keep();
		Free();
		CopyProperties<true>(other);
		return *this;
	}
		
	/// Move operator																				
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (TAny<T>&& other) {
		Free();
		CopyProperties<true>(other);
		other.ResetMemory();
		other.ResetState();
		return *this;
	}

	/// Shallow copy operator																	
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (const Any& other) {
		if (!InterpretsAs(other.mType)) {
			throw Except::Copy(Logger::Error()
				<< "Bad shallow-copy-assignment for TAny: from "
				<< GetToken() << " to " << other.GetToken());
		}

		// Overwrite everything except the type-constraint						
		other.Keep();
		Free();
		ResetState();
		CopyProperties<false>(other);
		return *this;
	}

	/// Move operator																				
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (Any&& other) {
		if (!InterpretsAs(other.mType)) {
			throw Except::Copy(Logger::Error()
				<< "Bad move-copy-assignment for TAny: from "
				<< GetToken() << " to " << other.GetToken());
		}

		// Overwrite everything except the type-constraint						
		Free();
		ResetState();
		CopyProperties<false>(other);
		other.ResetMemory();
		other.ResetState();
		return *this;
	}

	/// Shallow copy operator																	
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (const Block& other) {
		return TAny<T>::operator = (Any {other});
	}

	/// Shallow copy operator																	
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (Block&& other) {
		return TAny<T>::operator = (Any {Forward<Block>(other)});
	}

	/// Assign by shallow-copying some value different from Any or Block			
	///	@param value - the value to copy													
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (const T& value) requires (not Anyness::IsDeep<T>) {
		return TAny<T>::operator = (TAny<T> {value});
	}

	TEMPLATE()
	TAny<T>& TAny<T>::operator = (T& value) requires (not Anyness::IsDeep<T>) {
		return TAny<T>::operator = (const_cast<const T&>(value));
	}

	/// Assign by moving some value different from Any or Block						
	///	@param value - the value to move													
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (T&& value) requires (not Anyness::IsDeep<T>) {
		return TAny<T>::operator = (TAny<T> {Forward<T>(value)});
	}

	/// An internal function used to copy members, without copying type and		
	/// without overwriting states, if required											
	///	@param other - the block to copy from											
	TEMPLATE()
	template<bool OVERWRITE>
	void TAny<T>::CopyProperties(const Block& other) noexcept {
		mRaw = other.mRaw;
		mCount = other.mCount;
		mReserved = other.mReserved;
		if constexpr (OVERWRITE)
			mState = other.mState;
		else
			mState += other.mState;
		mEntry = other.mEntry;
	}

	/// Reset container state																	
	TEMPLATE()
	void TAny<T>::ResetState() noexcept {
		Block::ResetState<true>();
	}

	/// Check if contained data can be interpreted as a given type					
	/// Beware, direction matters (this is the inverse of CanFit)					
	///	@param type - the type check if current type interprets to				
	///	@return true if able to interpret current type to 'type'					
	TEMPLATE()
	bool TAny<T>::InterpretsAs(DMeta type) const {
		return mType->InterpretsAs<Langulus::IsSparse<T>>(type);
	}

	/// Check if contained data can be interpreted as a given count of type		
	/// For example: a vec4 can interpret as float[4]									
	/// Beware, direction matters (this is the inverse of CanFit)					
	///	@param type - the type check if current type interprets to				
	///	@param count - the number of elements to interpret as						
	///	@return true if able to interpret current type to 'type'					
	TEMPLATE()
	bool TAny<T>::InterpretsAs(DMeta type, Count count) const {
		return mType->InterpretsAs(type, count);
	}

	/// Wrap stuff in a container																
	///	@param anything - pack it inside a dense container							
	///	@returns the pack containing the data											
	TEMPLATE()
	TAny<T> TAny<T>::Wrap(const T& anything) {
		TAny<T> temp;
		temp << anything;
		return temp;
	}

	/// Pack a c-array inside container, doing a shallow copy						
	///	@param anything - pack it inside a dense container							
	///	@returns the pack containing the data											
	TEMPLATE()
	template<Count COUNT>
	TAny<T> TAny<T>::Wrap(const T(&anything)[COUNT]) {
		TAny<T> temp;
		temp.Reserve(COUNT);
		for (Count i = 0; i < COUNT; ++i)
			temp << anything[i];
		return temp;
	}

	/// Pack an array inside container, doing a shallow copy							
	///	@param anything - pack it inside a dense container							
	///	@param count - number of items													
	///	@returns the pack containing the data											
	TEMPLATE()
	TAny<T> TAny<T>::Wrap(const T* anything, const Count& count) {
		TAny<T> temp;
		temp.Reserve(count);
		for (Count i = 0; i < count; ++i)
			temp << anything[i];
		return temp;
	}

	/// Allocate 'count' elements and fill the container with zeroes				
	TEMPLATE()
	void TAny<T>::Null(const Count& count) {
		Allocate(count, false, true);
		FillMemory(mRaw, {}, GetSize());
	}

	/// Clear the container, destroying all elements,									
	/// but retaining allocation if possible												
	TEMPLATE()
	void TAny<T>::Clear() {
		if (!mCount)
			return;

		if (GetReferences() == 1) {
			// Only one use - just destroy elements and reset count,			
			// reusing the allocation for later										
			CallDestructors();
			ClearInner();
		}
		else {
			// We're forced to reset the memory, because it's in use			
			// Keep the type and state, though										
			const auto state = GetUnconstrainedState();
			Reset();
			mState += state;
		}
	}

	/// Reset the container, destroying all elements, and deallocating			
	TEMPLATE()
	void TAny<T>::Reset() {
		Free();
		ResetMemory();
		ResetState();
	}

	/// Clone the templated container														
	TEMPLATE()
	TAny<T> TAny<T>::Clone() const {
		return {Any::Clone()};
	}

	/// Return the typed raw data (const)													
	///	@return a constant pointer to the first element in the array			
	TEMPLATE()
	const T* TAny<T>::GetRaw() const noexcept {
		return Any::GetRawAs<T>();
	}

	/// Return the typed raw data																
	///	@return a mutable pointer to the first element in the array				
	TEMPLATE()
	T* TAny<T>::GetRaw() noexcept {
		return Any::GetRawAs<T>();
	}

	/// Return the typed raw data end pointer (const)									
	///	@return a constant pointer to one past the last element in the array	
	TEMPLATE()
	const T* TAny<T>::GetRawEnd() const noexcept {
		return Any::GetRawAs<T>() + mCount;
	}

	/// Return the typed raw data	end pointer												
	///	@return a mutable pointer to one past the last element in the array	
	TEMPLATE()
	T* TAny<T>::GetRawEnd() noexcept {
		return Any::GetRawAs<T>() + mCount;
	}

	/// Get an element in the way you want (const, unsafe)							
	/// This is a statically optimized variant of Block::Get							
	TEMPLATE()
	template<ReflectedData K>
	decltype(auto) TAny<T>::Get(const Offset& index) const noexcept {
		const T& element = GetRaw()[index];
		if constexpr (Langulus::IsDense<T> && Langulus::IsDense<K>)
			// IsDense -> IsDense (returning a reference)								
			return static_cast<const Decay<K>&>(element);
		else if constexpr (Langulus::IsSparse<T> && Langulus::IsDense<K>)
			// IsSparse -> IsDense (returning a reference)							
			return static_cast<const Decay<K>&>(*element);
		else if constexpr (Langulus::IsDense<T> && Langulus::IsSparse<K>)
			// IsDense -> IsSparse (returning a pointer)								
			return static_cast<const Decay<K>*>(&element);
		else
			// IsSparse -> IsSparse (returning a reference to pointer)			
			return static_cast<const Decay<K>* const&>(element);
	}

	/// Get an element in the way you want (unsafe)										
	/// This is a statically optimized variant of Block::Get							
	TEMPLATE()
	template<ReflectedData K>
	decltype(auto) TAny<T>::Get(const Offset& index) noexcept {
		T& element = GetRaw()[index];
		if constexpr (Langulus::IsDense<T> && Langulus::IsDense<K>)
			// IsDense -> IsDense (returning a reference)								
			return static_cast<Decay<K>&>(element);
		else if constexpr (Langulus::IsSparse<T> && Langulus::IsDense<K>)
			// IsSparse -> IsDense (returning a reference)							
			return static_cast<Decay<K>&>(*element);
		else if constexpr (Langulus::IsDense<T> && Langulus::IsSparse<K>)
			// IsDense -> IsSparse (returning a pointer)								
			return static_cast<Decay<K>*>(&element);
		else
			// IsSparse -> IsSparse (returning a reference to pointer)			
			return static_cast<Decay<K>*&>(element);
	}

	/// Access typed dense elements via a simple index (unsafe)						
	///	@param idx - the index to get														
	///	@return a reference to the element												
	TEMPLATE()
	decltype(auto) TAny<T>::operator [] (const Offset& index) noexcept requires Langulus::IsDense<T> {
		return Get<T>(index);
	}

	TEMPLATE()
	decltype(auto) TAny<T>::operator [] (const Offset& index) const noexcept requires Langulus::IsDense<T> {
		return Get<T>(index);
	}

	/// Access typed dense elements via a complex index (const, safe)				
	///	@param idx - the index to get														
	///	@return a constant reference to the element									
	TEMPLATE()
	decltype(auto) TAny<T>::operator [] (const Index& index) requires Langulus::IsDense<T> {
		return Get<T>(Block::ConstrainMore<T>(index).GetOffset());
	}

	TEMPLATE()
	decltype(auto) TAny<T>::operator [] (const Index& index) const requires Langulus::IsDense<T> {
		return const_cast<TAny<T>&>(*this).operator [] (index);
	}

	/// Access typed sparse elements via a simple index (unsafe)					
	/// Will wrap pointer in a SparseElement wrapper, in order to reference		
	///	@param idx - the index to get														
	///	@return a reference to the element												
	TEMPLATE()
	typename TAny<T>::SparseElement TAny<T>::operator [] (const Offset& index) noexcept requires Langulus::IsSparse<T> {
		return {Get<T>(index)};
	}

	TEMPLATE()
	decltype(auto) TAny<T>::operator [] (const Offset& index) const noexcept requires Langulus::IsSparse<T> {
		return Get<T>(index);
	}

	/// Access typed sparse elements via a complex index (const)					
	/// Will wrap pointer in a SparseElement wrapper, in order to reference		
	///	@param idx - the index to get														
	///	@return a constant reference to the element									
	TEMPLATE()
	typename TAny<T>::SparseElement TAny<T>::operator [] (const Index& index) requires Langulus::IsSparse<T> {
		return {Get<T>(ConstrainMore<T>(index).GetOffset())};
	}

	TEMPLATE()
	decltype(auto) TAny<T>::operator [] (const Index& index) const requires Langulus::IsSparse<T> {
		return Get<T>(ConstrainMore<T>(index).GetOffset());
	}

	/// Access last element (unsafe)															
	///	@return a reference to the last element										
	TEMPLATE()
	decltype(auto) TAny<T>::Last() {
		return Get<T>(mCount - 1);
	}

	/// Access last element (const, unsafe)												
	///	@return a constant reference to the last element							
	TEMPLATE()
	decltype(auto) TAny<T>::Last() const {
		return Get<T>(mCount - 1);
	}

	/// Check if the contained type is a pointer											
	///	@return true if container contains pointers									
	TEMPLATE()
	constexpr bool TAny<T>::IsSparse() const noexcept {
		return Langulus::IsSparse<T>;
	}

	/// Check if the contained type is not a pointer									
	///	@return true if container contains sequential data							
	TEMPLATE()
	constexpr bool TAny<T>::IsDense() const noexcept {
		return Langulus::IsDense<T>;
	}

	/// Get the size of a single contained element, in bytes							
	///	@return the number of bytes a single element contains						
	TEMPLATE()
	constexpr Size TAny<T>::GetStride() const noexcept {
		return sizeof(T); 
	}

	/// Insert an item by move-construction												
	///	@param item - the item to move													
	///	@param idx - the place where to emplace the item							
	///	@return 1 if successful																
	TEMPLATE()
	Count TAny<T>::Emplace(T&& item, const Index& idx) {
		return Any::Emplace<T, false>(Forward<T>(item), idx);
	}

	/// Insert by copy-construction															
	///	@param items - an array of items to insert									
	///	@param count - the number of items to insert									
	///	@param idx - the place where to insert the items							
	///	@return number of inserted items													
	TEMPLATE()
	Count TAny<T>::Insert(const T* items, const Count count, const Index& idx) {
		return Any::Insert<T, false>(items, count, idx);
	}

	/// Push data at the back by copy-construction										
	///	@param other - the item to insert												
	///	@return a reference to this container for chaining							
	TEMPLATE()
	TAny<T>& TAny<T>::operator << (const T& other) {
		Any::Insert<T, false>(&other, 1, Index::Back);
		return *this;
	}

	/// Push data at the back by move-construction										
	///	@param other - the item to move													
	///	@return a reference to this container for chaining							
	TEMPLATE()
	TAny<T>& TAny<T>::operator << (T&& other) {
		Any::Emplace<T, false>(Forward<T>(other), Index::Back);
		return *this;
	}

	/// Push data at the front by copy-construction										
	///	@param other - the item to insert												
	///	@return a reference to this container for chaining							
	TEMPLATE()
	TAny<T>& TAny<T>::operator >> (const T& other) {
		Any::Insert<T, false>(&other, 1, Index::Front);
		return *this;
	}

	/// Push data at the front by move-construction										
	///	@param other - the item to move													
	///	@return a reference to this container for chaining							
	TEMPLATE()
	TAny<T>& TAny<T>::operator >> (T&& other) {
		Any::Emplace<T, false>(Forward<T>(other), Index::Front);
		return *this;
	}

	/// Merge anything compatible to container											
	/// By merging we mean anything that is not found is pushed						
	///	@param items - the items to find and push										
	///	@param count - number of items to push											
	///	@param idx - the place to insert them											
	///	@return the number of inserted items											
	TEMPLATE()
	Count TAny<T>::Merge(const T* items, const Count count, const Index& idx) {
		return Any::Merge<T, false>(items, count, idx);
	}

	/// Push data at the back																	
	TEMPLATE()
	TAny<T>& TAny<T>::operator <<= (const T& other) {
		Merge(&other, 1, Index::Back);
		return *this;
	}

	/// Push data at the front																	
	TEMPLATE()
	TAny<T>& TAny<T>::operator >>= (const T& other) {
		Merge(&other, 1, Index::Front);
		return *this;
	}

	/// Find element(s) index inside container											
	///	@tparam ALT_T - type of the element to search for							
	///	@param item - the item to search for											
	///	@param index - the index to start searching at								
	///	@return the index of the found item, or Index::None if none found		
	TEMPLATE()
	template<ReflectedData ALT_T>
	Index TAny<T>::Find(const ALT_T& item, const Index& index) const {
		static_assert(IsComparable<T, ALT_T>, 
			"Provided type ALT_T is not comparable to the contained type T");
			
		if (!mCount)
			return Index::None;

		// Setup the iterator															
		Index::Type starti, istep;
		switch (index.mIndex) {
		case Index::Front:
			starti = 0;
			istep = 1;
			break;
		case Index::Back:
			starti = mCount - 1;
			istep = -1;
			break;
		default:
			starti = Constrain(index).mIndex;
			if (starti + 1 >= mCount)
				return Index::None;
			istep = index >= 0 ? 1 : -1;
		}

		auto data = GetRaw();
		if constexpr (Langulus::IsSparse<ALT_T>) {
			if constexpr (Langulus::IsSparse<T>) {
				// Searching for pointer inside a sparse container				
				for (auto i = starti; i < mCount && i >= 0; i += istep) {
					if constexpr (Inherits<T, ALT_T>) {
						if (static_cast<MakeConst<T>>(data[i]) == item)
							return i;
					}
					else if constexpr (Inherits<ALT_T, T>) {
						if (data[i] == static_cast<MakeConst<ALT_T>>(item))
							return i;
					}
					else {
						if (data[i] == item)
							return i;
					}
				}
			}
			else {
				// Searching for pointer inside a dense container				
				for (auto i = starti; i < mCount && i >= 0; i += istep) {
					if constexpr (Inherits<T, ALT_T>) {
						if (static_cast<MakeConst<T>>(data + i) == item)
							return i;
					}
					else if constexpr (Inherits<ALT_T, T>) {
						if (data + i == static_cast<MakeConst<ALT_T>>(item))
							return i;
					}
					else {
						if (data + i == item)
							return i;
					}
				}
			}
		}
		else {
			if constexpr (Langulus::IsSparse<T>) {
				// Searching for value inside a sparse container				
				for (Index::Type i = starti; i < mCount && i >= 0; i += istep) {
					// Test pointers first												
					if constexpr (Inherits<T, ALT_T>) {
						if (static_cast<MakeConst<T>>(data[i]) == &item)
							return i;
					}
					else if constexpr (Inherits<ALT_T, T>) {
						if (data[i] == static_cast<MakeConst<ALT_T>>(&item))
							return i;
					}
					else {
						if (data[i] == &item)
							return i;
					}
					
					// Test by value														
					if (*data[i] == item)
						return i;
				}
			}
			else {
				// Searching for value inside a dense container					
				for (Index::Type i = starti; i < mCount && i >= 0; i += istep) {
					// Test pointers first												
					if constexpr (Inherits<T, ALT_T>) {
						if (static_cast<MakeConst<T>>(data + i) == &item)
							return i;
					}
					else if constexpr (Inherits<ALT_T, T>) {
						if (data + 1 == static_cast<MakeConst<ALT_T>>(&item))
							return i;
					}
					else {
						if (data + 1 == &item)
							return i;
					}
					
					// Test by value														
					if (data[i] == item)
						return i;
				}
			}
		}

		// If this is reached, then no match was found							
		return Index::None;
	}

	/// Remove matching items																	
	///	@tparam ALT_T - type of the element to remove								
	///	@param item - the item to search for to remove								
	///	@param index - the index to start searching at								
	///	@return the number of removed items												
	TEMPLATE()
	template<ReflectedData ALT_T>
	Count TAny<T>::Remove(const ALT_T& item, const Index& index) {
		Count removed {};
		for (Offset i = 0; i < mCount; ++i) {
			const auto found = Find<ALT_T>(item, index);
			if (found)
				removed += RemoveIndex(found.GetOffset(), 1);
		}

		return removed;
	}

	/// Sort the pack																				
	TEMPLATE()
	void TAny<T>::Sort(const Index& first) {
		if constexpr (IsSortable<T>)
			Any::Sort<T>(first);
		else LANGULUS_ASSERT("Can't sort container - T is not sortable");
	}

	/// Remove elements on the back															
	TEMPLATE()
	TAny<T>& TAny<T>::Trim(const Count& count) {
		Any::Trim(count);
		return *this;
	}

	/// Swap two elements																		
	///	@param from - the first element													
	///	@param to - the second element													
	TEMPLATE()
	void TAny<T>::Swap(const Offset& from, const Offset& to) {
		Any::Swap<T>(from, to);
	}

	/// Swap two elements using special indices											
	///	@param from - the first element													
	///	@param to - the second element													
	TEMPLATE()
	void TAny<T>::Swap(const Index& from, const Index& to) {
		Any::Swap<T>(from, to);
	}

	/// Clone container array into a new owned memory block							
	/// If we have jurisdiction, the memory won't move at all						
	TEMPLATE()
	void TAny<T>::TakeAuthority() {
		if (mEntry)
			return;

		operator = (Clone());
	}



	///																								
	///	Sparse element implementation														
	///																								
	
	/// When overwriting the element, previous pointer must be dereferenced		
	/// and the new one - referenced															
	///	@param pointer - the pointer to set												
	///	@return a reference to this sparse element									
	TEMPLATE()
	TAny<T>::SparseElement& TAny<T>::SparseElement::operator = (T pointer) {
		if (mElement == pointer)
			return *this;

		const auto meta = MetaData::Of<T>();
		if (mElement) {
			// Dereference/destroy the previous element							
			auto entry = Allocator::Find(meta, mElement);
			if (entry->mReferences == 1) {
				delete mElement;
				entry->Deallocate();
			}

			--entry->mReferences;
		}

		// Set and reference the new element										
		mElement = pointer;
		Allocator::Reference(meta, mElement, 1);
		return *this;
	}

	/// When overwriting with nullptr, just dereference/destroy previous			
	///	@param pointer - null pointer														
	///	@return a reference to this sparse element									
	TEMPLATE()
	TAny<T>::SparseElement& TAny<T>::SparseElement::operator = (::std::nullptr_t) {
		if (!mElement)
			return *this;

		// Dereference/destroy the previous element								
		const auto meta = MetaData::Of<T>();
		auto entry = Allocator::Find(meta, mElement);
		if (entry->mReferences == 1) {
			delete mElement;
			entry->Deallocate();
		}

		--entry->mReferences;
		return *this;
	}

	/// Implicit cast to a constant pointer												
	TEMPLATE()
	TAny<T>::SparseElement::operator const T() const noexcept {
		return mElement;
	}

	/// Implicit cast to a mutable pointer													
	TEMPLATE()
	TAny<T>::SparseElement::operator T () noexcept {
		return mElement;
	}

	/// Pointer dereferencing (const)														
	TEMPLATE()
	auto TAny<T>::SparseElement::operator -> () const {
		if (!mElement)
			throw Except::Access("Invalid pointer");
		return mElement;
	}

	/// Pointer dereferencing																	
	TEMPLATE()
	auto TAny<T>::SparseElement::operator -> () {
		if (!mElement)
			throw Except::Access("Invalid pointer");
		return mElement;
	}

	/// Pointer dereferencing (const)														
	TEMPLATE()
	decltype(auto) TAny<T>::SparseElement::operator * () const {
		if (!mElement)
			throw Except::Access("Invalid pointer");
		return *mElement;
	}

	/// Pointer dereferencing																	
	TEMPLATE()
	decltype(auto) TAny<T>::SparseElement::operator * () {
		if (!mElement)
			throw Except::Access("Invalid pointer");
		return *mElement;
	}

	/// Call default constructors in a region and initialize memory				
	///	@param count - the number of elements to initialize						
	TEMPLATE()
	void TAny<T>::CallDefaultConstructors(const Count& count) {
		if constexpr (IsNullifiable<T>) {
			// Just zero the memory (optimization)									
			FillMemory(GetRawEnd(), {}, count * GetStride());
			mCount += count;
			return;
		}
		else if constexpr (IsDefaultConstructible<T>) {
			// Construct requested elements in place								
			new (GetRawEnd()) T {}[count];
			mCount += count;
		}
		else LANGULUS_ASSERT("Trying to default-construct elements that are incapable of default-construction");
	}

	/// Get a constant part of this container												
	///	@tparam WRAPPER - the container to use for the part						
	///			            use Block for unreferenced container					
	///	@return a container that represents the cropped part						
	TEMPLATE()
	template<Anyness::IsDeep WRAPPER>
	WRAPPER TAny<T>::Crop(const Offset& start, const Count& count) const {
		auto result = const_cast<TAny*>(this)->Crop<WRAPPER>(start, count);
		result.MakeConstant();
		return Abandon(result);
	}
	
	/// Get a part of this container															
	///	@tparam WRAPPER - the container to use for the part						
	///			            use Block for unreferenced container					
	///	@return a container that represents the cropped part						
	TEMPLATE()
	template<Anyness::IsDeep WRAPPER>
	WRAPPER TAny<T>::Crop(const Offset& start, const Count& count) {
		CheckRange(start, count);
		if (count == 0) {
			WRAPPER result {Disown(*this)};
			result.ResetMemory();
			return Abandon(result);
		}

		WRAPPER result {*this};
		result.MakeStatic();
		result.mCount = result.mReserved = count;
		result.mRaw += start * mType->mSize;
		return Abandon(result);
	}
	
	/// Extend the container and return the new part									
	///	@tparam WRAPPER - the container to use for the extended part			
	///			            use Block for unreferenced container					
	///	@return a container that represents the extended part						
	TEMPLATE()
	template<Anyness::IsDeep WRAPPER>
	WRAPPER TAny<T>::Extend(const Count& count) {
		if (!count || IsStatic())
			// You can not extend static containers								
			return {};

		const auto newCount = mCount + count;
		const auto oldCount = mCount;
		if (newCount <= mReserved) {
			// There is enough available space										
			if constexpr (IsPOD<T>)
				// No need to call constructors for IsPOD items					
				mCount += count;
			else
				CallDefaultConstructors(count);
		}
		else {
			// Allocate more space														
			mEntry = Allocator::Reallocate(mType, newCount, mEntry);
			mRaw = mEntry->GetBlockStart();
			if constexpr (IsPOD<T>) {
				// No need to call constructors for IsPOD items					
				mCount = mReserved = newCount;
			}
			else {
				mReserved = newCount;
				CallDefaultConstructors(count);
			}
		}

		WRAPPER result {*this};
		result.MakeStatic();
		result.mRaw += oldCount;
		result.mCount = result.mReserved = count;
		return Abandon(result);
	}
	
	/// Destructive concatenation																
	TEMPLATE()
	template<class WRAPPER, class RHS>
	TAny<T>& TAny<T>::operator += (const RHS& rhs) {
		if constexpr (Langulus::IsSparse<RHS>)
			return operator += <WRAPPER>(*rhs);
		else if constexpr (IsPOD<T> && Inherits<RHS, TAny>) {
			// Concatenate bytes directly (optimization)							
			const auto count = rhs.GetCount();
			Allocate(mCount + count, false, false);
			CopyMemory(rhs.mRaw, mRaw, count);
			mCount += count;
			return *this;
		}
		else if constexpr (IsConvertible<RHS, WRAPPER>) {
			// Finally, attempt converting											
			return operator += <WRAPPER>(static_cast<WRAPPER>(rhs));
		}
		else LANGULUS_ASSERT("Can't concatenate - RHS is not convertible to WRAPPER");
	}

	/// Concatenate containers																	
	TEMPLATE()
	template<class WRAPPER, class RHS>
	WRAPPER TAny<T>::operator + (const RHS& rhs) const {
		if constexpr (Langulus::IsSparse<RHS>)
			return operator + <WRAPPER>(*rhs);
		else if constexpr (IsPOD<T> && Inherits<RHS, TAny>) {
			// Concatenate bytes															
			WRAPPER result {Disown(*this)};
			result.mCount += rhs.mCount;
			result.mReserved = result.mCount;
			if (result.mCount) {
				result.mEntry = Allocator::Allocate(result.mType, result.mCount);
				result.mRaw = result.mEntry->GetBlockStart();
			}
			else {
				result.mEntry = nullptr;
				result.mRaw = nullptr;
			}

			CopyMemory(mRaw, result.mRaw, mCount);
			CopyMemory(rhs.mRaw, result.mRaw + mCount, rhs.mCount);
			return Abandon(result);
		}
		else if constexpr (IsConvertible<RHS, WRAPPER>) {
			// Attempt converting														
			return operator + <WRAPPER>(static_cast<WRAPPER>(rhs));
		}
		else LANGULUS_ASSERT("Can't concatenate - RHS is not convertible to WRAPPER");
	}

} // namespace Langulus::Anyness

#undef TEMPLATE

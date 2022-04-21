#pragma once
#include "TPointer.hpp"

#define TEMPLATE_OWNED() template<ReflectedData T>
#define TEMPLATE_SHARED() template<ReflectedData T, bool DR>

namespace Langulus::Anyness
{

	/// Initialize with a value																
	///	@param value - value to copy (no referencing shall occur if sparse)	
	TEMPLATE_OWNED()
	constexpr TOwned<T>::TOwned(const T& value) noexcept
		: mValue {value} { }

	/// Move ownership, resetting source value to default								
	///	@param value - value to move														
	TEMPLATE_OWNED()
	constexpr TOwned<T>::TOwned(TOwned&& value) noexcept
		: mValue {value.mValue} {
		value.mValue = {};
	}

	/// Copy a shared pointer																	
	///	@param other - pointer to reference												
	TEMPLATE_SHARED()
	TPointer<T, DR>::TPointer(const TPointer& other)
		: BASE {other} {
		if (BASE::mValue) {
			Allocator::Reference(MetaData::Of<T>(), BASE::mValue, 1);
			if constexpr (DR && Referencable<T>)
				BASE::mValue->Keep();
		}
	}

	/// Move a shared pointer																	
	///	@param other - pointer to move													
	TEMPLATE_SHARED()
	TPointer<T, DR>::TPointer(TPointer&& other) noexcept
		: BASE {Forward<BASE>(other)} { }

	/// Reference a pointer																		
	///	@param ptr - pointer to reference												
	TEMPLATE_SHARED()
	TPointer<T, DR>::TPointer(SparseT ptr)
		: BASE {ptr} {
		if (BASE::mValue) {
			Allocator::Reference(MetaData::Of<T>(), BASE::mValue, 1);
			if constexpr (DR && Referencable<T>)
				BASE::mValue->Keep();
		}
	}

	/// Shared pointer destruction															
	TEMPLATE_SHARED()
	TPointer<T, DR>::~TPointer() {
		Reset();
	}

	/// Create a new instance by moving														
	/// Resulting pointer created that way has exactly one reference				
	///	@param initializer - instance to move											
	///	@return the pointer																	
	TEMPLATE_SHARED()
	TPointer<T, DR> TPointer<T, DR>::Create(Decay<T>&& initializer) requires MoveConstructible<Decay<T>> {
		TPointer<T> pointer;
		pointer.mValue = new Decay<T> {Forward<Decay<T>>(initializer)};
		return pointer;
	}

	/// Create a new instance by copying													
	/// Resulting pointer created that way has exactly one reference				
	///	@param initializer - instance to copy											
	///	@return the pointer																	
	TEMPLATE_SHARED()
	TPointer<T, DR> TPointer<T, DR>::Create(const Decay<T>& initializer) requires CopyConstructible<Decay<T>> {
		TPointer<T> pointer;
		pointer.mValue = new Decay<T> {initializer};
		return pointer;
	}

	/// Create a default new instance														
	/// Resulting pointer created that way has exactly one reference				
	///	@return the pointer																	
	TEMPLATE_SHARED()
	TPointer<T, DR> TPointer<T, DR>::Create() requires DefaultConstructible<Decay<T>> {
		TPointer<T> pointer;
		pointer.mValue = new Decay<T>;
		return pointer;
	}

	/// Create a new instance of T by providing constructor arguments				
	///	@tparam ...ARGS - the deduced arguments										
	///	@param arguments - the arguments													
	///	@return the new instance															
	TEMPLATE_SHARED() template<typename... ARGS>
	TPointer<T, DR> TPointer<T, DR>::New(ARGS&&... arguments) {
		TPointer<T> pointer;
		pointer.mValue = new Decay<T> {Forward<ARGS>(arguments)...};
		return pointer;
	}

	/// Reset the value																			
	TEMPLATE_OWNED()
	void TOwned<T>::Reset() noexcept {
		mValue = {};
	}

	/// Reset the pointer																		
	TEMPLATE_SHARED()
	void TPointer<T, DR>::Reset() {
		if (BASE::mValue) {
			if constexpr (DR && Referencable<T>)
				BASE::mValue->Reference(-1);

			// This will call destructor on the pointer first					
			// and then the data behind it, if references reach zero			
			// It will zero the mValue for us										
			GetBlock().CallDestructors();
		}
	}

	/// Copy a shared pointer																	
	///	@param other - pointer to reference												
	TEMPLATE_SHARED()
	TPointer<T, DR>& TPointer<T, DR>::operator = (const TPointer<T, DR>& other) {
		Reset();
		new (this) TPointer<T, DR> { other };
		return *this;
	}

	/// Move a shared pointer																	
	///	@param other - pointer to move													
	TEMPLATE_SHARED()
	TPointer<T, DR>& TPointer<T, DR>::operator = (TPointer<T, DR>&& other) {
		Reset();
		new (this) TPointer<T, DR> { pcForward<TPointer<T, DR>>(other) };
		return *this;
	}

	/// Reference a raw pointer																
	///	@param ptr - pointer to reference												
	TEMPLATE_SHARED()
	TPointer<T, DR>& TPointer<T, DR>::operator = (SparseT ptr) {
		#ifdef PC_UNOWNED_POINTER_WARNING
			if (ptr && !PCMEMORY.CheckJurisdiction(sMeta, ptr))
				pcLogFuncWarning << "Given an unowned pointer!";
		#endif
		Reset();
		new (this) TPointer<T, DR> { ptr };
		return *this;
	}

	/// Move-assign a value																		
	///	@param value - the new value														
	TEMPLATE_OWNED()
	constexpr TOwned<T>& TOwned<T>::operator = (TOwned&& value) noexcept {
		mValue = value.mValue;
		value.mValue = {};
		return *this;
	}

	/// Overwrite the value																		
	///	@param value - the new value														
	TEMPLATE_OWNED()
	constexpr TOwned<T>& TOwned<T>::operator = (const T& value) noexcept {
		mValue = value;
		return *this;
	}

	/// Attempt to cast any pointer to the contained pointer							
	///	@param ptr - pointer to reference												
	TEMPLATE_SHARED() template<Sparse ANY_POINTER>
	TPointer<T, DR>& TPointer<T, DR>::operator = (ANY_POINTER ptr) {
		static_assert(Constant<T> || !Constant<ANY_POINTER>,
			"Can't assign a constant pointer to a non-constant pointer wrapper");

		#ifdef PC_UNOWNED_POINTER_WARNING
			if (ptr && !PCMEMORY.CheckJurisdiction(sMeta, ptr))
				pcLogFuncWarning << "Given an unowned pointer!";
		#endif
		Reset();
		new (this) TPointer<T, DR> {
			dynamic_cast<Conditional<Constant<ANY_POINTER>, const T*, T*>>(ptr)
		};
		return *this;
	}

	/// Attempt to cast any pointer to the contained pointer							
	///	@param ptr - pointer to reference												
	TEMPLATE_SHARED() template<class ANY_POINTER>
	TPointer<T, DR>& TPointer<T, DR>::operator = (const TPointer<ANY_POINTER, DR>& ptr) {
		static_assert(Constant<T> || !Constant<ANY_POINTER>,
			"Can't assign a constant pointer to a non-constant pointer wrapper");

		Reset();
		new (this) TPointer<T, DR> {
			dynamic_cast<Conditional<Constant<ANY_POINTER>, const T*, T*>>(ptr.Get())
		};
		return *this;
	}

	/// Get the pointer																			
	///	@return the contained pointer														
	TEMPLATE_OWNED()
	decltype(auto) TOwned<T>::Get() const noexcept {
		return mValue;
	}
	
	TEMPLATE_OWNED()
	decltype(auto) TOwned<T>::Get() noexcept {
		return mValue;
	}

	/// Get the hash of the contained type													
	///	@return the hash of the container type											
	TEMPLATE_OWNED()
	Hash TOwned<T>::GetHash() const requires Hashable<T> {
		if (!mValue)
			return {};
		return mValue->GetHash();
	}

	/// Perform a dynamic cast on the pointer												
	///	@tparam D - the desired type to cast to										
	///	@return the result of a dynamic_cast to the specified type				
	TEMPLATE_OWNED() template<class D>
	auto TOwned<T>::As() const noexcept requires Sparse<T> {
		using RESOLVED = Conditional<Constant<T>, const Decay<D>*, Decay<D>*>;
		return dynamic_cast<RESOLVED>(mValue);
	}

	/// Access the pointer																		
	///	@attention does not check if contained pointer is valid					
	///	@return the contained constant raw pointer									
	TEMPLATE_OWNED()
	auto TOwned<T>::operator -> () const requires Sparse<T> {
		if (!mValue)
			throw Except::Access("Invalid pointer");
		return mValue;
	}

	TEMPLATE_OWNED()
	auto TOwned<T>::operator -> () requires Sparse<T> {
		if (!mValue)
			throw Except::Access("Invalid pointer");
		return mValue;
	}

	/// Access the dereferenced pointer (const)											
	///	@attention does not check if contained pointer is valid					
	///	@return the contained constant dereferenced pointer						
	TEMPLATE_OWNED()
	decltype(auto) TOwned<T>::operator * () const requires Sparse<T> {
		if (!mValue)
			throw Except::Access("Invalid pointer");
		return *mValue;
	}

	TEMPLATE_OWNED()
	decltype(auto) TOwned<T>::operator * () requires Sparse<T> {
		if (!mValue)
			throw Except::Access("Invalid pointer");
		return *mValue;
	}

	/// Explicit boolean cast																	
	///	@return true if value differs from default value							
	TEMPLATE_OWNED()
	TOwned<T>::operator bool() const noexcept {
		return mValue != T {};
	}

	/// Cast to a constant pointer, if mutable											
	///	@return the constant equivalent to this pointer								
	TEMPLATE_OWNED()
	TOwned<T>::operator const T&() const noexcept {
		return mValue;
	}

	TEMPLATE_OWNED()
	TOwned<T>::operator T&() noexcept {
		return mValue;
	}

	/// Cast to a constant pointer, if mutable											
	///	@return the constant equivalent to this pointer								
	TEMPLATE_SHARED()
	TPointer<T, DR>::operator TPointer<const T, DR>() const noexcept requires Mutable<T> {
		return TPointer<const T, DR> { BASE::mValue };
	}

	/// Compare pointers for equality														
	///	@param rhs - the right pointer													
	///	@return true if pointers match													
	TEMPLATE_OWNED()
	bool TOwned<T>::operator == (const TOwned<T>& rhs) const noexcept {
		return mValue == rhs.mValue;
	}

	/// Compare pointers for inequality														
	///	@param rhs - the right pointer													
	///	@return true if pointers do not match											
	TEMPLATE_OWNED()
	bool TOwned<T>::operator != (const TOwned<T>& rhs) const noexcept {
		return mValue != rhs.mValue;
	}

	/// Compare pointers for equality														
	///	@param rhs - the right pointer													
	///	@return true if pointers match													
	TEMPLATE_OWNED()
	bool TOwned<T>::operator == (const T& rhs) const noexcept {
		return mValue == rhs;
	}

	/// Compare pointers for inequality														
	///	@param rhs - the right pointer													
	///	@return true if pointers do not match											
	TEMPLATE_OWNED()
	bool TOwned<T>::operator != (const T& rhs) const noexcept {
		return mValue != rhs;
	}

	/// Compare pointer for nullptr															
	///	@param rhs - the right pointer													
	///	@return true if pointers match													
	TEMPLATE_OWNED()
	bool TOwned<T>::operator == (std::nullptr_t) const noexcept {
		return mValue == nullptr;
	}

	/// Compare pointers for not nullptr													
	///	@param rhs - the right pointer													
	///	@return true if pointers do not match											
	TEMPLATE_OWNED()
	bool TOwned<T>::operator != (std::nullptr_t) const noexcept {
		return mValue != nullptr;
	}

	/// Get the block of the contained value												
	/// Can be invoked by the reflected resolver											
	///	@return the value, interfaced via a memory block							
	TEMPLATE_OWNED()
	Block TOwned<T>::GetBlock() const {
		if constexpr (Sparse<T>)
			return {DataState::Constrained + DataState::Sparse, MetaData::Of<T>(), 1, &mValue};
		else
			return {DataState::Constrained, MetaData::Of<T>(), 1, &mValue};
	}

	/// Get the block of the contained pointer											
	/// Can be invoked by the reflected resolver											
	///	@return the pointer, interfaced via a memory block							
	TEMPLATE_SHARED()
	Block TPointer<T, DR>::GetBlock() const {
		return {DataState::Constrained + DataState::Sparse, MetaData::Of<T>(), 1, &BASE::mValue};
	}
	
} // namespace Langulus::Anyness

#undef TEMPLATE_OWNED()
#undef TEMPLATE_SHARED()

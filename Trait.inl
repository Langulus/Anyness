namespace Langulus::Anyness
{

	/// Create a trait from a trait definition											
	template<RTTI::ReflectedTrait TRAIT, RTTI::ReflectedData DATA>
	Trait Trait::From() {
		return Trait(TRAIT::Reflect(), Block::From<DATA>());
	}

	/// Create a trait from a trait definition											
	template<RTTI::ReflectedTrait TRAIT>
	Trait Trait::FromMemory(const Block& memory) {
		return Trait(TRAIT::Reflect(), memory);
	}

	/// Create a trait from a trait definition											
	template<RTTI::ReflectedTrait TRAIT>
	Trait Trait::FromMemory(Block&& memory) {
		return Trait(TRAIT::Reflect(), pcForward<Block>(memory));
	}

	/// Create a trait from a trait definition and data								
	template<RTTI::ReflectedTrait TRAIT, RTTI::ReflectedData DATA>
	Trait Trait::From(const DATA& stuff) {
		return Trait(TRAIT::Reflect(), Any(stuff));
	}

	/// Create a trait from a trait definition by moving data						
	template<RTTI::ReflectedTrait TRAIT, RTTI::ReflectedData DATA>
	Trait Trait::From(DATA&& stuff) {
		return Trait(TRAIT::Reflect(), Any(pcForward<DATA>(stuff)));
	}

	/// Create a trait from a trait definition and data								
	template<RTTI::ReflectedData DATA>
	Trait Trait::From(TMeta meta, const DATA& stuff) {
		return Trait(meta, Any(stuff));
	}

	/// Create a trait from a trait definition and data								
	template<RTTI::ReflectedData DATA>
	Trait Trait::From(TMeta meta, DATA&& stuff) {
		return Trait(meta, Any(pcForward<DATA>(stuff)));
	}

	/// Create a trait from a trait definition and data								
	inline Trait Trait::FromMeta(TMeta tmeta, DMeta dmeta) {
		return Trait(tmeta, Block(DState::Default, dmeta));
	}

	template<RTTI::ReflectedTrait TRAIT>
	bool Trait::TraitIs() const {
		return TraitIs(TRAIT::ID);
	}

	inline bool Trait::operator != (const Trait& other) const noexcept {
		return !(*this == other);
	}

	inline bool Trait::operator != (const TraitID& other) const noexcept {
		return !(*this == other);
	}

	/// Assign by shallow-copying some value different from Trait					
	///	@param value - the value to copy													
	template<RTTI::ReflectedData T>
	Trait& Trait::operator = (const T& value) requires (Trait::NotCustom<T>) {
		Any::operator = (Any {value});
		return *this;
	}

	/// Assign by shallow-copying some value different from Trait					
	///	@param value - the value to copy													
	template<RTTI::ReflectedData T>
	Trait& Trait::operator = (T& value) requires (Trait::NotCustom<T>) {
		Any::operator = (const_cast<const T&>(value));
		return *this;
	}

	/// Assign by moving some value different from Any									
	///	@param value - the value to move													
	template<RTTI::ReflectedData T>
	Trait& Trait::operator = (T&& value) requires (Trait::NotCustom<T>) {
		Any::operator = (Any {pcForward<T>(value)});
		return *this;
	}

} // namespace Langulus::Anyness

#include "inner/Block.hpp"
#include "../Any.hpp"

namespace Langulus::Anyness::Inner
{

	/// Encrypt data																				
	/// Simple XOR encryption via key and PRNG. You can encrypt sparse or		
	/// NON-POD memory easily. Do not dereference data, however. Containers		
	/// should always decrypt data before deallocating it. Can also be used to 
	/// encrypt RAM with live links, too. Make sure you serialize, before		
	/// encoding prior to writing a file. Enable PC_PARANOID to automatically	
	/// nullify and remove encrypted RAM (if that RAM has only a single			
	/// reference of use!)																		
	Count Block::Encrypt(Block& result, const pcu32* keys, Count key_count) const {
		// First compress the data, to avoid repeating bytes					
		auto compressed_size = Compress(result, Compression::Fastest);
		if (0 == compressed_size)
			return 0;

		// Hash the compressed data for validation after decryption			
		const auto hash = result.GetHash();

		// Append the hash to the back of the compressed memory				
		result.Allocate(compressed_size + sizeof(hash));
		pcCopyMemory(&hash, result.At(compressed_size), sizeof(hash));
		compressed_size += sizeof(hash);
		result.mCount = compressed_size;

		// XOR the contents																
		//TODO mix with a PRNG
		for (Count i = 0; i < compressed_size / sizeof(pcu32); ++i)
			static_cast<pcu32*>(result.mRaw)[i] ^= keys[i % key_count];

		// Done																				
		return result.mCount;
	}

	/// Decrypt data																				
	Count Block::Decrypt(Block& result, const pcu32* keys, Count key_count) const {
		// Copy this encrypted data													
		Block decrypted;
		Clone(decrypted);

		// XOR the contents to decrypt them											
		//TODO mix with a PRNG
		for (Count i = 0; i < mCount / sizeof(pcu32); ++i)
			static_cast<pcu32*>(decrypted.mRaw)[i] ^= keys[i % key_count];

		// Get the hash part																
		const auto real_size = mCount - sizeof(Hash);
		const auto hash = *reinterpret_cast<Hash*>(decrypted.At(real_size));
		decrypted.mCount = real_size;

		// Hash the compressed data for validation after decryption			
		const auto decoded_hash = decrypted.GetHash();
		if (hash != decoded_hash) {
			//TODO the hash is small and there may be a collision!

			// Hashes don't match. Decryption failed.								
			// Cleanup and abort, returning zero. If we don't do that		
			// there's undefined behavior coming straight at ya.				
			decrypted.Free();
			return 0;
		}

		// Decompress data																
		const auto decompressed_size = decrypted.Decompress(result);

		// Cleanup and we're done														
		decrypted.Free();
		return decompressed_size;
	}

} // namespace Langulus::Anyness::Inner

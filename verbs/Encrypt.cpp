#include "../Any.hpp"

namespace Langulus::Anyness
{

	constexpr auto HS = sizeof(Hash);
	
	/// Encrypt data																				
	Size Block::Encrypt(Block& result, const Hash* keys, const Count& key_count) const {
		// First compress the data, to avoid repeating bytes					
		#if LANGULUS_FEATURE(ZLIB)
			auto compressed_size = Compress(result, Compression::Fastest);
		#else
			Clone(result);
			auto compressed_size = result.GetSize();
		#endif

		if (0 == compressed_size)
			return 0;

		// Hash the compressed data for validation after decryption			
		const Hash hash = result.GetHash();

		// Append the hash to the back of the compressed memory				
		const auto totalSize = compressed_size + (compressed_size % HS) + HS;
		result.SetType<Byte, true>();
		result.Allocate<false>(totalSize);
		result.mCount = totalSize;
		CopyMemory(&hash, result.At(compressed_size), HS);

		// XOR the contents																
		//TODO mix with a PRNG
		for (Count i = 0; i < compressed_size / HS; ++i)
			reinterpret_cast<Hash*>(result.mRaw)[i] ^= keys[i % key_count];

		// Done																				
		return result.mCount;
	}

	/// Decrypt data																				
	Size Block::Decrypt(Block& result, const Hash* keys, const Count& key_count) const {
		// Copy this encrypted data													
		Block decrypted;
		Clone(decrypted);

		// XOR the contents to decrypt them											
		//TODO mix with a PRNG
		for (Count i = 0; i < mCount / HS; ++i)
			reinterpret_cast<Hash*>(decrypted.mRaw)[i] ^= keys[i % key_count];

		// Get the hash part																
		const auto real_size = mCount - HS;
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

		#if LANGULUS_FEATURE(ZLIB)
			// Decompress data															
			const auto decompressed_size = decrypted.Decompress(result);

			// Cleanup and we're done													
			decrypted.Free();
			return decompressed_size;
		#else
			result = Move(decrypted);
			return result.GetSize();
		#endif
	}

} // namespace Langulus::Anyness

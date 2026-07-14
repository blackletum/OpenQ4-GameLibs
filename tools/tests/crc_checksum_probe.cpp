#include <climits>
#include <cstdint>
#include <cstring>

typedef unsigned char byte;

#define __PRECOMPILED_H__

namespace crc32_impl {
#include "../../src/idlib/hashing/CRC32.cpp"
}

namespace honeyman_impl {
#include "../../src/idlib/hashing/Honeyman.cpp"
}

struct checksumVector_t {
	const char *text;
	unsigned long crc32;
	unsigned long honeyman;
};

int main() {
#if !defined( __linux__ )
#error This checksum probe must be compiled on Linux.
#endif

	static_assert( sizeof( unsigned long ) == 8, "Linux x64/arm64 must use LP64 for this probe" );
	static_assert( sizeof( crc32_impl::crc32Word_t ) == 4, "CRC-32 state must stay 32-bit" );
	static_assert( sizeof( honeyman_impl::honeymanWord_t ) == 4, "Honeyman state must stay 32-bit" );

	const checksumVector_t vectors[] = {
		{ "", 0x00000000UL, 0x00000000UL },
		{ "a", 0xe8b7be43UL, 0x4b600000UL },
		{ "abc", 0x352441c2UL, 0x6f2fed80UL },
		{ "123456789", 0xcbf43926UL, 0x4dbabd04UL },
		{ "message digest", 0x20159d7fUL, 0x77423c07UL },
		{ "The quick brown fox jumps over the lazy dog", 0x414fa339UL, 0x346a4297UL },
	};

	for ( const checksumVector_t &vector : vectors ) {
		const int length = static_cast<int>( std::strlen( vector.text ) );
		if ( crc32_impl::CRC32_BlockChecksum( vector.text, length ) != vector.crc32 ) {
			return 1;
		}
		if ( honeyman_impl::Honeyman_BlockChecksum( vector.text, length ) != vector.honeyman ) {
			return 2;
		}

		unsigned long crc32 = 0;
		crc32_impl::CRC32_InitChecksum( crc32 );
		for ( int i = 0; i < length; ++i ) {
			crc32_impl::CRC32_Update( crc32, static_cast<byte>( vector.text[i] ) );
		}
		crc32_impl::CRC32_FinishChecksum( crc32 );
		if ( crc32 != vector.crc32 ) {
			return 3;
		}

		unsigned long honeyman = 0;
		honeyman_impl::Honeyman_InitChecksum( honeyman );
		for ( int i = 0; i < length; ++i ) {
			honeyman_impl::Honeyman_Update( honeyman, static_cast<byte>( vector.text[i] ) );
		}
		honeyman_impl::Honeyman_FinishChecksum( honeyman );
		if ( honeyman != vector.honeyman ) {
			return 4;
		}
	}

	// Public signatures remain unsigned long for source compatibility. These
	// seeded checks ensure their state still follows Win32's 32-bit truncation.
	unsigned long crc32 = 0xfeedfaceffffffffUL;
	crc32_impl::CRC32_UpdateChecksum( crc32, "abc", 3 );
	crc32_impl::CRC32_FinishChecksum( crc32 );
	if ( crc32 != 0x352441c2UL ) {
		return 5;
	}

	unsigned long honeyman = 0xfeedfaceffffffffUL;
	honeyman_impl::Honeyman_UpdateChecksum( honeyman, "abc", 3 );
	honeyman_impl::Honeyman_FinishChecksum( honeyman );
	if ( honeyman != 0x1f2fe9ffUL ) {
		return 6;
	}

	crc32 = 0xfeedface00000000UL;
	crc32_impl::CRC32_FinishChecksum( crc32 );
	if ( crc32 != 0xffffffffUL ) {
		return 7;
	}

	honeyman = 0xfeedface89abcdefUL;
	honeyman_impl::Honeyman_FinishChecksum( honeyman );
	if ( honeyman != 0x89abcdefUL ) {
		return 8;
	}

	return 0;
}

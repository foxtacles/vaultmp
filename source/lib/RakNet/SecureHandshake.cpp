/// \file
///
/// This file is part of RakNet Copyright 2003 Jenkins Software LLC
///
/// Usage of RakNet is subject to the appropriate license agreement.

#include "NativeFeatureIncludes.h"

#if LIBCAT_SECURITY==1

// If building a RakNet DLL, be sure to tweak the CAT_EXPORT macro meaning
#if !defined(_RAKNET_LIB) && defined(_RAKNET_DLL)
# define CAT_BUILD_DLL
#else
# define CAT_NEUTER_EXPORT
#endif

#include "cat/src/port/EndianNeutral.cpp"
#include "cat/src/port/AlignedAlloc.cpp"
#include "cat/src/time/Clock.cpp"
#include "cat/src/threads/Mutex.cpp"
#include "cat/src/threads/Thread.cpp"
#include "cat/src/threads/WaitableFlag.cpp"
#include "cat/src/hash/MurmurHash2.cpp"
#include "cat/src/lang/Strings.cpp"

#include "cat/src/math/BigRTL.cpp"
#include "cat/src/math/BigPseudoMersenne.cpp"
#include "cat/src/math/BigTwistedEdwards.cpp"

#include "cat/src/crypt/SecureCompare.cpp"
#include "cat/src/crypt/cookie/CookieJar.cpp"
#include "cat/src/crypt/hash/HMAC_MD5.cpp"
#include "cat/src/crypt/privatekey/ChaCha.cpp"
#include "cat/src/crypt/hash/Skein.cpp"
#include "cat/src/crypt/hash/Skein256.cpp"
#include "cat/src/crypt/hash/Skein512.cpp"
#include "cat/src/crypt/pass/Passwords.cpp"

#include "cat/src/crypt/rand/EntropyWindows.cpp"
#include "cat/src/crypt/rand/EntropyLinux.cpp"
#include "cat/src/crypt/rand/EntropyWindowsCE.cpp"
#include "cat/src/crypt/rand/EntropyGeneric.cpp"
#include "cat/src/crypt/rand/Fortuna.cpp"

#include "cat/src/crypt/tunnel/KeyAgreement.cpp"
#include "cat/src/crypt/tunnel/AuthenticatedEncryption.cpp"
#include "cat/src/crypt/tunnel/KeyAgreementInitiator.cpp"
#include "cat/src/crypt/tunnel/KeyAgreementResponder.cpp"
#include "cat/src/crypt/tunnel/KeyMaker.cpp"

#include "cat/src/crypt/tunnel/EasyHandshake.cpp"

#endif // LIBCAT_SECURITY

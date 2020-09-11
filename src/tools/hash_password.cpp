/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <openssl/evp.h>
#include <base/math.h>
#include <base/system.h>

// https://github.com/Anti-weakpasswords/PBKDF2-GCC-OpenSSL-library/blob/master/pbkdf2_openssl.c
void hash_password(const char* pString, const unsigned char* pSalt, int32_t Iterations, char* pHexResult)
{
	unsigned int i;
	unsigned char aDigest[32]; // SHA-256 <= 32
	PKCS5_PBKDF2_HMAC(pString, str_length((char*)pString), pSalt, str_length((char*)pSalt), Iterations, EVP_sha256(), 32, aDigest);
	for (i = 0; i < sizeof(aDigest); i++)
		sprintf(pHexResult + (i * 2), "%02x", 255 & aDigest[i]);
}

void dbg_print(const char* sys, const char* fmt, ...)
{
	va_list args;
	char str[1024 * 4];
	char* msg;
	int i, len;

	str_format(str, sizeof(str), "[%s]: ", sys);
	len = str_length(str);
	msg = (char*)str + len;

	va_start(args, fmt);
#if defined(CONF_FAMILY_WINDOWS) && !defined(__GNUC__)
	_vsprintf_p(msg, sizeof(str) - len, fmt, args);
#else
	vsnprintf(msg, sizeof(str) - len, fmt, args);
#endif
	va_end(args);

	printf("%s\n", str);
}

int main(int argc, char** argv)
{
	dbg_logger_stdout();

	if (secure_random_init() != 0)
	{
		dbg_print("hash_password", "could not initialize secure RNG");
		return -1;
	}

	char aPassword[32] = { 0 };
	secure_random_password(aPassword, sizeof(aPassword), 16);

	while (argc)
	{
		if (str_comp(*argv, "-p") == 0)
		{
			argc--; argv++;
			str_copy(aPassword, *argv, sizeof(aPassword));
		}

		argc--; argv++;
	}

	char aSalt[32] = { 0 };
	secure_random_password(aSalt, sizeof(aSalt), 24);

	char aHash[128] = { 0 };
	hash_password(aPassword, (const unsigned char*)aSalt, 16384, aHash);

	dbg_print("plain", aPassword);
	dbg_print("hash", aHash);
	dbg_print("salt", aSalt);

	return 0;
}
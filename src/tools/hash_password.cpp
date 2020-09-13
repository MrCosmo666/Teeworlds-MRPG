/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <stdio.h>
#include <stdarg.h>

#include <base/hash_ctxt.h>
#include <base/math.h>
#include <base/system.h>

void dbg_print(const char* sys, const char* fmt, ...)
{
	va_list args;
	char str[1024 * 4];
	char* msg;
	int len;

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

	// char aHash[128] = { 0 };
	// hash_password(aPassword, (const unsigned char*)aSalt, 16384, aHash);

	SHA256_CTX Sha256Ctx;
	sha256_init(&Sha256Ctx);

	char aPlaintext[128];
	str_format(aPlaintext, sizeof(aPlaintext), "%s%s%s", aSalt, aPassword, aSalt);

	sha256_update(&Sha256Ctx, aPlaintext, str_length(aPlaintext));
	SHA256_DIGEST Digest = sha256_finish(&Sha256Ctx);

	char aHash[SHA256_MAXSTRSIZE];
	sha256_str(Digest, aHash, sizeof(aHash));

	dbg_print("plain", aPassword);
	dbg_print("hash", aHash);
	dbg_print("salt", aSalt);

	return 0;
}
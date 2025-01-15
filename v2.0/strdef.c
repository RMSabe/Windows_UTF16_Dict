/*
	These are some definitions to ease application development on Windows systems.
	Version 3.0

	config.h is the macro configuration file, where all macro code settings are applied.
	Settings like text format, target system and so on are set in config.h.

	globldef.h is the global definition file. It defines/undefines macros based on the definitions in config.h file.
	It should be the first #include in all subsequent header and source files.

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "strdef.h"

BOOL WINAPI cstr_compare(const TCHAR *str1, const TCHAR *str2)
{
	SIZE_T len1 = 0u;
	SIZE_T len2 = 0u;
	SIZE_T nchar = 0u;

	if(str1 == NULL) return FALSE;
	if(str2 == NULL) return FALSE;

	len1 = (SIZE_T) cstr_getlength(str1);
	len2 = (SIZE_T) cstr_getlength(str2);

	if(len1 != len2) return FALSE;

	for(nchar = 0u; nchar < len1; nchar++) if(str1[nchar] != str2[nchar]) return FALSE;

	return TRUE;
}

BOOL WINAPI cstr_copy(const TCHAR *input_str, TCHAR *output_str, SIZE_T bufferout_length)
{
	SIZE_T input_len = 0u;
	SIZE_T nmax = 0u;
	SIZE_T nchar = 0u;

	if(bufferout_length < 1u) return FALSE;
	if(output_str == NULL) return FALSE;

	ZeroMemory(output_str, bufferout_length);

	if(input_str == NULL) return TRUE;

	input_len = (SIZE_T) cstr_getlength(input_str);

	if(input_len == 0u) return TRUE;

	if(input_len >= bufferout_length) nmax = bufferout_length - 1u;
	else nmax = input_len;

	for(nchar = 0u; nchar < nmax; nchar++) output_str[nchar] = input_str[nchar];

	output_str[nmax] = (TCHAR) '\0';

	return (input_len < bufferout_length);
}

BOOL WINAPI cstr_tolower(TCHAR *str, SIZE_T buffer_length)
{
	SIZE_T nchar = 0u;

	if(str == NULL) return FALSE;
	if(buffer_length == 0u) return FALSE;

	for(nchar = 0u; nchar < buffer_length; nchar++)
	{
		if(str[nchar] == '\0') break;
		if((str[nchar] >= 0x41) && (str[nchar] <= 0x5a)) str[nchar] = (TCHAR) (((UINT) str[nchar]) | 0x20);
	}

	str[buffer_length - 1u] = (TCHAR) '\0';

	return TRUE;
}

BOOL WINAPI cstr_toupper(TCHAR *str, SIZE_T buffer_length)
{
	SIZE_T nchar = 0u;

	if(str == NULL) return FALSE;
	if(buffer_length == 0u) return FALSE;

	for(nchar = 0u; nchar < buffer_length; nchar++)
	{
		if(str[nchar] == '\0') break;
		if((str[nchar] >= 0x61) && (str[nchar] <= 0x7a)) str[nchar] = (TCHAR) (((UINT) str[nchar]) & 0xdf);
	}

	str[buffer_length - 1u] = (TCHAR) '\0';

	return TRUE;
}

SSIZE_T WINAPI cstr_getlength(const TCHAR *str)
{
	SIZE_T len = 0u;

	if(str == NULL) return -1;

	while(str[len] != '\0') len++;

	return (SSIZE_T) len;
}

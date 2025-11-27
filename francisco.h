#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;
typedef signed char      s8;
typedef signed short     s16;
typedef signed int       s32;
typedef signed long long s64;
typedef float  f32;
typedef double f64;

typedef s8  b8;
typedef s32 b32;
typedef s64 b64;

#define true  1
#define false 0

#define clamp(val,min,max) (((val)<(min))?(min):((val)>(max))?(max):(val))

#define set_flags(flags, flag)    ((flags) |= (flag))
#define has_flags(flags, check_flags) (((flags) & (check_flags)) == (check_flags))
#define toggle_flag(flags, flag) ((flags) ^= (flag))

#define local_persist static
#define global        static
#define function      static

#define memory_copy(dst, src, size) memcpy((dst), (src), (size))
#define memory_move(dst, src, size) memmove((dst), (src), (size))
#define memory_set(dst, val, size)  memset((dst), (val), (size))
#define memory_match(a,b,size)     (memcmp((a),(b),(size)) == 0)

#define array_count(array) (sizeof(array)/sizeof(array[0]))

typedef struct String String;
struct String
{
  s64 size; // Does not include null terminator, though cstring contains it
  u8* cstring;
};
#define Sl(s) {sizeof(s)-1, (u8*)(s)}
#define S(s) make_string(s)

function void
print(String s)
{
  printf("Size: %lld, Cstring: %s\n", s.size, s.cstring);
}

function inline String
make_string(const char *s)
{
  String out;
  out.size = (s64)strlen(s);
  out.cstring = (u8*)s;
  return out;
}

function void
print_bits_u8(u8 byte, u8 significant_bits)
{
  clamp(significant_bits, 0, 8);
  u8 buffer[9];
  u8 buffer_cursor = 0;
  for (s32 j = significant_bits - 1; j >= 0; j -= 1)
  {
    u8 bit = '0' + ((byte >> j) & 1);
    buffer[buffer_cursor++] = bit;
  }
  printf("0b%.*s", 8, buffer);
}

function b32
equal(String a, String b)
{
  if (a.size != b.size)
  {
    return false;
  }
  b32 result = memory_match(a.cstring, b.cstring, a.size);
  return result;
}

function String
load_file(String path) /* NOTE(fz): Allocates the cstring in the result String */
{
  HANDLE file = CreateFileA(path.cstring, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if(file == INVALID_HANDLE_VALUE)
  {
    printf("Unable to open file %s\n", path.cstring);
    return (String){0,0};
  }
  String result = {0, 0};
  result.size = GetFileSize(file, 0);
  result.cstring = (u8*)calloc(result.size, 1);
  if(!ReadFile(file, result.cstring, result.size, NULL, NULL))
  {
    printf("Unable to read file %s\n", path.cstring);
    free(result.cstring);
    return (String){0,0};
  }
  CloseHandle(file);
  return result;
}

function void
write_file(String path, u8* buffer, u32 buffer_size)
{
  HANDLE file = CreateFileA(path.cstring, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
  if(file == INVALID_HANDLE_VALUE)
  {
    printf("Unable to open file %s\n", path.cstring);
    return;
  }
  if (!WriteFile(file, buffer, buffer_size, NULL, NULL))
  {
    printf("Unable to write to file %s\n", path.cstring);
  }
  CloseHandle(file);
}

function String
get_exe_path() /* NOTE(fz): Allocates the cstring in the result String */
{
  String result = {0};

  u8 temp_path[MAX_PATH];
  DWORD length = GetModuleFileNameA(NULL, (char*)temp_path, MAX_PATH);

  if (length > 0 && length < MAX_PATH)
  {
    result.size = (u64)length;
    result.cstring = calloc(result.size, 1);
    memory_copy(result.cstring, temp_path, result.size);
  }

  return result;
}

function void
pop_directory(String* path)
{
  for (s64 i = path->size - 1; i > 0; i -= 1)
  {
    if (path->cstring[i] == '\\')
    {
      path->cstring[i] = '\0';
      path->size = i;
      break;
    }
  }
}

function String
join(String a, String b)  /* NOTE(fz): Allocates the cstring in the result String */
{
  String result;
  result.size = a.size + b.size;
  result.cstring = calloc(result.size + 1, sizeof(u8));
  memory_copy(result.cstring, a.cstring, a.size);
  memory_copy(result.cstring+a.size, b.cstring, b.size);
  return result;
}


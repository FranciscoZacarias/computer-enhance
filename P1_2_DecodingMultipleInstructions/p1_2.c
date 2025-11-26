#include "../francisco.h"

typedef struct Op Op;
struct Op
{
  u8 opcode; // All bits don't necessairly represent a the op code. I.e. 6 bits could be opcode and then D and W.
  String name;
};

enum Mod_Table
{
  MOD_MemoryMode_NoDisplacement, // NOTE(fz): Except when R/M is 110, then 16bit displacement
  MOD_MemoryMode_8BitDisplacement,
  MOD_MemoryMode_16BitDisplacement,
  MOD_RegisterMode_NoDisplacement,
};

global Op ops[] = {
  { 0b100010, Sl("mov") },
};

global String reg_table[]      = { Sl("al"), Sl("cl"), Sl("dl"), Sl("bl"), Sl("ah"), Sl("ch"), Sl("dh"), Sl("bh") };
global String reg_table_wide[] = { Sl("ax"), Sl("cx"), Sl("dx"), Sl("bx"), Sl("sp"), Sl("bp"), Sl("si"), Sl("di") };

int
main()
{
  String exe_path = get_exe_path(); // @Leak
  pop_directory(&exe_path); // Pop .exe
  pop_directory(&exe_path); // Pop build

  String decompiled_asm_path = join(exe_path, S("\\listing_0039_decompiled.asm"));
  String decompiled_bin_path = join(exe_path, S("\\listing_0039_decompiled"));
  String original_bin_path   = join(exe_path, S("\\listing_0039_more_movs"));

  String path_l38 = join(exe_path, S("\\listing_0039_more_movs")); // @Leak
  String l38 = load_file(path_l38);
  
  u8* output_buffer = calloc(1024, sizeof(u8));;
  sprintf(output_buffer, "\nbits 16\n");

  u64 byte_count = 0;
  while (byte_count < l38.size)
  {
    u8 first_byte = l38.cstring[byte_count++];

    for (u32 i = 0; i < array_count(ops); i += 1)
    {
      Op op = ops[i];
      u8 opcode = first_byte;

      // Register/Memory to/from Register
      opcode >>= 2;
      if (opcode == op.opcode)
      {
        u8 D = first_byte & 0b10; // 1: REG is destination, 0: REG is NOT destination
        u8 W = first_byte & 0b1;  // 1: Is 16 bits, 0: Is 8 bits
      
        String* table = (W ? reg_table_wide : reg_table);
        String instruction = S("mov");

        u8 second_byte = l38.cstring[byte_count++];
        u8 MOD = (second_byte >> 6) & 0b00000011;
        u8 REG = (second_byte >> 3) & 0b00000111;
        u8 R_M = (second_byte >> 0) & 0b00000111;

        switch (MOD)
        {
          case MOD_MemoryMode_NoDisplacement:
          {
            if (R_M == 0b110)
            {
              printf("Memory mode no displacement with R/M=110 (I.e. but actually with 16bit displacement) not implemented.");
              return;
            }
            
            printf("MOD_MemoryMode_NoDisplacement not implemented.\n");
          }
          break;
          case MOD_MemoryMode_8BitDisplacement:
          {
            printf("MOD_MemoryMode_8BitDisplacement not implemented.\n");
          }
          break;
          case MOD_MemoryMode_16BitDisplacement:
          {
            printf("MOD_MemoryMode_16BitDisplacement not implemented.\n");
          }
          break;
          case MOD_RegisterMode_NoDisplacement:
          {
            String destination =  D ? table[R_M] : table[REG];
            String source      = !D ? table[R_M] : table[REG];

            sprintf(output_buffer, "%s\n%s %s, %s", output_buffer, instruction.cstring, source.cstring, destination.cstring);
          }
          break;
        }
      }
    }
  }

  printf("%s\n\n", output_buffer);
  write_file(decompiled_asm_path, output_buffer, strlen(output_buffer));

  u8 command_buffer[256];
  sprintf(command_buffer, "nasm %s", decompiled_asm_path.cstring);
  printf("Running: %s...\n\n", command_buffer);
  system(command_buffer);

  u8 command_buffer_fc[256];
  sprintf(command_buffer, "fc %s %s", decompiled_bin_path.cstring, original_bin_path.cstring);
  printf("Copy paste to test:\n%s\n", command_buffer);

  return 0;
}

/*
Listing 039 bytes:
0b10001001
0b11011110
0b10001000
0b11000110
0b10110001
0b00001100
0b10110101
0b11110100
0b10111001
0b00001100
0b00000000
0b10111001
0b11110100
0b11111111
0b10111010
0b01101100
0b00001111
0b10111010
0b10010100
0b11110000
0b10001010
0b00000000
0b10001011
0b00011011
0b10001011
0b01010110
0b00000000
0b10001010
0b01100000
0b00000100
0b10001010
0b10000000
0b10000111
0b00010011
0b10001001
0b00001001
0b10001000
0b00001010
0b10001000
0b01101110
0b00000000
*/
#include "../francisco.h"

#define INSTRUCTION_MOV 0b100010

enum Mod_Table
{
  MOD_MemoryMode_NoDisplacement, // NOTE(fz): Except when R/M is 110, then 16bit displacement
  MOD_MemoryMode_8BitDisplacement,
  MOD_MemoryMode_16BitDisplacement,
  MOD_RegisterMode_NoDisplacement,
};

global String reg_table[]      = { Sl("al"), Sl("cl"), Sl("dl"), Sl("bl"), Sl("ah"), Sl("ch"), Sl("dh"), Sl("bh") };
global String reg_table_wide[] = { Sl("ax"), Sl("cx"), Sl("dx"), Sl("bx"), Sl("sp"), Sl("bp"), Sl("si"), Sl("di") };

int
main()
{
  String exe_path = get_exe_path(); // @Leak
  pop_directory(&exe_path); // Pop .exe
  pop_directory(&exe_path); // Pop build

  String decompiled_asm_path = join(exe_path, S("\\listing_0038_decompiled.asm"));
  String decompiled_bin_path = join(exe_path, S("\\listing_0038_decompiled"));
  String original_bin_path   = join(exe_path, S("\\listing_0038_many_register_mov"));

  String path_l38 = join(exe_path, S("\\listing_0038_many_register_mov")); // @Leak
  String l38 = load_file(path_l38);
  
  u8* output_buffer = calloc(1024, sizeof(u8));;
  sprintf(output_buffer, "\nbits 16\n");

  u64 byte_count = 0;
  while (byte_count < l38.size)
  {
    u8 first_byte = l38.cstring[byte_count++];
    u8 opcode = first_byte >> 2;
    u8 D      = first_byte & 0b10; // 1: REG is destination, 0: REG is NOT destination
    u8 W      = first_byte & 0b1;  // 1: Is 16 bits, 0: Is 8 bits

    switch (opcode)
    {
      case INSTRUCTION_MOV: 
      {
        String instruction = S("mov");

        u8 second_byte = l38.cstring[byte_count++];
        u8 MOD = (second_byte >> 6) & 0b00000011;
        u8 REG = (second_byte >> 3) & 0b00000111;
        u8 R_M = (second_byte >> 0) & 0b00000111;

        switch (MOD)
        {
          case MOD_MemoryMode_NoDisplacement:
          {
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
            String* table = (W ? reg_table_wide : reg_table);

            String destination =  D ? table[R_M] : table[REG];
            String source      = !D ? table[R_M] : table[REG];

            sprintf(output_buffer, "%s\n%s %s, %s", output_buffer, instruction.cstring, source.cstring, destination.cstring);
          }
          break;
        }
      }
      break;
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
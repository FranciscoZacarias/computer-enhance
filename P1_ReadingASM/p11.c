#include "../francisco.h"

#define INSTRUCTION_MOV 0b100010

int
main()
{
  String exe_path = get_exe_path(); // @Leak
  pop_directory(&exe_path); // Pop .exe
  pop_directory(&exe_path); // Pop build

  String path_l37 = join(exe_path, S("\\listing_0037_single_register_mov")); // @Leak
  String l37 = load_file(path_l37);

  print_bits_u8(l37.cstring[0]); printf(" ");
  print_bits_u8(l37.cstring[1]);

  u64 byte_count = 0;

  while (byte_count < l37.size)
  {
    u8 current_byte = l37.cstring[byte_count];

    u8 opcode = current_byte >> 2;
    u8 D      = current_byte & 0b10; // 1: REG is destination, 0: REG is NOT destination
    u8 W      = current_byte & 0b1;  // 1: Is 16 bits, 0: Is 8 bits

    switch (opcode)
    {
      case INSTRUCTION_MOV: 
      {
        printf("\nmov inst d: %u w: %u\n", D, W);
      } break;
    }
    byte_count += 1;
  }

  return 0;
}
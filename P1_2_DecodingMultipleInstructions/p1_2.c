#include "../francisco.h"

#define INVALID_OFFSET 255

typedef u8 Data_Transfer_Type;
enum
{
  // MOV = Move
  DataTransfer_RegisterMemory_ToFrom_Register,
  DataTransfer_Immediate_To_RegisterMemory,
  DataTransfer_Immediate_To_Register,
  DataTransfer_Memory_To_Accumulator,
  DataTransfer_Accumulator_To_Memory,
  DataTransfer_RegisterMemory_To_SegmentRegister,
  DataTransfer_SegmentRegister_To_RegisterMemory
};

typedef u8 Mod_Type;
enum
{
  Mod_MemoryMode_NoDisplacement, // NOTE(fz): Except when R/M is 110, then 16bit displacement
  Mod_MemoryMode_8BitDisplacement,
  Mod_MemoryMode_16BitDisplacement,
  Mod_RegisterMode_NoDisplacement,
};

typedef struct Bit_Field Bit_Field;
struct Bit_Field
{
  u8 start_offset; // Offset from the left! If field doesnt exist then INVALID_OFFSET. 
  u8 mask;
  u8 data;
};
#define bit_field(o,m) {.start_offset=(o),.mask=(m), .data=0}
#define bit_field_non_existant() {.start_offset=INVALID_OFFSET,.mask=0, .data=0}

typedef struct Instruction_Encoding Instruction_Encoding;
struct Instruction_Encoding
{
  String name;
  Data_Transfer_Type data_transfer_type;

  u8 instruction; // Bit pattern of the instruction
  u8 opcode_mask; // I think the instruction encoding itself will always be on the first byte? So mask on the low byte should be enough
  u8 data_size; // How many bytes this Instruction necessairly has (could still have more depending on the encoding).
  union
  {
    u16 data; // Data payload when parsing the binary
    struct
    {
      u8 data_low;
      u8 data_high;
    };
  };

  Bit_Field S;
  Bit_Field W; // 1: Is 16 bits;         0: Is 8 bits
  Bit_Field D; // 1: REG is destination; 0: REG is NOT destination
  Bit_Field V;
  Bit_Field Z;

  Bit_Field MOD;
  Bit_Field REG;
  Bit_Field R_M;
};

global String reg_table[]      = { Sl("al"), Sl("cl"), Sl("dl"), Sl("bl"), Sl("ah"), Sl("ch"), Sl("dh"), Sl("bh") };
global String reg_table_wide[] = { Sl("ax"), Sl("cx"), Sl("dx"), Sl("bx"), Sl("sp"), Sl("bp"), Sl("si"), Sl("di") };

global Instruction_Encoding instructions[] = {
  #include "instruction_encodings.inl"
};

#define LISTING_FILE "listing_0039_more_movs"

/*
  There are quite a few string related leaks (leaked functions are marked in francisco.h)
  but since the program as a very short lifetime, I don't really bother freeing them.
*/

int
main()
{
  String exe_path = get_exe_path(); // @Leak
  pop_directory(&exe_path); // Pop .exe
  pop_directory(&exe_path); // Pop build

  String decompiled_asm_path = join(exe_path, S("\\" LISTING_FILE "_decompiled.asm")); // @Leak
  String decompiled_bin_path = join(exe_path, S("\\" LISTING_FILE "_decompiled"));     // @Leak
  String original_bin_path   = join(exe_path, S("\\" LISTING_FILE)); // @Leak
  String compiled_original_listing = load_file(original_bin_path);
  
  u8* output_buffer = calloc(1024, sizeof(u8));;
  sprintf(output_buffer, "\nbits 16\n");

  u64 byte_count = 0;
  while (byte_count < compiled_original_listing.size)
  {
    u8 low_byte  = compiled_original_listing.cstring[byte_count++];
    u8 high_byte = 0;

    // Find correct instruction
    Instruction_Encoding instruction = {0};
    for (u32 i = 0; i < array_count(instructions); i += 1)
    {
      u8 opcode = low_byte & instructions[i].opcode_mask;
      if (opcode == instructions[i].instruction)
      {
        instruction = instructions[i];

        // Fill data
        instruction.data_low = low_byte;
        if (instruction.data_size == 2)
        {
          instruction.data_high = compiled_original_listing.cstring[byte_count++];
        }

        break;
      }
    }
  }

  printf("\n```%s\n```", output_buffer);

#if 0
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

        u8 second_byte = compiled_original_listing   .cstring[byte_count++];
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

  write_file(decompiled_asm_path, output_buffer, strlen(output_buffer));

  u8 command_buffer[256];
  sprintf(command_buffer, "nasm %s", decompiled_asm_path.cstring);
  printf("Running: %s...\n\n", command_buffer);
  system(command_buffer);

  u8 command_buffer_fc[256];
  sprintf(command_buffer, "fc %s %s", decompiled_bin_path.cstring, original_bin_path.cstring);
  printf("Copy paste to test:\n%s\n", command_buffer);
#endif

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
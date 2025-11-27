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
  Mod_MemoryMode_NoDisplacement    = 0b00, // NOTE(fz): Except when R/M is 110, then 16bit displacement
  Mod_MemoryMode_8BitDisplacement  = 0b01,
  Mod_MemoryMode_16BitDisplacement = 0b10,
  Mod_RegisterMode_NoDisplacement  = 0b11,
};

typedef struct Bit_Field Bit_Field;
struct Bit_Field
{
  u8 offset; // If field doesnt exist then INVALID_OFFSET. 
  u8 mask;
  u8 data;
};
#define bit_field(o,m) {.offset=(o),.mask=(m), .data=0}
#define bit_field_non_existant() {.offset=INVALID_OFFSET,.mask=0, .data=0}

typedef struct Instruction_Encoding Instruction_Encoding;
struct Instruction_Encoding
{
  String name;
  Data_Transfer_Type data_transfer_type;

  u8 instruction; // Bit pattern of the instruction
  u8 opcode_mask; // I think the instruction encoding itself will always be on the first byte? So mask on the low byte should be enough
  u8 encoding_size; // How many bytes this Instruction necessairly has (could still have more depending on the encoding).
  union
  {
    u16 encoding; // Data payload when parsing the binary
    struct
    {
      u8 encoding_high;
      u8 encoding_low;
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

global String reg_table[] = {
  Sl("al"),
  Sl("cl"),
  Sl("dl"),
  Sl("bl"),
  Sl("ah"),
  Sl("ch"),
  Sl("dh"),
  Sl("bh")
};
global String reg_table_wide[] = {
  Sl("ax"),
  Sl("cx"),
  Sl("dx"),
  Sl("bx"),
  Sl("sp"),
  Sl("bp"),
  Sl("si"),
  Sl("di")
};
global String effective_address_calc_no_displacement[] = {
  Sl("[bx + si]"),
  Sl("[bx + di]"),
  Sl("[bp + si]"),
  Sl("[bp + di]"),
  Sl("[si]"),
  Sl("[di]"),
  Sl("DIRECT ADDRESS"),
  Sl("[bx]")
};

global String effective_address_calc_with_displacement[] = {
  Sl("[bx + si + %u]"),
  Sl("[bx + di + %u]"),
  Sl("[bp + si + %u]"),
  Sl("[bp + di + %u]"),
  Sl("[si + %u]"),
  Sl("[di + %u]"),
  Sl("[bp + %u]"),
  Sl("[bx + %u]")
};

global Instruction_Encoding instructions[] = {
  #include "instruction_encodings.inl"
};

#define LISTING_FILE "listing_0039_more_movs"

function inline u8
get_bitfields(u16 data, u8 offset, u8 mask)
{
  u8 shift  = data >> offset;
  u8 masked = shift & mask; 
  return masked;
}

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
      u8 instruction_code = instructions[i].instruction & instructions[i].opcode_mask;

      if (opcode == instruction_code)
      {
        instruction = instructions[i];

        // Fill data
        instruction.encoding_low = low_byte;
        if (instruction.encoding_size == 2) instruction.encoding_high = compiled_original_listing.cstring[byte_count++];

        if (instruction.S.offset != INVALID_OFFSET) instruction.S.data = get_bitfields(instruction.encoding, instruction.S.offset, instruction.S.mask);
        if (instruction.W.offset != INVALID_OFFSET) instruction.W.data = get_bitfields(instruction.encoding, instruction.W.offset, instruction.W.mask);
        if (instruction.D.offset != INVALID_OFFSET) instruction.D.data = get_bitfields(instruction.encoding, instruction.D.offset, instruction.D.mask);
        if (instruction.V.offset != INVALID_OFFSET) instruction.V.data = get_bitfields(instruction.encoding, instruction.V.offset, instruction.V.mask);
        if (instruction.Z.offset != INVALID_OFFSET) instruction.Z.data = get_bitfields(instruction.encoding, instruction.Z.offset, instruction.Z.mask);

        if (instruction.MOD.offset != INVALID_OFFSET) instruction.MOD.data = get_bitfields(instruction.encoding, instruction.MOD.offset, instruction.MOD.mask);
        if (instruction.REG.offset != INVALID_OFFSET) instruction.REG.data = get_bitfields(instruction.encoding, instruction.REG.offset, instruction.REG.mask);
        if (instruction.R_M.offset != INVALID_OFFSET) instruction.R_M.data = get_bitfields(instruction.encoding, instruction.R_M.offset, instruction.R_M.mask);

        break;
      }
    }

    switch (instruction.data_transfer_type)
    {
      case DataTransfer_RegisterMemory_ToFrom_Register:
      {
        String* table = (instruction.W.data ? reg_table_wide : reg_table);
        switch (instruction.MOD.data)
        {
          case Mod_MemoryMode_NoDisplacement:
          {
            String destination = !instruction.D.data ? effective_address_calc_no_displacement[instruction.R_M.data] : table[instruction.REG.data];
            String source      =  instruction.D.data ? effective_address_calc_no_displacement[instruction.R_M.data] : table[instruction.REG.data];
            sprintf(output_buffer, "%s\nmov %s, %s", output_buffer, destination.cstring, source.cstring);
          }
          break;
          case Mod_MemoryMode_8BitDisplacement:
          {
            String source;
            String destination;

            u8 temp[16]; // To put the instruction with displacement from format 
            u8 displacement_low = compiled_original_listing.cstring[byte_count++];

            if (instruction.D.data)
            {
              destination = table[instruction.REG.data];
              source      = effective_address_calc_with_displacement[instruction.R_M.data];

              sprintf(temp, source.cstring, displacement_low);
              sprintf(output_buffer, "%s\nmov %s, %s", output_buffer, destination.cstring, temp);
            }
            else
            {
              destination = effective_address_calc_with_displacement[instruction.R_M.data];
              source      = table[instruction.REG.data];
              
              sprintf(temp, destination.cstring, displacement_low);
              sprintf(output_buffer, "%s\nmov %s, %s", output_buffer, temp, source.cstring);
            }

          }
          break;
          case Mod_MemoryMode_16BitDisplacement:
          {
            String destination = !instruction.D.data ? effective_address_calc_with_displacement[instruction.R_M.data] : table[instruction.REG.data];
            String source      =  instruction.D.data ? effective_address_calc_with_displacement[instruction.R_M.data] : table[instruction.REG.data];

            u8 displacement_low  = compiled_original_listing.cstring[byte_count++];
            u8 displacement_high = compiled_original_listing.cstring[byte_count++];

            u16 displacement = displacement_low;
            displacement |= ((u16)displacement_high << 8);

            u8 temp[16];
            sprintf(temp, source.cstring, displacement);
            sprintf(output_buffer, "%s\nmov %s, %s", output_buffer, destination.cstring, temp);
          }
          break;
          case Mod_RegisterMode_NoDisplacement:
          {
            String destination = !instruction.D.data ? table[instruction.R_M.data] : table[instruction.REG.data];
            String source      =  instruction.D.data ? table[instruction.R_M.data] : table[instruction.REG.data];
            sprintf(output_buffer, "%s\nmov %s, %s", output_buffer, destination.cstring, source.cstring);
          }
          break;
        }
      }
      break;
      case DataTransfer_Immediate_To_RegisterMemory:
      {
        printf("DataTransfer_Immediate_To_RegisterMemory not implemented.\n");
        goto end;
      }
      break;
      case DataTransfer_Immediate_To_Register:
      {
        String* table = (instruction.W.data ? reg_table_wide : reg_table);
        u16 data = compiled_original_listing.cstring[byte_count++];

        if (instruction.W.data)
        {
          u8 data_high = compiled_original_listing.cstring[byte_count++];
          data |= ((u16)data_high << 8);
        }

        String destination = table[instruction.REG.data];
        sprintf(output_buffer, "%s\nmov %s, %u", output_buffer, destination.cstring, data);
      }
      break;
      case DataTransfer_Memory_To_Accumulator:
      {
        printf("DataTransfer_Memory_To_Accumulator not implemented.\n");
        goto end;
      }
      break;
      case DataTransfer_Accumulator_To_Memory:
      {
        printf("DataTransfer_Accumulator_To_Memory not implemented.\n");
        goto end;
      }
      break;
      case DataTransfer_RegisterMemory_To_SegmentRegister:
      {
        printf("DataTransfer_RegisterMemory_To_SegmentRegister not implemented.\n");
        goto end;
      }
      break;
      case DataTransfer_SegmentRegister_To_RegisterMemory:
      {
        printf("DataTransfer_SegmentRegister_To_RegisterMemory not implemented.\n");
        goto end;
      }
      break;
    }
  }

end:
  printf("\n\n```%s\n```\n\n", output_buffer);

  write_file(decompiled_asm_path, output_buffer, strlen(output_buffer));

  u8 command_buffer[256];
  sprintf(command_buffer, "nasm %s", decompiled_asm_path.cstring);
  printf("Running: %s...\n\n", command_buffer);
  system(command_buffer);

  u8 command_buffer_fc[256];
  sprintf(command_buffer_fc, "fc %s %s", decompiled_bin_path.cstring, original_bin_path.cstring);
  system(command_buffer_fc);

  return 0;
}
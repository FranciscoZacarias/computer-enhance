#include "../francisco.h"

#define INVALID_OFFSET 255
#define DIRECT_ADDRESS 0b110

typedef u8 Data_Transfer_Type;
enum
{
  DataTransfer_None = 0,

  // Mov
  DataTransfer_MOV_RegisterMemory_ToFrom_Register,
  DataTransfer_MOV_Immediate_To_RegisterMemory,
  DataTransfer_MOV_Immediate_To_Register,
  DataTransfer_MOV_Memory_To_Accumulator,
  DataTransfer_MOV_Accumulator_To_Memory,
  DataTransfer_MOV_RegisterMemory_To_SegmentRegister,
  DataTransfer_MOV_SegmentRegister_To_RegisterMemory,

  // Add
  DataTransfer_ADD_RegisterMemory_With_Register_To_Either,
  DataTransfer_ADD_Immediate_To_RegisterMemory,
  DataTransfer_ADD_Immediate_To_Accumulator,
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
  Sl("[%d]"),
  Sl("[bx]")
};

global String effective_address_calc_with_displacement[] = {
  Sl("[bx + si + %d]"),
  Sl("[bx + di + %d]"),
  Sl("[bp + si + %d]"),
  Sl("[bp + di + %d]"),
  Sl("[si + %d]"),
  Sl("[di + %d]"),
  Sl("[bp + %d]"),
  Sl("[bx + %d]")
};

global Instruction_Encoding instructions[] = {
  #include "instruction_encodings.inl"
};

#define LISTING_FILE "listing_0041_add_sub_cmp_jnz"

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
  String original_bin_path   = join(exe_path, S("\\" LISTING_FILE));                   // @Leak
  String compiled_original_listing = load_file(original_bin_path);                     // @Leak
  
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

    if (instruction.data_transfer_type == DataTransfer_None)
    {
      printf("Instruction: "); 
      print_bits_u8(low_byte, 8);
      printf(" not implemented.\n");
      goto end;
    }

    switch (instruction.data_transfer_type)
    {
      // MOV
      case DataTransfer_MOV_RegisterMemory_ToFrom_Register:
      {
        String* table = (instruction.W.data ? reg_table_wide : reg_table);
        switch (instruction.MOD.data)
        {
          case Mod_MemoryMode_NoDisplacement:
          {
            String effective_address = effective_address_calc_no_displacement[instruction.R_M.data];

            if (instruction.R_M.data == DIRECT_ADDRESS)
            {
              String destination = !instruction.D.data ? effective_address : table[instruction.REG.data];

              u8 explicit_size[16];
              if (!instruction.W.data)
              {
                s8 data = (s8)compiled_original_listing.cstring[byte_count++];
                sprintf(explicit_size, "[%d]", data);
              }
              else
              {
                u8 data_low  = compiled_original_listing.cstring[byte_count++];
                u8 data_high = compiled_original_listing.cstring[byte_count++];
                u16 unsigned_data = data_low;
                unsigned_data |= ((u16)data_high << 8);
                s16 data = (s16)unsigned_data;

                sprintf(explicit_size, "[%d]", data);
              }

              //String source      =  instruction.D.data ? effective_address : table[instruction.REG.data];
              sprintf(output_buffer, "%s\nmov %s, %s", output_buffer, destination.cstring, explicit_size);
            }
            else
            {
              String destination = !instruction.D.data ? effective_address : table[instruction.REG.data];
              String source      =  instruction.D.data ? effective_address : table[instruction.REG.data];
              sprintf(output_buffer, "%s\nmov %s, %s", output_buffer, destination.cstring, source.cstring);
            }

          }
          break;
          case Mod_MemoryMode_8BitDisplacement:
          {
            String source;
            String destination;

            u8 temp[16]; // To put the instruction with displacement from format 
            s8 displacement_low = (s8)compiled_original_listing.cstring[byte_count++];
            s16 displacement = (s16)displacement_low; // 8086 Manual 4-20: If the displacement is only a single byte, the 8086 or 8088 automatically sign-extends this quantity to 16-bits

            if (instruction.D.data)
            {
              destination = table[instruction.REG.data];
              source      = effective_address_calc_with_displacement[instruction.R_M.data];
              sprintf(temp, source.cstring, displacement);
              sprintf(output_buffer, "%s\nmov %s, %s", output_buffer, destination.cstring, temp);
            }
            else
            {
              destination = effective_address_calc_with_displacement[instruction.R_M.data];
              source      = table[instruction.REG.data];
              sprintf(temp, destination.cstring, displacement);
              sprintf(output_buffer, "%s\nmov %s, %s", output_buffer, temp, source.cstring);
            }
          }
          break;
          case Mod_MemoryMode_16BitDisplacement:
          {
            String source;
            String destination;

            u8 temp[16]; // To put the instruction with displacement from format 

            u8 displacement_low  = compiled_original_listing.cstring[byte_count++];
            u8 displacement_high = compiled_original_listing.cstring[byte_count++];
            u16 unsigned_displacement = displacement_low;
            unsigned_displacement |= ((u16)displacement_high << 8);

            s16 displacement = (s16)unsigned_displacement;

            if (instruction.D.data)
            {
              destination = table[instruction.REG.data];
              source      = effective_address_calc_with_displacement[instruction.R_M.data];
              sprintf(temp, source.cstring, displacement);
              sprintf(output_buffer, "%s\nmov %s, %s", output_buffer, destination.cstring, temp);
            }
            else
            {
              destination = effective_address_calc_with_displacement[instruction.R_M.data];
              source      = table[instruction.REG.data];
              sprintf(temp, destination.cstring, displacement);
              sprintf(output_buffer, "%s\nmov %s, %s", output_buffer, temp, source.cstring);
            }
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
      case DataTransfer_MOV_Immediate_To_RegisterMemory:
      {
        String* table = (instruction.W.data ? reg_table_wide : reg_table);

        switch (instruction.MOD.data)
        {
          case Mod_MemoryMode_NoDisplacement:
          {
            String destination = instruction.D.data ? table[instruction.REG.data] : effective_address_calc_no_displacement[instruction.R_M.data];
            String source      = S("123");

            u8 explicit_size[16];
            if (!instruction.W.data)
            {
              s8 data = (s8)compiled_original_listing.cstring[byte_count++];
              sprintf(explicit_size, "byte %d", data);
            }
            else
            {
              s16 data = (s16)compiled_original_listing.cstring[byte_count++];
              sprintf(explicit_size, "word %d", data);
            }

            sprintf(output_buffer, "%s\nmov %s, %s", output_buffer, destination.cstring, explicit_size);
          }
          break;
          case Mod_MemoryMode_8BitDisplacement:
          {
            printf("Mod_MemoryMode_8BitDisplacement not implemented.\n");
            goto end;
          }
          break;
          case Mod_MemoryMode_16BitDisplacement:
          {
            String source;
            String destination;

            u8 temp[16]; // To put the instruction with displacement from format 

            u8 displacement_low  = compiled_original_listing.cstring[byte_count++];
            u8 displacement_high = compiled_original_listing.cstring[byte_count++];
            u16 unsigned_displacement = displacement_low;
            unsigned_displacement |= ((u16)displacement_high << 8);
            s16 displacement = (s16)unsigned_displacement;

            u8 explicit_size[16];
            if (!instruction.W.data)
            {
              s8 data = (s8)compiled_original_listing.cstring[byte_count++];
              sprintf(explicit_size, "byte %d", data);
            }
            else
            {
              u8 data_low  = compiled_original_listing.cstring[byte_count++];
              u8 data_high = compiled_original_listing.cstring[byte_count++];
              u16 unsigned_data = data_low;
              unsigned_data |= ((u16)data_high << 8);
              s16 data = (s16)unsigned_data;

              sprintf(explicit_size, "word %d", data);
            }

            if (instruction.D.data)
            {
              destination = table[instruction.REG.data];
              source      = effective_address_calc_with_displacement[instruction.R_M.data];
              sprintf(temp, source.cstring, displacement);
              sprintf(output_buffer, "%s\nmov %s, %s", output_buffer, destination.cstring, explicit_size);
            }
            else
            {
              destination = effective_address_calc_with_displacement[instruction.R_M.data];
              source      = table[instruction.REG.data];
              sprintf(temp, destination.cstring, displacement);
              sprintf(output_buffer, "%s\nmov %s, %s", output_buffer, temp, explicit_size);
            }
          }
          break;
          case Mod_RegisterMode_NoDisplacement:
          {
            printf("Mod_RegisterMode_NoDisplacement not implemented.\n");
            goto end;
          }
          break;
        }
      }
      break;
      case DataTransfer_MOV_Immediate_To_Register:
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
      case DataTransfer_MOV_Memory_To_Accumulator:
      {
        String* table = (instruction.W.data ? reg_table_wide : reg_table);
        String destination = !instruction.D.data ? table[instruction.R_M.data] : table[instruction.REG.data];

        u8 memory[16];
        if (!instruction.W.data)
        {
          s8 data = (s8)compiled_original_listing.cstring[byte_count++];
          sprintf(memory, "[%d]", data);
        }
        else
        {
          u8 data_low  = compiled_original_listing.cstring[byte_count++];
          u8 data_high = compiled_original_listing.cstring[byte_count++];
          u16 unsigned_data = data_low;
          unsigned_data |= ((u16)data_high << 8);
          s16 data = (s16)unsigned_data;
          sprintf(memory, "[%d]", data);
        }

        sprintf(output_buffer, "%s\nmov %s, %s", output_buffer, destination.cstring, memory);
      }
      break;
      case DataTransfer_MOV_Accumulator_To_Memory:
      {
        String* table = (instruction.W.data ? reg_table_wide : reg_table);
        String destination = !instruction.D.data ? table[instruction.R_M.data] : table[instruction.REG.data];

        u8 memory[16];
        if (!instruction.W.data)
        {
          s8 data = (s8)compiled_original_listing.cstring[byte_count++];
          sprintf(memory, "[%d]", data);
        }
        else
        {
          u8 data_low  = compiled_original_listing.cstring[byte_count++];
          u8 data_high = compiled_original_listing.cstring[byte_count++];
          u16 unsigned_data = data_low;
          unsigned_data |= ((u16)data_high << 8);
          s16 data = (s16)unsigned_data;
          sprintf(memory, "[%d]", data);
        }

        sprintf(output_buffer, "%s\nmov %s, %s", output_buffer, memory, destination.cstring);
      }
      break;
      case DataTransfer_MOV_RegisterMemory_To_SegmentRegister:
      {
        printf("DataTransfer_MOV_RegisterMemory_To_SegmentRegister not implemented.\n");
        goto end;
      }
      break;
      case DataTransfer_MOV_SegmentRegister_To_RegisterMemory:
      {
        printf("DataTransfer_MOV_SegmentRegister_To_RegisterMemory not implemented.\n");
        goto end;
      }
      break;

      // ADD
      case DataTransfer_ADD_RegisterMemory_With_Register_To_Either:
      {
        printf("DataTransfer_ADD_RegisterMemory_With_Register_To_Either not implemented.\n");
        goto end;
      }
      break;
      case DataTransfer_ADD_Immediate_To_RegisterMemory:
      {
        printf("DataTransfer_ADD_Immediate_To_RegisterMemory not implemented.\n");
        goto end;
      }
      break;
      case DataTransfer_ADD_Immediate_To_Accumulator:
      {
        printf("DataTransfer_ADD_Immediate_To_Accumulator not implemented.\n");
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
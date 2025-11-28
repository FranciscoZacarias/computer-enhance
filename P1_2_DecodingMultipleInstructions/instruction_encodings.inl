

// MOV
// ---
{
  // Register/memory to/from register 
  .name = Sl("mov"),
  .data_transfer_type = DataTransfer_RegisterMemory_ToFrom_Register,
  .instruction        = 0b100010'00,
  .opcode_mask        = 0b111111'00,
  .encoding_size      = 2,
  .encoding           = 0,
  .S   = bit_field_non_existant(),
  .W   = bit_field(8,0b1),
  .D   = bit_field(9,0b1),
  .V   = bit_field_non_existant(),
  .Z   = bit_field_non_existant(),
  .MOD = bit_field(6,0b11),
  .REG = bit_field(3,0b111),
  .R_M = bit_field(0,0b111)
},

{
  // Immediate to register/memory
  .name = Sl("mov"),
  .data_transfer_type = DataTransfer_Immediate_To_RegisterMemory,
  .instruction        = 0b1100011'0,
  .opcode_mask        = 0b1111111'0,
  .encoding_size      = 2,
  .encoding           = 0,
  .S   = bit_field_non_existant(),
  .W   = bit_field(8,0b1),
  .D   = bit_field_non_existant(),
  .V   = bit_field_non_existant(),
  .Z   = bit_field_non_existant(),
  .MOD = bit_field(6,0b11),
  .REG = bit_field_non_existant(),
  .R_M = bit_field(0,0b111)
},

{
  // Immediate to register
  .name = Sl("mov"),
  .data_transfer_type = DataTransfer_Immediate_To_Register,
  .instruction        = 0b1011'0000,
  .opcode_mask        = 0b1111'0000,
  .encoding_size      = 1,
  .encoding           = 0,
  .S   = bit_field_non_existant(),
  .W   = bit_field(11,0b1),
  .D   = bit_field_non_existant(),
  .V   = bit_field_non_existant(),
  .Z   = bit_field_non_existant(),
  .MOD = bit_field_non_existant(),
  .REG = bit_field(8,0b111),
  .R_M = bit_field_non_existant()
},

{
  // Memory to Accumulator
  .name = Sl("mov"),
  .data_transfer_type = DataTransfer_Memory_To_Accumulator,
  .instruction        = 0b1010000'0,
  .opcode_mask        = 0b1111111'0,
  .encoding_size      = 1,
  .encoding           = 0,
  .S   = bit_field_non_existant(),
  .W   = bit_field(8,0b1),
  .D   = bit_field_non_existant(),
  .V   = bit_field_non_existant(),
  .Z   = bit_field_non_existant(),
  .MOD = bit_field_non_existant(),
  .REG = bit_field_non_existant(),
  .R_M = bit_field_non_existant()
},

{
  // Accumulator to memory
  .name = Sl("mov"),
  .data_transfer_type = DataTransfer_Accumulator_To_Memory,
  .instruction        = 0b1010001'0,
  .opcode_mask        = 0b1111111'0,
  .encoding_size      = 1,
  .encoding           = 0,
  .S   = bit_field_non_existant(),
  .W   = bit_field(8,0b1),
  .D   = bit_field_non_existant(),
  .V   = bit_field_non_existant(),
  .Z   = bit_field_non_existant(),
  .MOD = bit_field_non_existant(),
  .REG = bit_field_non_existant(),
  .R_M = bit_field_non_existant()
},

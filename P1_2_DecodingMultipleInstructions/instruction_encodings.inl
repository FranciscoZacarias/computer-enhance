

// MOV
// ---

// Register/memory to/from register 
{
  .name=Sl("mov"),
  .data_transfer_type=DataTransfer_RegisterMemory_ToFrom_Register,
  .instruction=0b100010'00,
  .opcode_mask=0b111111'00,
  .data_size=2,
  .data=0,
  .S=bit_field_non_existant(),
  .W=bit_field(6,0b1),
  .D=bit_field(7,0b1),
  .V=bit_field_non_existant(),
  .Z=bit_field_non_existant(),
  .MOD=bit_field(8, 0b11),
  .REG=bit_field(10,0b111),
  .R_M=bit_field(13,0b111)
},

// 0000: 01 7b f2 9d 33 46 0c 15  c0 53 2a 03
set_byte_mode();

// f2 9d 33 46 0c 15  c0
read_header_bytes(); // Sets title as "Interplay"
call 0x032A

// 032A: 09 07 86 12 98 1a
// 0330: 43 01 00 11 96 11 41 01
op_09(0x07) // load word_3AE2 with 7
load_resource_section(word_3AE2); // store resource index into word_3AE2
op_12(0x98) // set game state 0x98 with word_3AE2 (resource index)
op_1A(0x43, 0x01) // set game_state 0x43 with 0x01
set_word_mode();
op_11(0x96) // set game_state 0x96/0x97 (word mode) with 0x0000.
op_11(0x41) // set game_state 0x41/0x41 with 0x0000
set_byte_mode();


// Initialize characters single byte at a time in loop of 255.
// 0338: 06 FF 0F 96 17 41 49 3A
// 0340: 03
int counter = 0xFF; // typically stored in word_3AE4
do {
  do {
    // 0x33A

    // Transfer point is through word_3AE2
    op_0F(0x96); // loads word_3AE2
    store_data_into_resource(0x41) // stores word_3AE2
    counter--;
  } while(counter > 0);

  // 0341: 23 42 23 97 0a 42 3e 0e 41 3a 03 0a 56 99 44
  increment_memory(0x42);
  increment_memory(0x97);
  load_word3AE2_gamestate(0x42);
} while (word_3AE2 < 0xE);

// 034C: 0a 56 99 44
op_0A(0x56)

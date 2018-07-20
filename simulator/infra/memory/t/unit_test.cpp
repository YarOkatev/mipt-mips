// generic C
#include <cassert>
#include <cstdlib>

// Catch2
#include <catch.hpp>

// uArchSim modules
#include "../memory.h"

static const std::string valid_elf_file = TEST_DATA_PATH "mips_bin_exmpl.out";

//
// Check that all incorect input params of the constructor
// are properly handled.
//
TEST_CASE( "Func_memory_init: Process_Wrong_Args_Of_Constr")
{
    // check memory initialization with default parameters
    CHECK_NOTHROW( FuncMemory( ));
    // check memory initialization with custom parameters
    CHECK_NOTHROW( FuncMemory( 48, 15, 10));
    // check memory initialization with 4GB page
    CHECK_THROWS_AS( FuncMemory( 64, 15, 32), FuncMemoryBadMapping);
}

TEST_CASE( "Func_memory_init: Process_Correct_ElfInit")
{
    CHECK_NOTHROW( FuncMemory( ).load_elf_file( valid_elf_file));
    CHECK_NOTHROW( FuncMemory( 48, 15, 10).load_elf_file( valid_elf_file));
    
    // test behavior when the file name does not exist
    const std::string wrong_file_name = "./1234567890/qwertyuiop";
    // must exit and return EXIT_FAILURE
    CHECK_THROWS_AS( FuncMemory( ).load_elf_file( wrong_file_name), InvalidElfFile);
}

TEST_CASE( "Func_memory: StartPC_Method_Test")
{
    FuncMemory func_mem;
    func_mem.load_elf_file( valid_elf_file);

    CHECK( func_mem.startPC() == 0x4000b0u /*address of the ".text" section*/);
}

TEST_CASE( "Func_memory: Read_Method_Test")
{
    FuncMemory func_mem;
    func_mem.load_elf_file( valid_elf_file);

    // the address of the ".data" section
    uint64 dataSectAddr = 0x4100c0;

    // read 4 bytes from the func_mem start addr
    uint64 right_ret = 0x03020100;
    CHECK( func_mem.read<uint32>( dataSectAddr) == right_ret);

    // read 3 bytes from the func_mem start addr + 1
    right_ret = 0x030201;
    CHECK( func_mem.read<uint32>( dataSectAddr + 1, 0xFFFFFFull) == right_ret);

    // read 2 bytes from the func_mem start addr + 2
    right_ret = 0x0302;
    CHECK( func_mem.read<uint32>( dataSectAddr + 2, 0xFFFFull) == right_ret);
    CHECK( func_mem.read<uint16>( dataSectAddr + 2) == right_ret);

    // read 1 bytes from the func_mem start addr + 3
    right_ret = 0x03;
    CHECK( func_mem.read<uint8>( dataSectAddr + 3) == right_ret);

    // check hadling the situation when read
    // from not initialized or written data
    CHECK( func_mem.read<uint8>( 0x300000) == NO_VAL8);
}

TEST_CASE( "Func_memory: Write_Read_Initialized_Mem_Test")
{
    FuncMemory func_mem;
    func_mem.load_elf_file( valid_elf_file);

    // the address of the ".data" func_memion
    uint64 data_sect_addr = 0x4100c0;

    // write 1 into the byte pointed by data_sect_addr
    func_mem.write<uint8>( 1, data_sect_addr);
    uint64 right_ret = 0x03020101; // before write it was 0x03020100
    CHECK( func_mem.read<uint32>( data_sect_addr) == right_ret);

    // write 0x7777 into the two bytes pointed by ( data_sect_addr + 1)
    func_mem.write<uint16>( 0x7777, data_sect_addr + 1);
    right_ret = 0x03777701; // before write it was 0x03020101
    CHECK( func_mem.read<uint32>( data_sect_addr) == right_ret);

    // write 0x00000000 into the four bytes pointed by data_sect_addr
    func_mem.write<uint32>( 0x00000000, data_sect_addr, 0xFFFFFFFFull);
    right_ret = 0x00000000; // before write it was 0x03777701
    CHECK( func_mem.read<uint32>( data_sect_addr) == right_ret);
}

TEST_CASE( "Func_memory: Write_Read_Not_Initialized_Mem_Test")
{
    FuncMemory func_mem;
    func_mem.load_elf_file( valid_elf_file);

    uint64 write_addr = 0x3FFFFE;

    // write 0x03020100 into the four bytes pointed by write_addr
    func_mem.write<uint32>( 0x03020100, write_addr);
    uint64 right_ret = 0x0100;
    CHECK( func_mem.read<uint16>( write_addr) == right_ret);

    right_ret = 0x0201;
    CHECK( func_mem.read<uint16>( write_addr + 1) == right_ret);

    right_ret = 0x0302;
    CHECK( func_mem.read<uint16>( write_addr + 2) == right_ret);
}


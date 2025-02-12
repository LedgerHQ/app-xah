cmake_minimum_required(VERSION 3.10)
project(test_parser C)

include(CTest)

set(CMAKE_C_STANDARD 11)

# required for fuzzing
set(CMAKE_C_COMPILER /usr/bin/clang)

enable_testing()

include_directories(
  include
  ../src
  ../src/xah
  ../src/apdu/messages
)

add_library(xah
  ../src/xah/amount.c
  ../src/xah/amount.h
  ../src/xah/array.h
  ../src/xah/fields.c
  ../src/xah/fields.h
  ../src/xah/field_sort.c
  ../src/xah/field_sort.h
  ../src/xah/flags.c
  ../src/xah/flags.h
  ../src/xah/fmt.c
  ../src/xah/fmt.h
  ../src/xah/general.c
  ../src/xah/general.h
  ../src/xah/number_helpers.c
  ../src/xah/number_helpers.h
  ../src/xah/percentage.c
  ../src/xah/percentage.h
  ../src/xah/readers.c
  ../src/xah/readers.h
  ../src/xah/ascii_strings.c
  ../src/xah/ascii_strings.h
  ../src/xah/time.c
  ../src/xah/time.h
  ../src/xah/transaction_types.h
  ../src/xah/xah_helpers.c
  ../src/xah/xah_helpers.h
  ../src/xah/xah_parse.c
  ../src/xah/xah_parse.h
)

add_executable(test_printers
  src/test_printers.c
  src/cx.c
  include/bolos_target.h
  include/cx.h
  include/os.h
)

add_executable(test_swap
  src/test_swap.c
  src/cx.c
  ../src/swap/handle_check_address.h
  ../src/swap/swap_utils.c
  ../src/swap/swap_utils.h
  include/bolos_target.h
  include/cx.h
  include/os.h
  )

add_executable(test_tx
  src/test_tx.c
  src/cx.c
  include/bolos_target.h
  include/cx.h
  include/os.h
)

add_executable(fuzz_tx
  src/fuzz_tx.c
  src/cx.c
  include/bolos_target.h
  include/cx.h
  include/os.h
)

target_compile_options(fuzz_tx PRIVATE -Wall -fsanitize=fuzzer,address -g -ggdb2)
target_link_libraries(fuzz_tx PRIVATE cmocka crypto ssl xah -fsanitize=fuzzer,address)
target_link_libraries(test_printers PRIVATE cmocka crypto ssl xah)
target_link_libraries(test_swap PRIVATE cmocka crypto ssl xah)
target_link_libraries(test_tx PRIVATE cmocka crypto ssl xah)

add_test(test_printers test_printers)
add_test(test_swap test_swap)
add_test(test_tx test_tx)

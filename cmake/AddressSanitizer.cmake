add_library(AddressSanitizer INTERFACE)
target_compile_options(AddressSanitizer INTERFACE -fsanitize=address -fno-omit-frame-pointer)
target_link_libraries(AddressSanitizer INTERFACE asan)

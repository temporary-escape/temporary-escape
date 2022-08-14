# Temporary Escape Game

[![build](https://github.com/temporary-escape/temporary-escape/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/temporary-escape/temporary-escape/actions/workflows/build.yml)

Website: **[https://temporary-escape.github.io/](https://temporary-escape.github.io/)**

```
ASAN_SYMBOLIZER_PATH=/usr/bin/llvm-symbolizer;
LD_LIBRARY_PATH=/opt/vulkan-sdk/1.3.216.0/x86_64/lib;
LSAN_OPTIONS=suppressions=/home/mnovak/Projects/temporary-escape/src/supp:fast_unwind_on_malloc=0;
VK_LAYER_PATH=/opt/vulkan-sdk/1.3.216.0/x86_64/etc/vulkan/explicit_layer.d;VK_LOADER_DEBUG=all;
VULKAN_SDK=/opt/vulkan-sdk/1.3.216.0/x86_64;VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_api_dump:
VK_LAYER_KHRONOS_validation;PATH=/opt/vulkan-sdk/1.3.216.0/x86_64/bin
```

```
-DTEMPORARY_ESCAPE_LLVM_SYMBOLIZER_PATH=/usr/bin/llvm-symbolizer
```
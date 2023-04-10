# Temporary Escape Game

[![build](https://github.com/temporary-escape/temporary-escape/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/temporary-escape/temporary-escape/actions/workflows/build.yml)

Website: **[https://temporary-escape.github.io/](https://temporary-escape.github.io/)**

```
ASAN_SYMBOLIZER_PATH=/usr/bin/llvm-symbolizer;
LSAN_OPTIONS=suppressions=/home/mnovak/Projects/temporary-escape/src/supp:fast_unwind_on_malloc=0;

VULKAN_SDK=/opt/vulkan-sdk/1.3.239.0/x86_64
LD_LIBRARY_PATH=/opt/vulkan-sdk/1.3.239.0/x86_64/lib
VK_LAYER_PATH=/opt/vulkan-sdk/1.3.239.0/x86_64/etc/vulkan/explicit_layer.d
```

```
-DTEMPORARY_ESCAPE_LLVM_SYMBOLIZER_PATH=/usr/bin/llvm-symbolizer
```

# 4. Modules

Each Lua script is a module. In some languages (such as Python) creating a variable outside of classes and functions is
only global to the script (module) where you have created it.

**In Lua, creating such variable is global everywhere! We don't want that and we want to avoid that all the time!**

Therefore, it is highly recommended to always use `local` keyword to create local variables. An example:

```lua
-- DO NOT CREATE GLOBAL VARIABLES!!!
my_var = 42

-- Do this instead!
local my_var = 42

-- DO NOT CREATE GLOBAL FUNCTIONS!!!
function my_function()
    print("Hello World!")
end

-- Do this instead!
local function my_function()
    print("Hello World!")
end
```

However, sometimes you need to use functions or variables from another module. To do that simply create a local `module`
table and return it at the end of the file. All functions and variables you wish to use from another module must be part
of this table. Example:

```lua
-- File: foo_module.lua

-- Visible only to this script file!
local my_local_var = 42

-- Define a table to export functions to other modules that import this script file
local module = {}

module.my_var = 42

function module.my_function()
    print("Hello World!")
end

return module
```

Then, in another module, you can use these exported functions and variables as the following:

```lua
local foo = require("my_mod.foo_module")

-- This won't work
print(string.format("The value of my_local_var is: %d"), foo.my_local_var)

-- This will work
foo.my_function()
print(string.format("The value of my_var is: %d"), foo.my_var)
```

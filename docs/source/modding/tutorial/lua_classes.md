# 5. Classes

Most things in this game uses a class based approach. This may be different from what you may be used to in other games.

## An example

A class is defined like this:

```lua
-- File: foo.lua
local MyFooClass = {}

function MyFooClass.new (first_arg, second_arg)
    local inst = {}
    setmetatable(inst, { __index = MyFooClass })
    inst.first = first_arg
    inst.second = second_arg
    return inst
end

function MyFooClass.print_variables (self)
    print("MyFooClass variables:")
    print(string.format(" - first: %d", self.first))
    print(string.format(" - second: %s", self.second))
end

return MyFooClass
```

**Notice that we are returning the class from the script!** This is on purpose. The class is treated as the module.
This is almost identical behavior to the JavaScript ES or TypeScript language.

From another script file, you can use it as the following:

```lua
-- File: sector.lua
local MyFooClass = require("my_mod.foo")

local foo = MyFooClass.new(123, "abcd")
foo.first = 456
foo:print_variables()
```

**Couple of things to keep in mind:**

* Constructing a new instance of class is always done by the `.new()` function. Some other games use the colon
  symbol `:`, but in this game the dot-new `.new()` is always used, no exceptions.
* Calling functions is done by the colon symbol `:` after the variable name as: `instance:function_name(argument)`
* Classes should be treated as modules and "exported" (returned at the end of the script)

## Inheritance

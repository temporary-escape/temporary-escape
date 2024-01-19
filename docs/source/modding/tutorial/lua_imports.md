# 3. Imports

Importing another script file is done via the `require` function. The imports work based on module names
and use absolute paths. Each file is a separate module. This is almost identical to Python's import behavior.

## Local mod files

For example, if your mod folder structure looks like this:

```text
assets/   <- Contains all mods
    base/   <-- The base game mod
    my_mod/ <-- Your custom mod
        utils/
            foo.lua
        bar.lua
        sector.lua
        manifest.xml
```

And you want to import these two scripts from the `sector.lua`:

* `my_mod/bar.lua`
* `my_mod/utils/foo.lua`

Then inside of the `sector.lua` you have to write:

```lua
-- File: my_mod/sector.lua
local foo = require("my_mod.utils.foo")
local bar = require("my_mod.bar")
```

An equivalent in Python would be:

```python
import my_mod.utils.foo as foo
import my_mod.bar as bar
```

_This is just a Python equivalent, we don't use Python in this game!_

**What about importing `bar.lua` into `foo.lua`?** It works the exact same way:

```lua
-- File: my_mod/utils/foo.lua

local bar = require("my_mod.bar")
```

## Import from other mod

Importing scripts from other mods is allowed. It works the same way as with local script files. The only difference
is the name of the mod in the import name.

For example, if your mod folder structure looks like this:

```text
assets/   <- Contains all mods
    base/   <-- The base game mod
        utils/
            foo.lua
    my_mod/ <-- Your custom mod
        sector.lua
        manifest.xml
```

And you want to import a script `base/utils/foo.lua` from the base mod into your mod's script `my_mod/sector.lua`, then
you have to do the following:

```lua
-- File: my_mod/sector.lua
local foo = require("base.utils.foo")
```

Simple, right?

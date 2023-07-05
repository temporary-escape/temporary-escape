import enum
import os
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import List, Dict, Tuple, Optional

DIR = os.path.dirname(os.path.realpath(__file__))


class LuaKind(enum.Enum):
    Function = "function"
    Field = "field"
    Class = "class"


class LuaAccess(enum.Enum):
    Readonly = "readonly"
    Writeonly = "writeonly"
    ReadWrite = "readwrite"


@dataclass
class LuaPatameter:
    name: str
    type: str
    description: str


@dataclass
class LuaElement:
    name: str
    kind: LuaKind
    items: List['LuaElement']
    params: List[LuaPatameter]
    description: str = ""
    static: bool = False
    module: Optional['LuaModule'] = None
    readonly: bool = False
    type: Optional[str] = None
    returns: Optional[LuaPatameter] = None
    access: LuaAccess = LuaAccess.ReadWrite
    note: Optional[str] = None
    warn: Optional[str] = None
    code: Optional[str] = None
    see: Optional[str] = None


@dataclass
class LuaModule:
    name: str
    items: List[LuaElement]


class Parser:
    def __init__(self, content: str, modules: Dict[str, LuaModule]):
        self.lines = content.split("\n")
        self.index = 0
        self.line: str = None
        self.module: LuaModule = None
        self.modules = modules

    def next(self) -> bool:
        index = self.index
        if index >= len(self.lines):
            return False
        self.line = self.lines[index]
        self.index += 1
        return True

    def new_module(self, name: str):
        if name not in self.modules:
            self.module = LuaModule(name=name, items=[])
            self.modules[name] = self.module
        else:
            self.module = self.modules[name]

        return self.module

    def find_parent(self, name: str):
        if self.module is None:
            return None
        for item in self.module.items:
            if item.name == name:
                return item
        return None

    def accept_description(self, item: LuaElement, line: str):
        if item.description and not item.description.endswith(" "):
            item.description += " "
        item.description += line

    def accept_param(self, item: LuaElement, text: str):
        tokens = text.split(" ", maxsplit=2)
        if len(tokens) != 3:
            print(f"Bad param string: '{text}', expected <name> <type> <text>")
            sys.exit(1)

        item.params.append(LuaPatameter(
            name=tokens[0],
            type=tokens[1],
            description=tokens[2]
        ))

    def accept_type(self, item: LuaElement, text: str):
        item.type = text

    def accept_return(self, item: LuaElement, text: str):
        tokens = text.split(" ", maxsplit=1)
        if len(tokens) != 2:
            print(f"Bad param string: '{text}', expected <type> <text]")
            sys.exit(1)
        item.returns = LuaPatameter(
            type=tokens[0],
            name="result",
            description=tokens[1]
        )

    def accept_readonly(self, item: LuaElement):
        item.access = LuaAccess.Readonly

    def accept_note(self, item: LuaElement, text: str):
        item.note = text

    def accept_warn(self, item: LuaElement, text: str):
        item.warn = text

    def accept_code(self, item: LuaElement):
        code = ""
        while self.next():
            stripped = self.line.strip()
            if stripped.startswith("* "):
                stripped = stripped[2:]
            elif stripped.startswith("*"):
                stripped = stripped[1:]

            if stripped == "@endcode":
                break
            if code:
                code += "\n"
            code += stripped
        item.code = code

    def accept_see(self, item: LuaElement, text: str):
        item.see = text

    def accept_annotation(self, item: LuaElement):
        while self.next():
            stripped = self.line.strip()
            if not stripped.startswith("* ") or stripped == "*/":
                return

            # Erase "* " beginning
            stripped = stripped[2:]
            if stripped.startswith("@"):
                tokens = stripped.split(" ")
                annotation = tokens[0]
                if len(tokens) > 1:
                    text = " ".join(tokens[1:])
                else:
                    text = None
                if annotation == "@param":
                    self.accept_param(item, text)
                elif annotation == "@type":
                    self.accept_type(item, text)
                elif annotation == "@return":
                    self.accept_return(item, text)
                elif annotation == "@readonly":
                    self.accept_readonly(item)
                elif annotation == "@note":
                    self.accept_note(item, text)
                elif annotation == "@warning":
                    self.accept_warn(item, text)
                elif annotation == "@code":
                    self.accept_code(item)
                elif annotation == "@see":
                    self.accept_see(item, text)
            else:
                self.accept_description(item, stripped)

    def parse_ownership(self, name: str) -> Tuple[bool, str | None, str]:
        if "." in name:
            owner, name = name.split(".", maxsplit=1)
            is_static = True
        elif ":" in name:
            owner, name = name.split(":", maxsplit=1)
            is_static = False
        else:
            owner = None
            is_static = True
        return is_static, owner, name

    def accept_class(self, name: str):
        klass = LuaElement(
            name=name,
            kind=LuaKind.Class,
            items=[],
            module=self.module,
            params=[],
        )
        self.module.items.append(klass)
        self.accept_annotation(klass)

    def accept_function(self, name: str):
        is_static, parent, function_name = self.parse_ownership(name)

        function = LuaElement(
            name=function_name,
            kind=LuaKind.Function,
            items=[],
            static=is_static,
            module=self.module,
            params=[],
        )

        if parent is not None:
            parent = self.find_parent(parent)
            if parent is None:
                print(f"Function: {name} has an unknown parent!")
                sys.exit(1)

            parent.items.append(function)
        else:
            self.module.items.append(function)

        self.accept_annotation(function)

    def accept_field(self, name: str):
        is_static, parent, field_name = self.parse_ownership(name)

        if not is_static:
            print(f"Field: {name} can not contain ':' character!")
            sys.exit(1)

        field = LuaElement(
            name=field_name,
            kind=LuaKind.Field,
            items=[],
            module=self.module,
            static=False,
            params=[],
        )

        if parent is not None:
            parent = self.find_parent(parent)
            if parent is None:
                print(f"Field: {name} has an unknown parent!")
                sys.exit(1)

            parent.items.append(field)
        else:
            self.module.items.append(field)
        self.accept_annotation(field)

    def accept_comment(self, line: str):
        annotation, text = line.split(" ", maxsplit=1)
        if annotation == "@module":
            self.new_module(text)
        elif self.module != None and annotation == "@class":
            self.accept_class(text)
        elif self.module != None and annotation == "@function":
            self.accept_function(text)
        elif self.module != None and annotation == "@field":
            self.accept_field(text)
        elif annotation == "@endmodule":
            self.module = None

    def accept_comment_body(self):
        while self.next():
            stripped = self.line.strip()
            if not stripped.startswith("* ") or stripped == "*/":
                return
            # Erase "* " beginning
            stripped = stripped[2:]
            if stripped.startswith("@"):
                self.accept_comment(stripped)
                return

    def parse(self):
        while self.next():
            stripped = self.line.strip()
            if stripped == "/**":
                self.accept_comment_body()


def process_file(file: str, modules: Dict[str, LuaModule]):
    print(f"Processing: {file}")
    with open(file, "r") as f:
        content = f.read()

    parser = Parser(content, modules)
    parser.parse()


def get_member_functions(item: LuaElement) -> List[LuaElement]:
    results = []
    for child in item.items:
        if child.kind == LuaKind.Function and child.static == False:
            results.append(child)
    return results


def get_static_functions(item: LuaElement) -> List[LuaElement]:
    results = []
    for child in item.items:
        if child.kind == LuaKind.Function and child.static == True and child.name != "new":
            results.append(child)
    return results


def get_constructors(item: LuaElement) -> List[LuaElement]:
    results = []
    for child in item.items:
        if child.kind == LuaKind.Function and child.static == True and child.name == "new":
            results.append(child)
    return results


def get_member_fields(item: LuaElement) -> List[LuaElement]:
    results = []
    for child in item.items:
        if child.kind == LuaKind.Field and child.static == False:
            results.append(child)
    return results


def params_str(item: LuaElement) -> str:
    text = ""
    for param in item.params:
        if text:
            text += ", "
        text += param.name
    return text


def safe_url(text: str):
    return text.replace("<", "_").replace(">", "_")


def safe_md(text: str):
    return text.replace("<", "&lt;").replace(">", "&gt;")


def generate_docs_child(f, child: LuaElement):
    if child.description:
        f.write(f"{child.description}\n\n")

    if child.access == LuaAccess.Readonly:
        f.write(f"**Read-only!**\n\n")

    if child.access == LuaAccess.Writeonly:
        f.write(f"**Write-only!**\n\n")

    if child.kind == LuaKind.Field and child.type:
        f.write(f"**Type:** `{child.type}`\n\n")

    if child.params:
        f.write(f"**Parameters:**\n\n")
        for param in child.params:
            f.write(f"* {param.name}: `{param.type}`")
            if param.description:
                f.write(f" - {param.description}\n")
            else:
                f.write(f"\n")

        f.write("\n")

    if child.returns:
        f.write(f"**Returns:** `{child.returns.type}`")
        if child.returns.description:
            f.write(f" - {child.returns.description}\n")
        else:
            f.write(f"\n")

        f.write("\n")

    if child.code:
        f.write("**Example:**\n\n")
        f.write("```lua\n")
        f.write(child.code)
        f.write("\n```\n\n")

    if child.note:
        f.write("```{note}\n")
        f.write(child.note)
        f.write("\n```\n\n")
    
    if child.warn:
        f.write("```{warning}\n")
        f.write(child.warn)
        f.write("\n```\n\n")

    if child.see:
        f.write(f"**See also:** [{safe_md(child.see)}]({safe_url(child.see)})\n\n")


def generate_docs_class(path: str, item: LuaElement):
    with open(path, "w") as f:
        f.write(f"# Class {safe_md(item.name)}\n\n")

        f.write(f"**Module:** `require(\"{item.module.name}\")`\n\n")

        generate_docs_child(f, item)

        constructors = get_constructors(item)
        if constructors:
            f.write("------------\n")
            f.write("## Constructors\n\n")
            f.write(f"List of constructors that will create an instance of `{item.name}`.\n")
            f.write("\n")

            for child in constructors:
                f.write(f"### {child.name}({params_str(child)})\n\n")
                generate_docs_child(f, child)

        member_functions = get_member_functions(item)
        if member_functions:
            f.write("------------\n")
            f.write("## Member Functions\n\n")
            f.write(f"List of **member** functions within the table `{item.name}`.\n")
            f.write(f"These functions need to be called using the colon symbol on some instance of `{item.name}`.\n")
            f.write("\n")

            for child in member_functions:
                f.write(f"### {child.name}({params_str(child)})\n\n")
                generate_docs_child(f, child)

        static_functions = get_static_functions(item)
        if static_functions:
            f.write("------------\n")
            f.write("## Static Functions\n\n")
            f.write(f"List of functions within the table `{item.name}`.\n")
            f.write(f"These functions can be called directly on the class without an instance.\n")
            f.write("\n")

            for child in static_functions:
                f.write(f"### {child.name}({params_str(child)})\n\n")
                generate_docs_child(f, child)

        fields = get_member_fields(item)
        if fields:
            f.write("------------\n")
            f.write("## Fields\n\n")
            f.write(f"List of fields within the table `{item.name}`.\n")
            f.write(f"These variables belong to this class and can be accessed only via an instance of this class.\n")
            f.write("\n")

            for child in fields:
                f.write(f"### {child.name}\n\n")
                generate_docs_child(f, child)


def generate_docs_index(path: str, toctree: str, module: LuaModule):
    with open(path, "w") as f:
        f.write(f"# Lua API\n\n")

        f.write("## Classes\n\n")

        f.write("| Class | Description |\n")
        f.write("| ----- | ----------- |\n")
        for item in module.items:
            if item.kind != LuaKind.Class:
                continue

            f.write(f"| [{item.name}]({safe_url(item.name)}) | {item.description} |\n")
        f.write("\n")

        f.write("```{toctree}\n")
        f.write(":hidden:\n")
        f.write(toctree)
        f.write("```\n")


def generate_docs(path: str, module: LuaModule):
    print(f"Generating docs in: {path}")

    # Remove old files
    for file in Path(path).rglob("*.md"):
        print(f"Removing old file: {file}")
        os.remove(file)

    toctree = ""
    for item in module.items:
        if item.kind == LuaKind.Class:
            dst = os.path.join(path, safe_url(item.name) + ".md")
            generate_docs_class(dst, item)
            toctree += f"{safe_url(item.name)}\n"

    generate_docs_index(os.path.join(path, "index.md"), toctree, module)


def main():
    path = os.path.join(DIR, "..", "src", "engine")
    dst = os.path.join(DIR, "source", "modding", "api")
    modules: Dict[str, LuaModule] = {}
    for file in Path(path).rglob("*.cpp"):
        process_file(file, modules)

    generate_docs(dst, modules["engine"])


if __name__ == "__main__":
    main()

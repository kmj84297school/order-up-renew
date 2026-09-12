#!/usr/bin/env python3
"""
Static consistency checks for the HayDayFarm Unreal module.

This is NOT a compiler and does not pretend to be one. It exists because the
project is developed in an environment without an Unreal installation, so the
usual "just build it" validation is unavailable. It catches the error classes
that are actually likely when writing Unreal C++ by hand:

  * a UCLASS/USTRUCT/UENUM header missing its .generated.h, or not having it
    last (UHT requires it to be the final include)
  * a UCLASS/USTRUCT without GENERATED_BODY()
  * UPROPERTY/UFUNCTION outside a reflected type
  * a project include that points at a file that does not exist
  * unbalanced braces/parens
  * a method declared in a header with no definition anywhere (link error)
  * .uproject / Build.cs / Target.cs / module directory name disagreement
  * a referenced Farm* type that is never declared

Run:  python3 Tools/validate_project.py
Exit: 0 = clean, 1 = errors found.
"""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent.parent
MODULE_NAME = "HayDayFarm"
MODULE_ROOT = PROJECT_ROOT / "Source" / MODULE_NAME
API_MACRO = f"{MODULE_NAME.upper()}_API"

errors: list[str] = []
warnings: list[str] = []


def error(path: Path, msg: str) -> None:
    errors.append(f"{path.relative_to(PROJECT_ROOT)}: {msg}")


def warn(path: Path, msg: str) -> None:
    warnings.append(f"{path.relative_to(PROJECT_ROOT)}: {msg}")


def tokenize(text: str) -> tuple[str, str]:
    """Returns (without_comments, without_comments_and_string_contents).

    Both preserve offsets and line numbers so brace positions and reported
    line numbers stay meaningful. The first form keeps string contents, which
    the include checks need; the second blanks them, which every other check
    needs so that a brace or a keyword inside a literal cannot confuse it.
    """
    keep, blank = [], []
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if c == "/" and i + 1 < n and text[i + 1] == "/":
            while i < n and text[i] != "\n":
                keep.append(" ")
                blank.append(" ")
                i += 1
        elif c == "/" and i + 1 < n and text[i + 1] == "*":
            while i < n and not (text[i] == "*" and i + 1 < n and text[i + 1] == "/"):
                filler = "\n" if text[i] == "\n" else " "
                keep.append(filler)
                blank.append(filler)
                i += 1
            keep.append("  ")
            blank.append("  ")
            i += 2
        elif c in "\"'":
            quote = c
            keep.append(quote)
            blank.append(quote)
            i += 1
            while i < n and text[i] != quote:
                if text[i] == "\\" and i + 1 < n:
                    keep.append(text[i:i + 2])
                    blank.append("  ")
                    i += 2
                    continue
                keep.append(text[i])
                blank.append("\n" if text[i] == "\n" else " ")
                i += 1
            if i < n:
                keep.append(quote)
                blank.append(quote)
                i += 1
        else:
            keep.append(c)
            blank.append(c)
            i += 1
    return "".join(keep), "".join(blank)


def strip_code(text: str) -> str:
    return tokenize(text)[1]


def check_balance(path: Path, code: str) -> None:
    pairs = {")": "(", "]": "[", "}": "{"}
    stack = []
    line = 1
    for ch in code:
        if ch == "\n":
            line += 1
        elif ch in "([{":
            stack.append((ch, line))
        elif ch in ")]}":
            if not stack or stack[-1][0] != pairs[ch]:
                error(path, f"line {line}: unbalanced '{ch}'")
                return
            stack.pop()
    if stack:
        ch, line = stack[-1]
        error(path, f"unclosed '{ch}' opened at line {line}")


def check_header(path: Path, raw: str, code: str, text: str) -> None:
    if "#pragma once" not in raw:
        error(path, "missing '#pragma once'")

    includes = re.findall(r'^\s*#include\s+"([^"]+)"', text, re.M)
    reflected = re.search(r"^\s*(UCLASS|USTRUCT|UENUM|UINTERFACE)\s*\(", code, re.M)
    generated = f"{path.stem}.generated.h"

    if reflected:
        if generated not in includes:
            error(path, f"has {reflected.group(1)} but does not include \"{generated}\"")
        elif includes[-1] != generated:
            error(
                path,
                f'"{generated}" must be the LAST include (found "{includes[-1]}" after it)',
            )
    elif generated in includes:
        error(path, f'includes "{generated}" but declares no reflected type')

    # Every UCLASS/USTRUCT must be followed by a type with GENERATED_BODY().
    for m in re.finditer(r"^\s*(UCLASS|USTRUCT|UINTERFACE)\s*\(", code, re.M):
        tail = code[m.end():m.end() + 1200]
        body_start = tail.find("{")
        if body_start == -1:
            error(path, f"{m.group(1)} at offset {m.start()} has no type body")
            continue
        if "GENERATED_BODY()" not in tail[body_start:body_start + 400]:
            line = code[: m.start()].count("\n") + 1
            error(path, f"line {line}: {m.group(1)} type is missing GENERATED_BODY()")

    # UPROPERTY / UFUNCTION are only legal inside a reflected type.
    if not reflected:
        for kw in ("UPROPERTY", "UFUNCTION"):
            if re.search(rf"^\s*{kw}\s*\(", code, re.M):
                error(path, f"{kw} used in a header with no reflected type")


def resolve_include(inc: str) -> bool:
    """True if the include names a file inside this module."""
    return (MODULE_ROOT / inc).is_file()


def module_files_named(basename: str) -> list[Path]:
    return [p for p in MODULE_ROOT.rglob(basename) if p.is_file()]


def check_includes(path: Path, text: str) -> None:
    """Flags project includes whose path is wrong.

    Farm-prefixed basenames belong to this project, even when missing.
    Other names are judged only when the file exists somewhere in the module:
    Camera/ is also an engine directory, so a directory-prefix test is unsafe.
    """
    for inc in re.findall(r'^\s*#include\s+"([^"]+)"', text, re.M):
        if inc.endswith(".generated.h") or resolve_include(inc):
            continue

        candidates = module_files_named(Path(inc).name)
        if not candidates:
            if Path(inc).name.startswith("Farm"):
                error(path, f'project include "{inc}" does not exist')
            continue  # engine / plugin header

        actual = ", ".join(str(c.relative_to(MODULE_ROOT)) for c in candidates)
        error(path, f'include "{inc}" is wrong; the module file is at {actual}')


DECL_RE = re.compile(
    r"""^[ \t]*
        (?:(?:virtual|static|explicit|inline|FORCEINLINE)\s+)*
        (?P<ret>[A-Za-z_][\w:<>,\*& \t]*?)[ \t]+
        (?P<name>~?[A-Za-z_]\w*)[ \t]*
        \((?P<args>[^;{}()\n]*)\)[ \t]*
        (?:const\s*)?(?:override\s*)?(?:noexcept\s*)?
        ;\s*$""",
    re.M | re.X,
)


def collect_class_bodies(code: str) -> list[tuple[str, str]]:
    """Returns (class_name, body_text) for each class declared in a header."""
    results = []
    for m in re.finditer(rf"\bclass\s+(?:{API_MACRO}\s+)?(?P<name>[A-Z]\w+)\s*(?::[^{{;]*)?\{{", code):
        start = m.end() - 1
        depth = 0
        for i in range(start, len(code)):
            if code[i] == "{":
                depth += 1
            elif code[i] == "}":
                depth -= 1
                if depth == 0:
                    results.append((m.group("name"), code[start + 1:i]))
                    break
    return results


NON_DEFINITION_NAMES = {"if", "for", "while", "switch", "return", "else", "do"}

# UnrealHeaderTool generates the body of these, so a declaration with no
# hand-written definition is correct rather than a link error. The author
# supplies the _Implementation instead, which is checked normally.
UHT_GENERATED_SPECIFIERS = ("BlueprintNativeEvent", "BlueprintImplementableEvent")


def is_uht_generated(body: str, decl_start: int) -> bool:
    """True if the declaration is preceded by a UFUNCTION that UHT implements."""
    preceding = body[max(0, decl_start - 400):decl_start]
    last_ufunction = preceding.rfind("UFUNCTION")
    if last_ufunction == -1:
        return False
    # Only the UFUNCTION immediately before this declaration counts.
    between = preceding[last_ufunction:]
    if between.count(";") > 0:
        return False
    return any(spec in between for spec in UHT_GENERATED_SPECIFIERS)


def check_definitions(header: Path, code: str) -> None:
    cpp = header.with_suffix(".cpp")
    cpp_code = strip_code(cpp.read_text()) if cpp.is_file() else ""

    # Definitions may also live in another translation unit of the module.
    for class_name, body in collect_class_bodies(code):
        for m in DECL_RE.finditer(body):
            name = m.group("name")
            ret = m.group("ret").strip()
            if name in NON_DEFINITION_NAMES or ret.endswith(("return", "=")):
                continue
            if is_uht_generated(body, m.start()):
                continue
            if "=" in m.group(0).split(")")[-1]:  # pure virtual or = default/delete
                continue
            if re.search(rf"\b{re.escape(class_name)}::{re.escape(name)}\s*\(", cpp_code):
                continue
            if any(
                re.search(rf"\b{re.escape(class_name)}::{re.escape(name)}\s*\(", strip_code(o.read_text()))
                for o in MODULE_ROOT.rglob("*.cpp")
            ):
                continue
            line = body[: m.start()].count("\n") + code[: code.find(body)].count("\n") + 1
            error(header, f"{class_name}::{name}() is declared but never defined (link error)")


def check_project_wiring() -> None:
    uproject = PROJECT_ROOT / f"{MODULE_NAME}.uproject"
    if not uproject.is_file():
        errors.append(f"{MODULE_NAME}.uproject is missing")
        return

    data = json.loads(uproject.read_text())
    module_names = [m.get("Name") for m in data.get("Modules", [])]
    if MODULE_NAME not in module_names:
        error(uproject, f"does not declare module '{MODULE_NAME}' (found {module_names})")

    if not MODULE_ROOT.is_dir():
        errors.append(f"Source/{MODULE_NAME}/ directory is missing")

    build_cs = MODULE_ROOT / f"{MODULE_NAME}.Build.cs"
    if not build_cs.is_file():
        errors.append(f"Source/{MODULE_NAME}/{MODULE_NAME}.Build.cs is missing")
    elif f"public class {MODULE_NAME} : ModuleRules" not in build_cs.read_text():
        error(build_cs, f"class name must be '{MODULE_NAME}' to match the module")

    for target, kind in ((f"{MODULE_NAME}.Target.cs", "Target"), (f"{MODULE_NAME}Editor.Target.cs", "EditorTarget")):
        tpath = PROJECT_ROOT / "Source" / target
        if not tpath.is_file():
            errors.append(f"Source/{target} is missing")
            continue
        expected = target.replace(".Target.cs", "Target")
        if f"public class {expected} : TargetRules" not in tpath.read_text():
            error(tpath, f"class must be named '{expected}'")

    impl = MODULE_ROOT / f"{MODULE_NAME}.cpp"
    if impl.is_file() and "IMPLEMENT_PRIMARY_GAME_MODULE" not in impl.read_text():
        error(impl, "primary game module must call IMPLEMENT_PRIMARY_GAME_MODULE")


def check_type_references(headers: list[Path], all_files: list[Path]) -> None:
    declared = set()
    for h in headers:
        code = strip_code(h.read_text())
        declared.update(re.findall(r"\bclass\s+(?:%s\s+)?([AUI]Farm\w+)" % API_MACRO, code))
        declared.update(re.findall(r"\bstruct\s+(?:%s\s+)?(FFarm\w+)" % API_MACRO, code))
        declared.update(re.findall(r"\benum class\s+(EFarm\w+)", code))
        # DECLARE_*_DELEGATE*(FName, ...) declares a type via macro.
        declared.update(re.findall(r"\bDECLARE_\w*DELEGATE\w*\s*\(\s*(FFarm\w+)", code))

    for f in all_files:
        code = strip_code(f.read_text())
        for name in set(re.findall(r"\b([AUEI]Farm\w+|FFarm\w+)\b", code)):
            if name not in declared:
                error(f, f"references type '{name}' which is never declared in the module")


def main() -> int:
    headers = sorted(MODULE_ROOT.rglob("*.h"))
    sources = sorted(MODULE_ROOT.rglob("*.cpp"))
    all_files = headers + sources

    if not headers:
        print("No headers found -- is the module path correct?")
        return 1

    check_project_wiring()

    for path in all_files:
        raw = path.read_text()
        text, code = tokenize(raw)
        check_balance(path, code)
        check_includes(path, text)
        if path.suffix == ".h":
            check_header(path, raw, code, text)
            check_definitions(path, code)

    check_type_references(headers, all_files)

    print(f"Checked {len(headers)} headers and {len(sources)} sources in Source/{MODULE_NAME}/.\n")

    for w in warnings:
        print(f"  WARN  {w}")
    for e in errors:
        print(f"  ERROR {e}")

    if errors:
        print(f"\n{len(errors)} error(s).")
        return 1

    print(f"No structural errors found ({len(warnings)} warning(s)).")
    print("NOTE: this is a static check, not a compile. A real build still")
    print("      requires Unreal Engine on Windows -- see NEXT_TASK.md.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

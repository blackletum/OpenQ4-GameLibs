#!/usr/bin/env python3
"""Static checks for the openQ4-GameLibs source-input contract."""

from __future__ import annotations

from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def read(relative_path: str) -> str:
    return (ROOT / relative_path).read_text(encoding="utf-8")


def require(haystack: str, needle: str, context: str) -> None:
    if needle not in haystack:
        raise AssertionError(f"Missing {needle!r} in {context}")


def main() -> None:
    readme = read("README.md")
    agents = read("AGENTS.md")
    src_meson = read("src/meson.build")
    workflow = read(".github/workflows/commit-validation.yml")

    require(readme, "canonical source-input repository", "README source-input overview")
    require(readme, "openQ4 consumes this repository as source input", "README integration contract")
    require(readme, "Linux source-input contract checks", "README CI contract")
    require(readme, "rather than building standalone Linux modules", "README Linux build boundary")
    require(readme, "openQ4's staged engine build is the supported path", "README Linux integration path")

    require(agents, "canonical source-input repository", "AGENTS cross-repo workflow")
    require(agents, "Linux standalone module builds are intentionally not supported here", "AGENTS Linux boundary")
    require(agents, "openQ4's staged engine build", "AGENTS Linux validation path")
    require(agents, "Do not maintain or edit an openQ4 `src/game` mirror", "AGENTS mirror warning")

    require(src_meson, "if not is_windows and not is_darwin", "Meson standalone platform guard")
    require(
        src_meson,
        "Windows/MSVC and macOS/Clang bring-up",
        "Meson unsupported Linux diagnostic",
    )

    require(workflow, "runs-on: ubuntu-24.04", "Linux CI runner")
    require(workflow, "Script Smoke", "Linux source-input CI job")
    require(workflow, "tools/tests/source_input_contract.py", "workflow source-input test compile")
    require(workflow, "python tools/tests/source_input_contract.py", "workflow source-input test run")

    print("source_input_contract: ok")


if __name__ == "__main__":
    main()

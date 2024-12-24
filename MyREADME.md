## Build

Assume you've cloned the vcpkg repository to 'A/vcpkg/'. If you are on Windows system, run 'A/vcpkg/bootstrap-vcpkg.exe'. On Linux, run 'A/vcpkg/bootstrap-vcpkg.sh'. This installs vcpkg.

After that, run `vcpkg install` in the repo directory. The `vcpkg.json` file contains what vcpkg should help you install. This process is in theory automatically done by cmake, but it just didn't happen on my device even if I added `-DCMAKE_TOOLCHAIN_FILE={vcpkg path}/scripts/buildsystems/vcpkg.cmake`. The installed libraries should be located at `{repo path}/vcpkg_installed` if you use this way. On the other hand, there is an exactly same `vcpkg.json` under `{repo path}/external/grassland/src` so installing there is also a choice.

Depending on where the `vcpkg_installed` directory actually is, add `.../vcpkg_installed/x64-windows (or linux)/` to environment variable PATH. This enables the `find_package` function in `{repo path}/external/grassland/` to find the packages installed by vcpkg, and also makes numerous headers under `.../vcpkg_installed/x64-windows (or linux)/include/` available to the codes. On Windows, `.../vcpkg_installed/x64-windows (or linux)/debug/bin/` should be added to PATH too because this contains important `.dll` files without which the game couldn't run.

On Linux , a possible error (which occurred on my wsl) is
```bash
Target "freetype" links to:

    ZLIB::ZLIB

but the target was not found. Possible reasons include:

    * There is a typo in the target name.
    * A find_package call is missing for an IMPORTED target.
    * An ALIAS target is missing.
```
This corresponds to an error in `freetype-config.cmake` just downloaded, which then results from the following lines in `freetype-targets.cmake`:
```bash
add_library(freetype STATIC IMPORTED)

set_target_properties(free type PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
    INTERFACE_LINK_LIBRARIES "\$<LINK_ONLY:ZLIB::ZLIB>; ... "
)
```
Adding `find_package(ZLIB REQUIRED)` into the blank line in the middle seems to solve this problem.


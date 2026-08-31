#!/usr/bin/env sh
set -eu

repository_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
dependency_root="${repository_root}/.dependencies"
vcpkg_root="${VCPKG_ROOT:-${dependency_root}/vcpkg}"
baseline="63feddf004cc39169a8b0b7f79c2eba5065d6140"

if [ ! -d "${vcpkg_root}/.git" ]; then
  mkdir -p "${dependency_root}"
  git clone https://github.com/microsoft/vcpkg.git "${vcpkg_root}"
fi

git -C "${vcpkg_root}" fetch --depth 1 origin "${baseline}"
git -C "${vcpkg_root}" checkout --detach "${baseline}"
"${vcpkg_root}/bootstrap-vcpkg.sh" -disableMetrics

printf '%s\n' "vcpkg is ready at ${vcpkg_root}"
printf '%s\n' "Run: export VCPKG_ROOT='${vcpkg_root}'"
printf '%s\n' "Then: cmake --preset debug && cmake --build --preset debug && ctest --preset debug"

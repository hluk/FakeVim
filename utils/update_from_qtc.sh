#!/bin/bash
files_to_update=(
    fakevimactions.cpp
    fakevimactions.h
    fakevimhandler.cpp
    fakevimhandler.h
    fakevimtr.h
)

# Standalone shim headers in fakevim/utils/ (hostosinfo.h, store.h, storekey.h)
# are NOT from upstream — they provide minimal standalone implementations of
# Utils::Key, Utils::HostOsInfo etc. They are not touched by this script and
# survive the sync automatically since we only rsync individual files above.

qtc_home=$1

script_dir=$(dirname "$(readlink -f "$0")")
base_dir=$script_dir/..

die() {
    echo "$1" 1>&2
    exit 1
}

set -e

[ -n "$qtc_home" ] ||
    die "Usage: $0 PATH_TO_QT_CREATOR"

echo "--- Fetching latest development code for Qt Creator"
cd "$qtc_home"
git fetch origin master
git checkout origin/master
commit=$(git rev-parse --short HEAD)

echo "--- Updating source files"
cd "$base_dir/fakevim"
for file in "${files_to_update[@]}"; do
    echo "-- $file"
    if [[ "$file" == fakevim* ]]; then
        dir=plugins/fakevim
    else
        dir=libs
    fi
    rsync --mkpath -- "$qtc_home/src/$dir/$file" "$file"
    git add -- "$file"
done

echo "--- Patching source files and creating commit"
git commit -m "Update from Qt Creator (commit $commit)"
git apply --3way -- "$script_dir/patches/add-patches-for-upstream.patch"
git add -- "${files_to_update[@]}"
git commit --amend --no-edit --allow-empty

echo ""
echo "--- Post-sync manual steps required:"
echo "  1. Build: mkdir -p build && cd build && cmake .. -DBUILD_TESTS=ON -DBUILD_EXAMPLE=ON && cmake --build ."
echo "  2. If build fails with QString/QByteArray conversion errors, fix them in fakevimhandler.cpp"
echo "     and regenerate the patch: git diff HEAD~1 -- fakevim/*.{h,cpp} > $script_dir/patches/add-patches-for-upstream.patch"
echo "     then: git add fakevim/ utils/patches/ && git commit --amend --no-edit"
echo "  3. Run tests: cd build && ctest --output-on-failure"
echo "     Fix any test expectation changes in tests/fakevim_test.cpp (backspace behavior is a known area)"
echo "  4. If upstream changed Callback or ExCommand API, update python/fakevimproxy.cpp (.connect → .set)"
echo "  5. Check example/editor.cpp for any API changes (settings accessor, signal signatures)"

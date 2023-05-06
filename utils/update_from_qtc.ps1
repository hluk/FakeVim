Param(
    [Parameter(HelpMessage="Synchronizes the Qt-Creator repository")]
    [switch]$Update = $false,

    [Parameter(Mandatory = $true, HelpMessage="Path to the Qt-Creator repository")]
    [string]$QtCreatorPath
)

if (!(Test-Path -Path $QtCreatorPath))
{
    Write-Output "Folder \"$($args[0])\" does not exist!"
    Exit(1)
}

$qtc_home = (Get-Item $QtCreatorPath)
$script_dir = $PSScriptRoot
$base_dir = (Get-Item $PSScriptRoot).Parent.FullName


Push-Location $qtc_home

if ($Update)
{
    Write-Output "--- Fetching latest development code for Qt Creator"
    git fetch origin master
}

git checkout origin/master
$commit = git rev-parse --short HEAD
Write-Output $commit

Pop-Location

Write-Output "--- Updating source files"
Push-Location "$base_dir/fakevim"

$files_to_update = @(
    'fakevimactions.cpp',
    'fakevimactions.h',
    'fakevimhandler.cpp',
    'fakevimhandler.h',
    'fakevimtr.h'
)

foreach ($file in $files_to_update)
{
    Write-Output "-- $file"
    Copy-Item "$qtc_home/src/plugins/fakevim/$file" $file
    git add -- $file
}

Write-Output "--- Patching source files and creating commit"
git commit -m "Update from Qt Creator (commit $commit)"
git apply -- "$script_dir/patches/add-patches-for-upstream.patch"
git add -- $($files_to_update -join " ")
git commit --amend --no-edit --allow-empty

Pop-Location

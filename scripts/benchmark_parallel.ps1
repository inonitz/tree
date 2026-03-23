$benchmarkExe = "ctest"
$processList = @()


# 1. Query CTest for all defined tests in JSON format
Write-Host "Querying CTest for benchmark list..." -ForegroundColor Cyan
$ctestJson = & $benchmarkExe --show-only=json-v1 | ConvertFrom-Json

# Filter for tests (excluding headers/metadata)
$tests = $ctestJson.tests
$totalCount = $tests.Count

if ($totalCount -eq 0) {
    Write-Error "No CTests found. Ensure you are in the build directory."
    return
}


Write-Host "Found $totalCount benchmarks. Launching..." -ForegroundColor Cyan


# 2. Launch each CTest pinned to a core
for ($i = 0; $i -lt $totalCount; $i++) {
    $testName = $tests[$i].name
    
    # Calculate core affinity (cycling through available processors)
    $coreIndex = $i % $env:NUMBER_OF_PROCESSORS
    $affinityMask = [int]([Math]::Pow(2, $coreIndex))

    # Launch ctest for this specific test name
    $p = Start-Process -FilePath $benchmarkExe `
        -ArgumentList "-R `"^$($testName)$`"", "--output-on-failure" `
        -PassThru -NoNewWindow
    
    # Apply Affinity
    $p.ProcessorAffinity = $affinityMask
    $processList += [PSCustomObject]@{ Process = $p; Name = $testName; Done = $false }
}


# 3. Progress Monitoring Loop
while ($true) {
    $finishedCount = ($processList | Where-Object { $_.Process.HasExited }).Count
    $percent = [int](($finishedCount / $totalCount) * 100)

    Write-Progress -Activity "Executing CTest Benchmarks with Affinity" `
                   -Status "$finishedCount of $totalCount Complete ($percent%)" `
                   -PercentComplete $percent `
                   -CurrentOperation "Core Affinity cycling 0 to $($env:NUMBER_OF_PROCESSORS - 1)"

    if ($finishedCount -eq $totalCount) { break }
    Start-Sleep -Seconds 2
}

Write-Progress -Activity "Executing CTest Benchmarks" -Completed
Write-Host "All CTests finished." -ForegroundColor Green
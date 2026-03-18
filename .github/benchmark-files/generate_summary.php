<?php

/**
 * Generate performance benchmark summary from callgrind results
 *
 * This script reads benchmark results from the results directory, merges them
 * into a single directory, and generates a markdown summary with performance
 * metrics for different PHP versions, commands, and Php-Debugger modes.
 */

// Create merged directory and copy all result files into it
if (!is_dir('merged')) {
    mkdir('merged', 0755, true);
    echo "Created merged directory\n";
}

// Check if results directory exists
if (!is_dir('results')) {
    fwrite(STDERR, "Error: results directory not found\n");
    exit(1);
}

// Recursively find all .txt files in results directory and copy them to merged
$iterator = new RecursiveIteratorIterator(
    new RecursiveDirectoryIterator('results', RecursiveDirectoryIterator::SKIP_DOTS),
    RecursiveIteratorIterator::LEAVES_ONLY
);

$fileCount = 0;
foreach ($iterator as $file) {
    if ($file->isFile() && $file->getExtension() === 'txt') {
        copy($file->getPathname(), 'merged/' . $file->getFilename());
        $fileCount++;
    }
}
echo "Copied $fileCount files to merged directory\n";

// Now merge all matrix-values files into a single unique list
$matrixFiles = glob('merged/matrix-values-*.txt');
if ($matrixFiles === false || $matrixFiles === []) {
    fwrite(STDERR, "Error: No matrix-values files found in merged directory\n");
    exit(1);
}

$allMatrixValues = [];
foreach ($matrixFiles as $file) {
    $lines = file($file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
    if ($lines !== false) {
        $allMatrixValues = array_merge($allMatrixValues, $lines);
    }
}

// Get unique values and sort them
$allMatrixValues = array_unique($allMatrixValues);
sort($allMatrixValues);

// Write the unique matrix values to file
file_put_contents('unique-matrix-values.txt', implode("\n", $allMatrixValues) . "\n");
echo "Created unique-matrix-values.txt with " . count($allMatrixValues) . " unique combinations\n";

// Fetch previous benchmark results for comparison
$previousResults = fetchPreviousBenchmarkResults();

// Parse unique matrix values to get all combinations
$matrixValues = $allMatrixValues;

// Extract unique values for each dimension
$commands = [];
$phpVersions = [];
$phpDebuggerModes = [];

foreach ($matrixValues as $line) {
    [$php, $command, $phpDebugger] = explode(',', $line);
    $phpVersions[$php] = true;
    $commands[$command] = true;
    $phpDebuggerModes[$phpDebugger] = true;
}

// Sort the arrays
$commands = array_keys($commands);
$phpVersions = array_keys($phpVersions);
$phpDebuggerModes = array_keys($phpDebuggerModes);
sort($commands);
sort($phpVersions);

// Sort Php-Debugger modes according to the defined order, leaving only those which actually exist in the data
$phpDebuggerModeOrder = ["no", "off", "debug"];
$phpDebuggerModes = array_values(array_filter($phpDebuggerModeOrder, function($mode) use ($phpDebuggerModes) {
    return in_array($mode, $phpDebuggerModes);
}));

// Start building the markdown summary and CSV data
$output = "# 🕒 Performance Results\n";
$csvData = [];
$csvData[] = ['command', 'php', 'php_debugger_mode', 'instructions', 'slowdown'];

// Loop through each command
foreach ($commands as $command) {
    $output .= "\n## **Command:** `$command`\n";

    // Loop through each PHP version
    foreach ($phpVersions as $php) {
        $output .= "\n### **PHP Version:** `$php`\n\n";
        $output .= "| Php-Debugger | Instructions | Slowdown | Improvement |\n";
        $output .= "|--------------|-------------:|---------:|------------:|\n";

        // Get base value (when Php-Debugger mode is "no")
        $baseFile = "merged/php-{$php}_cmd-{$command}_php_debugger-no.txt";
        if (!file_exists($baseFile)) {
            fwrite(STDERR, "Warning: Base file not found: $baseFile\n");
            continue;
        }

        $baseValue = (int)trim(file_get_contents($baseFile));

        // Loop through each Php-Debugger mode
        foreach ($phpDebuggerModes as $phpDebugger) {
            $file = "merged/php-{$php}_cmd-{$command}_php_debugger-{$phpDebugger}.txt";

            if (!file_exists($file)) {
                continue;
            }

            $value = (int)trim(file_get_contents($file));

            // Calculate slowdown
            if ($phpDebugger === 'no') {
                $slowdown = '0%';
                $slowdownPercent = 0.0;
            } else {
                $slowdownPercent = (($value - $baseValue) * 100) / $baseValue;
                $slowdown = sprintf('%.1f%%', $slowdownPercent);
            }

            // Format the value with thousands separators for markdown
            $formattedValue = number_format($value);

            // Calculate performance change compared to previous results
            $performanceChange = '--';
            $key = $command . '-' . $php . '-' . $phpDebugger;
            if (isset($previousResults[$key])) {
                $previousSlowdown = $previousResults[$key];
                $performanceChange = '0%';
                if ($previousSlowdown !== 0) {
                    $currentSlowdown = sprintf('%.1f', $slowdownPercent);
                    $changePercent = (($previousSlowdown - $currentSlowdown) * 100) / $previousSlowdown;
                    $performanceChange = sprintf('%+.1f%%', $changePercent);
                }
            }

            $output .= "| $phpDebugger | $formattedValue | $slowdown | $performanceChange |\n";

            // Add to CSV data (with raw numbers, not formatted)
            $csvData[] = [$command, $php, $phpDebugger, $value, sprintf('%.1f', $slowdownPercent)];
        }
    }
}

// Add performance summary section with aggregated data across all PHP versions
$output .= "\n# Performance Results Summary\n";
$output .= "\nThese tables show aggregated results across all PHP versions:\n";

// Aggregate data across all PHP versions for each command and Php-Debugger mode
foreach ($commands as $command) {
    $output .= "\n## **Command:** `$command`\n\n";
    $output .= "| Php-Debugger | Slowdown | Improvement |\n";
    $output .= "|--------------|---------:|------------:|\n";

    // Calculate aggregated values for each Php-Debugger mode
    $aggregatedData = [];
    foreach ($phpDebuggerModes as $phpDebugger) {
        $totalInstructions = 0;
        $totalBaseInstructions = 0;
        $count = 0;
        $totalPerformanceChange = 0;
        $performanceChangeCount = 0;

        foreach ($phpVersions as $php) {
            $file = "merged/php-{$php}_cmd-{$command}_php_debugger-{$phpDebugger}.txt";
            $baseFile = "merged/php-{$php}_cmd-{$command}_php_debugger-no.txt";

            if (file_exists($file) && file_exists($baseFile)) {
                $value = (int)trim(file_get_contents($file));
                $baseValue = (int)trim(file_get_contents($baseFile));

                $totalInstructions += $value;
                $totalBaseInstructions += $baseValue;
                $count++;

                $slowdownPercent = (($value - $baseValue) * 100) / $baseValue;

                // Calculate performance change if previous data exists
                $key = $command . '-' . $php . '-' . $phpDebugger;
                if (isset($previousResults[$key])) {
                    $previousSlowdown = $previousResults[$key];
                    $changePercent = 0;
                    if ($previousSlowdown !== 0) {
                        $currentSlowdown = sprintf('%.1f', $slowdownPercent);
                        $changePercent = (($previousSlowdown - $currentSlowdown) * 100) / $previousSlowdown;
                    }
                    $totalPerformanceChange += $changePercent;
                    $performanceChangeCount++;
                }
            }
        }

        if ($count > 0) {
            // Calculate average slowdown
            if ($phpDebugger === 'no') {
                $avgSlowdown = '0%';
            } else {
                $avgSlowdownPercent = (($totalInstructions - $totalBaseInstructions) * 100) / $totalBaseInstructions;
                $avgSlowdown = sprintf('%.1f%%', $avgSlowdownPercent);
            }

            // Calculate average performance change
            $performanceChange = '--';
            if ($performanceChangeCount > 0) {
                $avgPerformanceChange = $totalPerformanceChange / $performanceChangeCount;
                $performanceChange = sprintf('%+.1f%%', $avgPerformanceChange);
            }

            $output .= "| $phpDebugger | $avgSlowdown | $performanceChange |\n";
        }
    }
}

// Write the summary to file
file_put_contents('summary.md', $output);
echo "Summary generated successfully in summary.md\n";

// Write CSV file with metadata header
$csvFile = fopen('summary.csv', 'w');

// Add metadata rows at the top
fputcsv($csvFile, ['date', date('c')]); // ISO 8601 format

// For pull requests, GITHUB_HEAD_REF contains the source branch name
// For other events (workflow_dispatch, schedule), use GITHUB_REF_NAME which contains the current branch name
$branch = getenv('GITHUB_HEAD_REF') ?: getenv('GITHUB_REF_NAME') ?: 'unknown';
fputcsv($csvFile, ['branch', $branch]);

fputcsv($csvFile, ['commit', getenv('GITHUB_SHA') ?: 'unknown']);

// Add the benchmark data
foreach ($csvData as $row) {
    fputcsv($csvFile, $row);
}
fclose($csvFile);
echo "CSV data generated successfully in summary.csv\n";

/**
 * Makes a GitHub API request using curl
 *
 * @param string $url The URL to request
 * @return string|false The response body on success, false on failure
 */
function githubApiRequest(string $url): string|false {
    $token = getenv('GITHUB_TOKEN');
    if (!$token) {
        fwrite(STDERR, "Warning: GITHUB_TOKEN not set for API request\n");
        return false;
    }

    $ch = curl_init($url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
    curl_setopt($ch, CURLOPT_HTTPHEADER, [
        'User-Agent: GitHub-Actions',
        'Accept: application/vnd.github.v3+json',
        'Authorization: Bearer ' . $token
    ]);

    $response = curl_exec($ch);
    $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    curl_close($ch);

    if ($httpCode !== 200 || !$response) {
        fwrite(STDERR, "Warning: GitHub API request failed (HTTP $httpCode): $url\n");
        return false;
    }

    return $response;
}

/**
 * Fetches benchmark results from the latest successful run of the benchmark workflow on the base branch.
 *
 * @return array Array indexed by "command-php-php_debugger" with instruction counts, or empty array if not found
 */
function fetchPreviousBenchmarkResults(): array {
    // Determine the base branch for comparison
    // If COMPARISON_BRANCH is set, use it explicitly
    // Otherwise for pull requests, use GITHUB_BASE_REF
    // For other actions, use GITHUB_REF_NAME (current branch)
    $comparisonBranch = getenv('COMPARISON_BRANCH');
    if ($comparisonBranch !== false && $comparisonBranch !== '') {
        $baseBranch = $comparisonBranch;
    } else {
        $baseBranch = getenv('GITHUB_BASE_REF') ?: getenv('GITHUB_REF_NAME');
    }

    if (!$baseBranch) {
        fwrite(STDERR, "Warning: Could not determine base branch for comparison\n");
        return [];
    }

    $repo = getenv('GITHUB_REPOSITORY');
    if (!$repo) {
        fwrite(STDERR, "Warning: GITHUB_REPOSITORY not set, cannot fetch previous results\n");
        return [];
    }

    echo "Fetching previous benchmark results from branch '$baseBranch'...\n";

    // Get the latest successful run for the benchmark.yml workflow on the base branch
    $runsUrl = "https://api.github.com/repos/{$repo}/actions/workflows/benchmark.yml/runs?branch=" . urlencode($baseBranch) . "&status=success&per_page=1";

    $response = githubApiRequest($runsUrl);
    if ($response === false) {
        return [];
    }

    $runs = json_decode($response, true);

    if (empty($runs['workflow_runs'])) {
        fwrite(STDERR, "Warning: No successful runs found for branch '$baseBranch'\n");
        return [];
    }

    $latestRun = $runs['workflow_runs'][0];

    // Get artifacts for this run
    $artifactsUrl = $latestRun['artifacts_url'];

    $response = githubApiRequest($artifactsUrl);
    if ($response === false) {
        return [];
    }

    $artifacts = json_decode($response, true);

    // Find the artifact containing summary.csv from the performance job
    $summaryArtifact = null;
    foreach ($artifacts['artifacts'] as $artifact) {
        if (strpos($artifact['name'], 'summary') !== false) {
            $summaryArtifact = $artifact;
            break;
        }
    }

    if (!$summaryArtifact) {
        fwrite(STDERR, "Warning: No summary artifact found\n");
        return [];
    }

    // Download the artifact
    $downloadUrl = $summaryArtifact['archive_download_url'];

    $zipContent = githubApiRequest($downloadUrl);
    if ($zipContent === false) {
        return [];
    }

    // Save the zip file temporarily
    $tempZip = tempnam(sys_get_temp_dir(), 'artifact_') . '.zip';
    file_put_contents($tempZip, $zipContent);

    // Extract the zip file
    $zip = new ZipArchive();
    if ($zip->open($tempZip) !== true) {
        fwrite(STDERR, "Warning: Failed to open artifact zip\n");
        unlink($tempZip);
        return [];
    }

    $csvContent = $zip->getFromName('summary.csv');
    $zip->close();
    unlink($tempZip);

    if ($csvContent === false) {
        fwrite(STDERR, "Warning: summary.csv not found in artifact\n");
        return [];
    }

    // Parse the CSV content
    $lines = explode("\n", $csvContent);
    $previousData = [];

    foreach ($lines as $line) {
        $line = trim($line);
        if (empty($line)) {
            continue;
        }

        $fields = str_getcsv($line);

        // Skip metadata rows (date, branch, commit) and header row
        if (count($fields) === 2 && in_array($fields[0], ['date', 'branch', 'commit'])) {
            continue;
        }
        if ($fields[0] === 'command') {
            continue;
        }

        if (count($fields) >= 4) {
            // Fields: command, php, php_debugger_mode, instructions, slowdown
            $key = $fields[0] . '-' . $fields[1] . '-' . $fields[2];
            $previousData[$key] = (int)$fields[4];
        }
    }

    echo "Loaded previous benchmark results from branch '$baseBranch'\n";
    return $previousData;
}

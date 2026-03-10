<?php
/**
 * PHP Debugger Installer
 *
 * Usage: php -r "copy('https://raw.githubusercontent.com/pronskiy/php-debugger-src/main/install.php','/tmp/i.php');" && php /tmp/i.php
 * Or:    curl -fsSL https://raw.githubusercontent.com/pronskiy/php-debugger-src/main/install.php | php
 */

if (PHP_SAPI !== 'cli') {
    fwrite(STDERR, "Error: run this from the command line\n");
    exit(1);
}

$repo    = 'pronskiy/php-debugger-src';
$extName = 'php_debugger';
$phpVer  = PHP_MAJOR_VERSION . '.' . PHP_MINOR_VERSION;
$os      = PHP_OS_FAMILY === 'Darwin' ? 'macos' : (PHP_OS_FAMILY === 'Linux' ? 'ubuntu' : null);
$arch    = php_uname('m');
$extDir  = ini_get('extension_dir');
$extFile = $extName . '.so';

echo "PHP Debugger Installer\n";
echo "======================\n\n";
echo "PHP version:   {$phpVer} (" . PHP_VERSION . ")\n";
echo "Platform:      {$os}-{$arch}\n";
echo "Extension dir: {$extDir}\n\n";

if ($os === null) {
    fwrite(STDERR, "Error: unsupported platform (" . PHP_OS_FAMILY . "). Pre-built binaries available for Linux and macOS.\n");
    exit(1);
}

if (version_compare(PHP_VERSION, '8.2.0', '<') || version_compare(PHP_VERSION, '8.6.0', '>=')) {
    fwrite(STDERR, "Error: PHP {$phpVer} is not supported. Requires PHP 8.2–8.5.\n");
    exit(1);
}

if (extension_loaded('php_debugger') || extension_loaded('xdebug')) {
    echo "\u26a0\ufe0f  Extension already loaded. Reinstalling...\n\n";
}

// Download
$artifact = "php-debugger-php{$phpVer}-{$os}-{$arch}";
$url = "https://github.com/{$repo}/releases/latest/download/{$artifact}.so";

echo "Downloading {$url}...\n";

$ctx = stream_context_create(['http' => [
    'follow_location' => true,
    'header' => "User-Agent: php-debugger-installer\r\n",
]]);

$binary = @file_get_contents($url, false, $ctx);

if ($binary === false) {
    fwrite(STDERR, "\nError: could not download binary for PHP {$phpVer} on {$os}-{$arch}\n");
    fwrite(STDERR, "Check available releases: https://github.com/{$repo}/releases\n");
    exit(1);
}

echo sprintf("Downloaded %s (%.1f KB)\n\n", $artifact, strlen($binary) / 1024);

// Install
$dest = $extDir . DIRECTORY_SEPARATOR . $extFile;

if (@file_put_contents($dest, $binary) === false) {
    // Try with a temp file and sudo
    $tmp = tempnam(sys_get_temp_dir(), 'php_debugger_');
    file_put_contents($tmp, $binary);
    chmod($tmp, 0755);

    echo "Extension dir not writable, trying sudo...\n";
    $ret = 0;
    passthru("sudo cp " . escapeshellarg($tmp) . " " . escapeshellarg($dest), $ret);
    passthru("sudo chmod 755 " . escapeshellarg($dest));
    unlink($tmp);

    if ($ret !== 0) {
        fwrite(STDERR, "Error: could not install to {$dest}\n");
        exit(1);
    }
} else {
    chmod($dest, 0755);
}

echo "Installed {$extFile} to {$extDir}/\n\n";

// Check ini
$iniPath = php_ini_loaded_file();
$scanDir = php_ini_scanned_files() ? dirname(explode(',', php_ini_scanned_files())[0]) : null;

echo "Add to your php.ini:\n\n";
echo "    zend_extension={$extFile}\n\n";

if ($iniPath) {
    echo "Loaded ini: {$iniPath}\n";
}
if ($scanDir) {
    echo "Or create:  {$scanDir}/99-php_debugger.ini\n";
}

echo "\nDone! Verify with: php -m | grep -i debugger\n";

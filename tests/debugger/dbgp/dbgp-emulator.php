<?php
require_once __DIR__ . '/dbgpclient.php';

class DbgpEmulator extends DebugClient
{
	protected $commands = [];
	protected $conversation = [];

	public function __construct()
	{
		parent::__construct();
		$this->setPort(9003);
	}

	public function loadCommands($filename)
	{
		if (!file_exists($filename)) {
			echo "Command file not found: {$filename}\n";
			return false;
		}

		$content = file_get_contents($filename);
		$this->commands = array_filter(
			array_map('trim', explode("\n", $content)),
			function($line) {
				return !empty($line) && !str_starts_with($line, '#');
			}
		);

		return true;
	}

	// Override doRead to capture conversation
	function doRead($conn, ?string $transaction_id = null)
	{
		ob_start();
		$result = parent::doRead($conn, $transaction_id);
		$output = ob_get_clean();

		if (!empty($output)) {
			$this->conversation[] = "<- " . trim($output);
		}

		return $result;
	}

	// Override sendCommand to capture conversation
	function sendCommand($conn, $command, $transaction_id)
	{
		ob_start();
		parent::sendCommand($conn, $command, $transaction_id);
		$output = ob_get_clean();

		if (!empty($output)) {
			$this->conversation[] = trim($output);
		}
	}

	public function runEmulator()
	{
		// Open socket and wait for connection (but don't launch PHP)
		$errno = null;
		$errstr = null;
		$this->socket = $this->open($errno, $errstr);

		if ($this->socket === false) {
			echo "Could not create socket server - already in use?\n";
			echo "Error: {$errstr}, errno: {$errno}\n";
			echo "Address: {$this->getAddress()}\n";
			return false;
		}

		$conn = $this->acceptConnection(-1);

		if ($conn === false) {
			$this->conversation[] = "ERROR: Failed to accept connection";
			fclose($this->socket);
			return false;
		}

		$this->conversation[] = "Connection accepted!";

		// Read init packet and extract PID
		$this->doRead($conn);
		$lastMessage = end($this->conversation);
		if (preg_match('@appid="(\d+)"@', $lastMessage, $matches)) {
			$this->pid = (int)$matches[1];
		}

		// Send commands
		$i = 1;
		foreach ($this->commands as $command) {
			$this->sendCommand($conn, $command, $i);
			$response = $this->doRead($conn, (string)$i);

			// Check if the debugger has stopped
			if ($response === null || feof($conn)) {
				$this->conversation[] = "Connection closed by debugger";
				break;
			}

			$i++;
		}

		// Clean up
		$this->closeConnection($conn);

		$this->printConversation();

		return true;
	}

	function printConversation()
	{
		echo "\n";
		echo "=== Full DBGP Conversation ===\n";
		echo "==============================\n\n";
		foreach ($this->conversation as $line) {
			echo $line, "\n";
		}
		echo "\n==============================\n";
	}
}

// Main execution
if (php_sapi_name() !== 'cli') {
	die("This script must be run from the command line\n");
}

if ($argc < 2) {
	die("Error: Command file argument is required\n");
}

$commandFile = $argv[1];

$daemon = new DbgpEmulator();

if (!$daemon->loadCommands($commandFile)) {
	exit(1);
}

echo "Starting DBGP emulator on port 9003...\n";
echo "Waiting for debugger connection...\n";

$daemon->runEmulator();
?>

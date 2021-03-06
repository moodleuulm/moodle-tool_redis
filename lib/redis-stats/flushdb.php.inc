<?php

/* KIZ MODIFICATION START
   REASON: Redis Stats must not use any custom session handling. */
require(__DIR__ . '/../../../../../config.php');
if (!isset($_GET['id']) || isset($_GET['id']) && $_GET['id'] != sesskey())
{
    die("Invalid request.");
}
/* KIZ MODIFICATION END */
/* ORIGINAL START
session_start();

if (!isset($_GET['id']) || isset($_GET['id']) && $_GET['id'] != $_SESSION['id'])
{
	$_SESSION['id'] = '';
	die("Invalid request.");
}
   ORIGINAL END */

if (file_exists(dirname(__FILE__)."/config.php"))
{
    require_once dirname(__FILE__).'/config.php';
}
if (!$servers)
{
	die("No servers in config found.");
}

// Default debug setting
if (!defined('DEBUG'))
{
	define("DEBUG", false);
}

// Process GET request
$server = 0;
if (isset($_GET['s']) && intval($_GET['s']) < count($servers))
{
	$server = intval($_GET['s']);
}
$serverName = $servers[$server][0];
if (isset($_GET['db']))
{
	$db = intval($_GET['db']);
}
if (isset($_GET['async']))
{
	$async = intval($_GET['async']);
}

// Command mapping
$FLUSHDB = 'FLUSHDB';
if (isset($command[$serverName]['FLUSHDB']) && !is_null($command[$serverName]['FLUSHDB']) && !empty($command[$serverName]['FLUSHDB']))
{
	$FLUSHDB = $command[$serverName]['FLUSHDB'];
}
$FLUSHALL = 'FLUSHALL';
if (isset($command[$serverName]['FLUSHALL']) && !is_null($command[$serverName]['FLUSHALL']) && !empty($command[$serverName]['FLUSHALL']))
{
	$FLUSHALL = $command[$serverName]['FLUSHALL'];
}
$AUTH = 'AUTH';
if (isset($command[$serverName]['AUTH']) && !is_null($command[$serverName]['AUTH']) && !empty($command[$serverName]['AUTH']))
{
	$AUTH = $command[$serverName]['AUTH'];
}

// Talk to Redis server
$error = null;

$fp = @fsockopen($servers[$server][1], $servers[$server][2], $errno, $errstr, 30);

$info = array();

if (!$fp)
{
	die($errstr);
}
else
{
	$redisCommand = '';
	$ASYNC        = '';

	isset($servers[$server][3]) ? $pwdEntry = $servers[$server][3] : $pwdEntry = null;
	if (!is_null($pwdEntry) && !empty($pwdEntry))
	{
		if (is_array($pwdEntry))
		{
			if (!isset($pwdEntry[1]) || is_null($pwdEntry[1]) || empty($pwdEntry[1]))
			{
				$pwdEntry[1] = '0';
			}
			$credentials = "$pwdEntry[0] $pwdEntry[1]";
		}
		else
		{
			$credentials = $pwdEntry;
		}
		$redisCommand = "$AUTH $credentials\r\n";
	}
	if ($async) // we want async flush
	{
		$ASYNC = ' ASYNC';
	}
	if ($db != -1) // one specific database
	{
		$redisCommand .= "SELECT $db\r\n$FLUSHDB$ASYNC\r\nQUIT\r\n";
	}
	else // entire instance
	{
		$redisCommand .= "$FLUSHALL$ASYNC\r\nQUIT\r\n";
	}

	fwrite($fp, $redisCommand);
	while (!feof($fp))
	{
		$info[] = trim(fgets($fp));
	}
	fclose($fp);
}

if (array_unique($info) === array('+OK'))
{
/* KIZ MODIFICATION START
   REASON: Redis Stats must not use any custom session handling. */
/* ORIGINAL START
	$_SESSION['id'] = '';
   ORIGINAL END */
	echo "Success";
}
else
{
	if (DEBUG === true)
	{
		var_dump($redisCommand);
		var_dump($info);
	}
	foreach ($info as $v)
	{
		if ($v != '+OK')
			die($v);
	}
}

<?php
/**
 * Created by PhpStorm.
 * User: Jenner
 * Date: 2016/7/25
 * Time: 11:48
 */

$process = new SimpleFork\Process(null, "process_name");
var_dump($process);
var_dump($process->name());
$process->name("test");
var_dump($process->name());
var_dump($process->updateStatus());
var_dump($process->isRunning());
var_dump($process->isStopped());
var_dump($process->isStarted());
var_dump($process->errno());
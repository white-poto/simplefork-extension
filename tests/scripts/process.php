<?php
/**
 * Created by PhpStorm.
 * User: Jenner
 * Date: 2016/7/25
 * Time: 11:48
 */

$process = new SimpleFork\Process(null, "process_name");
var_dump($process->name());
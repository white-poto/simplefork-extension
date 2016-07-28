<?php
/**
 * Created by PhpStorm.
 * User: Jenner
 * Date: 2016/7/25
 * Time: 11:48
 */

$process = new SimpleFork\Process(function() {
    echo "sub_process", PHP_EOL;
    sleep(20);
}, "process_name");
//var_dump($process);
//var_dump($process->name());
//$process->name("test");
//var_dump($process->name());
//var_dump($process->updateStatus());
//var_dump($process->isRunning());
//var_dump($process->isStopped());
//var_dump($process->isStarted());
//var_dump($process->errorNo());
//var_dump($process->errmsg());
//var_dump($process->ifSignal());
//var_dump($process->start());
$process->start();
for($i=0; $i<2; $i++) {
    echo "wait1", PHP_EOL;
    $process->wait(false);
    echo "wait2, ", $i, PHP_EOL;
}
$process->wait();

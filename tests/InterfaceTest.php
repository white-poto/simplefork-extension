<?php

/**
 * Created by PhpStorm.
 * User: Jenner
 * Date: 2015/9/9
 * Time: 11:58
 */



class InterfaceTest extends TestSuite
{

    public function testExists()
    {
        $this->assertTrue(interface_exists('SimpleFork\\CacheInterface'));
        $this->assertTrue(interface_exists('SimpleFork\\QueueInterface'));
        $this->assertTrue(interface_exists('SimpleFork\\LockInterface'));
        $this->assertTrue(interface_exists('SimpleFork\\Runnable'));
    }

    public function testCache()
    {
        $cache = new SharedMemory();
        $reflect = new ReflectionObject($cache);
        $this->assertTrue($reflect->hasMethod('get'));
        $this->assertTrue($reflect->hasMethod('set'));
        $this->assertTrue($reflect->hasMethod('has'));
        $this->assertTrue($reflect->hasMethod('delete'));
        $this->assertTrue($reflect->hasMethod('flush'));
    }

    public function testQueue()
    {
        $queue = new Queue();
        $reflect = new ReflectionObject($queue);
        $this->assertTrue($reflect->hasMethod('put'));
        $this->assertTrue($reflect->hasMethod('get'));
        $this->assertTrue($reflect->hasMethod('size'));
        $this->assertTrue($reflect->hasMethod('remove'));
    }

    public function testLock(){
        $lock = new Lock();
        $reflect = new ReflectionObject($lock);
        $this->assertTrue($reflect->hasMethod('acquire'));
        $this->assertTrue($reflect->hasMethod('release'));
    }

    public function testRunnable(){
        $run = new Run();
        $reflect = new ReflectionObject($run);
        $this->assertTrue($reflect->hasMethod('run'));
    }

}


class SharedMemory implements SimpleFork\CacheInterface
{
    /**
     * get var
     * @param $key
     * @param null $default
     * @return bool|mixed
     */
    public function get($key, $default = null)
    {

    }

    /**
     * set var
     * @param $key
     * @param null $value
     * @return
     */
    public function set($key, $value)
    {

    }

    /**
     * has var ?
     * @param $key
     * @return bool
     */
    public function has($key)
    {

    }

    /**
     * delete var
     * @param $key
     * @return bool
     */
    public function delete($key)
    {

    }

    public function flush()
    {

    }
}


class Queue implements SimpleFork\QueueInterface
{
    /**
     * put value into the queue of channel
     * @param $channel
     * @param $value
     * @return mixed
     */
    public function put($channel, $value)
    {

    }

    /**
     * get value from the queue of channel
     * @param $channel
     * @return mixed
     */
    public function get($channel)
    {

    }

    /**
     * get the size of the queue of channel
     * @param $channel
     * @return mixed
     */
    public function size($channel)
    {

    }

    /**
     * remove the queue resource
     * @return mixed
     */
    public function remove()
    {

    }
}

class Lock implements SimpleFork\LockInterface {
    /**
     * get a lock
     * @return mixed
     */
    public function acquire(){

    }

    /**
     * release lock
     * @return mixed
     */
    public function release(){

    }
}

class Run implements SimpleFork\Runnable {

    public function run(){

    }
}
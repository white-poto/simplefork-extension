<?php
/**
 * Created by PhpStorm.
 * User: Jenner
 * Date: 2015/9/10
 * Time: 19:07
 */

namespace SimpleFork;


interface QueueInterface
{
    /**
     * put value into the queue of channel
     * @param $channel
     * @param $value
     * @return mixed
     */
    public function put($channel, $value);

    /**
     * get value from the queue of channel
     * @param $channel
     * @return mixed
     */
    public function get($channel);

    /**
     * get the size of the queue of channel
     * @param $channel
     * @return mixed
     */
    public function size($channel);

    /**
     * remove the queue resource
     * @return mixed
     */
    public function remove();
}
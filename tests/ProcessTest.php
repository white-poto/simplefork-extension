<?php

/**
 * Created by PhpStorm.
 * User: Jenner
 * Date: 2015/9/9
 * Time: 14:28
 */
class ProcessTest extends TestSuite
{
    public function setUp(){

    }

    public function testProperties(){
        $process = new SimpleFork\Process();
        $reflect = new ReflectionObject($process);
        $this->assertTrue($reflect->hasConstant("BEFORE_START"));
        $this->assertTrue($reflect->hasConstant("BEFORE_EXIT"));
    }



}
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
        $reflect = new ReflectionClass("SimpleFork\\Process");
        print_r($reflect->getMethods());
        $this->assertTrue($reflect->hasConstant("BEFORE_START"));
        $this->assertTrue($reflect->hasConstant("BEFORE_EXIT"));
    }



}
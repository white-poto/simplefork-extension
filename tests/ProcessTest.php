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
        $this->assertTrue($reflect->hasMethod("__destruct"));
        $this->assertTrue($reflect->hasMethod("__construct"));
        $this->assertTrue($reflect->hasConstant("BEFORE_START"));
        $this->assertTrue($reflect->hasConstant("BEFORE_EXIT"));
    }

    public function testConstruct(){
        try{
            $process = new SimpleFork\Process("test");
        }catch (Exception $e){
            $this->assertEquals(0, $e->getCode());
            $this->assertEquals("execution param must be callable", $e->getMessage());
        }

    }



}
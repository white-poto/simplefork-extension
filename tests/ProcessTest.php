<?php

/**
 * Created by PhpStorm.
 * User: Jenner
 * Date: 2015/9/9
 * Time: 14:28
 */
class ProcessTest extends TestSuite
{
    public function setUp()
    {

    }

    public function testProperties()
    {
        $reflect = new ReflectionClass("SimpleFork\\Process");
        $this->assertTrue($reflect->hasMethod("__destruct"));
        $this->assertTrue($reflect->hasMethod("__construct"));
        $this->assertTrue($reflect->hasMethod("setCache"));
        $this->assertTrue($reflect->hasMethod("setQueue"));
        $this->assertTrue($reflect->hasMethod("getPid"));
        $this->assertTrue($reflect->hasMethod("isAlive"));
        $this->assertTrue($reflect->hasMethod("exitCode"));
        $this->assertTrue($reflect->hasMethod("on"));
        $this->assertTrue($reflect->hasMethod("start"));
        $this->assertTrue($reflect->hasConstant("BEFORE_START"));
        $this->assertTrue($reflect->hasConstant("BEFORE_EXIT"));
    }

    public function testConstruct()
    {
        try {
            $process = new SimpleFork\Process("test");
        } catch (Exception $e) {
            $this->assertEquals(0, $e->getCode());
            $this->assertEquals("execution param must be callable", $e->getMessage());
        }
        $this->assertTrue(isset($e));

        try {
            $process_2 = new SimpleFork\Process(function () {

            });
        } catch (Exception $e_2) {

        }
        $this->assertTrue(!isset($e_2));
    }

    public function testIsAlive()
    {
        $process = new SimpleFork\Process();
        $this->assertFalse($process->isAlive());
    }

    public function testOn(){
        $process = new SimpleFork\Process();
        try{
            $process->on(SimpleFork\Process::BEFORE_START, 'false');
            $process->on(SimpleFork\Process::BEFORE_EXIT, 'false');
        }catch (Exception $e){
            $this->assertEquals($e->getMessage(), "try to register a function that it is not callable");
        }
        $this->assertTrue(isset($e));
    }

    public function testOn2(){
        $process = new SimpleFork\Process();
        try{
            $process->on(SimpleFork\Process::BEFORE_START, function(){

            });
            $process->on(SimpleFork\Process::BEFORE_EXIT, function(){

            });
        }catch (Exception $e){
            $this->assertTrue(false);
        }
        $this->assertTrue(!isset($e));
    }

    public function testWait(){
        $process = new SimpleFork\Process();
        try{
            $process->wait();
        }catch (Exception $e){
            $this->assertEquals($e->getMessage(), "Process is not running because it has no pid");
        }
    }
}



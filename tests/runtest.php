<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2015/9/14
 * Time: 16:43
 */

require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . "TestSuite.php");
require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . "InterfaceTest.php");
require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . "ProcessTest.php");

TestSuite::run("InterfaceTest");
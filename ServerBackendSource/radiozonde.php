<?php
	$hostname="";
	$username = "";
	$passwd = "";
	$dbname = "";
	$connect = mysql_connect($hostname,$username,$passwd)or die("Failed");
	$result = mysql_select_db($dbname,$connect);

	mysql_query("set names utf8");

	$id = $_POST["id"];
	$data = $_POST["data"];

	$query = "INSERT INTO radiozonde (id,data) VALUES ('".$id."', '".$data."')";
	mysql_query($query,$connect);
	mysql_close($connect);
?>

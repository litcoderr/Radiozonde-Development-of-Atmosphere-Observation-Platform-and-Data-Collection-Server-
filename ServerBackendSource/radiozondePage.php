<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="utf-8">
</head>

<body>
	<?php
	$hostname="";
    	$username = "";
    	$passwd = "";
    	$dbname = "";
    	$connect = mysql_connect($hostname,$username,$passwd)or die("Failed");
    	$result = mysql_select_db($dbname,$connect);

    	mysql_query("set names utf8");
    	$sql = "SELECT * FROM radiozonde";
    	$rs = mysql_query($sql,$connect);

    	$data = array();
    	$i = 0;
    	$finalI = 0;
    	while($info=mysql_fetch_array($rs)){
	        $data[$i][0]=$info["id"];
	        $data[$i][1]=$info["data"];
	        $finalI = $i;
	        $i++;
    	}
	?>
	<ul>
		<?php
			for($j=0;$j<=$finalI;$j++){
				echo "<li>id: ".$data[$j][0]." data: ".$data[$j][1]."</li>";
			}
		?>
	</ul>
</body>

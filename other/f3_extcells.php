<?php

function import($file,$dlc)
{
	global $db;
	$f=file_get_contents($file);
	if($f===FALSE)
		die("Error loading $file");
	$c=explode("\n",$f);
	createTable("exteriors");
	foreach($c as $t)
	{
		list($name,$id,$x,$y,$wrld)=explode("|",$t);
		
		$id=hexdec($id);
		$wrld=hexdec($wrld);
if ($id == 0)
continue;
		$name=trim($name);
                $prep = $db->prepare("insert into exteriors (baseID,x,y,wrld,dlc) values (?, ?, ?, ?, ?)");
                if ($prep === FALSE) {
			echo "Fail: " . $id;
			continue;
		}
		$r = $prep->execute(array($id, $x, $y, $wrld, $dlc));
if (!$r) {
$arr = $prep->errorInfo();
echo $arr[2];
}
	}
}


function createTable($tb)
{
	global $db;
	$db->exec("CREATE TABLE $tb (baseID integer,x integer,y integer,wrld integer,dlc integer)");
}

$db = new PDO('sqlite:fallout3.sqlite');
if(!$db)
{
    die("Errore Sqlite");
}

import("data3/base_cells_list.txt",0);
import("data3/tp_cells_list.txt",1);
import("data3/oa_cells_list.txt",2);
import("data3/bs_cells_list.txt",3);
import("data3/pl_cells_list.txt",4);
import("data3/mz_cells_list.txt",5);

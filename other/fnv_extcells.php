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
                $prep = $db->prepare("insert into exteriors (baseID,name,x,y,wrld,dlc) values (?, ?, ?, ?, ?, ?)");
                if ($prep === FALSE) {
			echo "Fail: " . $id;
			continue;
		}
		$r = $prep->execute(array($id, $name, $x, $y, $wrld, $dlc));
if (!$r) {
$arr = $prep->errorInfo();
echo $arr[2];
}
	}
}


function createTable($tb)
{
	global $db;
	$db->exec("CREATE TABLE $tb (baseID integer(11),name varchar(128),x integer(11),y integer(11),wrld integer(11),dlc integer(11))");
}

$db = new PDO('sqlite:newvegas.sqlite');
if(!$db)
{
    die("Errore Sqlite");
}

import("data/base_cells_list.txt",0);
import("data/dm_cells_list.txt",1);
import("data/hh_cells_list.txt",2);
import("data/owb_cells_list.txt",3);
import("data/lr_cells_list.txt",4);

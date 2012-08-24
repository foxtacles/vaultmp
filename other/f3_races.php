<?php

function import($file,$dlc)
{
	global $db;
	$f=file_get_contents($file);
	if($f===FALSE)
		die("Error loading $file");
	$c=explode("\n",$f);
	createTable("races");
	foreach($c as $t)
	{
		list($id,$name,$child,$younger,$older)=explode("|",$t);
		
		$id=hexdec($id);
		$younger=hexdec($younger);
		$older=hexdec($older);

if ($id == 0)
continue;

                $prep = $db->prepare("insert into races (baseID,child,younger,older,dlc) values (?, ?, ?, ?, ?)");

                if ($prep === FALSE) {
			echo "Fail: " . $id;
			continue;
		}
		$r = $prep->execute(array($id, $child,$younger, $older, $dlc));
if (!$r) {
$arr = $prep->errorInfo();
echo $arr[2];
}

	}
}


function createTable($tb)
{
	global $db;
	$db->exec("CREATE TABLE $tb (baseID integer, child integer, younger integer, older integer, dlc integer)");
}


$db = new PDO('sqlite:fallout3.sqlite');
if(!$db)
{
    die("Errore Sqlite");
}

import("data3/f3_main_races.txt",0);
import("data3/f3_tp_races.txt",1);
import("data3/f3_oa_races.txt",2);
import("data3/f3_bsn_races.txt",3);
import("data3/f3_pl_races.txt",4);
import("data3/f3_mz_races.txt",5);

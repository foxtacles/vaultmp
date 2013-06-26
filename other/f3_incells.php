<?php

function import($file,$dlc)
{
	global $db;
	$f=file_get_contents($file);
	if($f===FALSE)
		die("Error loading $file");
	$c=explode("\n",$f);
	createTable("interiors");
	foreach($c as $t)
	{
		list($id,$x1,$y1,$z1,$x2,$y2,$z2)=explode("|",$t);

if ($id == 0)
continue;

                $prep = $db->prepare("insert into interiors (baseID,x1,y1,z1,x2,y2,z2) values (?, ?, ?, ?, ?, ?, ?)");
                if ($prep === FALSE) {
			echo "Fail: " . $id;
			continue;
		}
		$r = $prep->execute(array($id, $x1,$y1,$z1,$x2,$y2,$z2, $dlc));
if (!$r) {
$arr = $prep->errorInfo();
echo $arr[2];
}
	}
}


function createTable($tb)
{
	global $db;
	$db->exec("CREATE TABLE $tb (baseID integer,x1 integer, y1 integer, z1 integer, x2 integer, y2 integer, z2 integer,dlc integer)");
}

$db = new PDO('sqlite:fallout3.sqlite');
if(!$db)
{
    die("Errore Sqlite");
}

import("data3/f3_main_cellbounds.txt",0);
import("data3/f3_tp_cellbounds.txt",1);
import("data3/f3_oa_cellbounds.txt",2);
import("data3/f3_bsn_cellbounds.txt",3);
import("data3/f3_pl_cellbounds.txt",4);
import("data3/f3_mz_cellbounds.txt",5);

<?php

function import($file,$dlc)
{
	global $db;
	$f=file_get_contents($file);
	if($f===FALSE)
		die("Error loading $file");
	$c=explode("\n",$f);
	createTable("npcitems");
	foreach($c as $t)
	{
		list($id,$item,$count,$condition)=explode("|",$t);
		
		$id=hexdec($id);
		$item=hexdec($item);

if ($id == 0)
continue;

                $prep = $db->prepare("insert into npcitems (baseID,item,count,condition,dlc) values (?, ?, ?, ?, ?)");

                if ($prep === FALSE) {
			echo "Fail: " . $id;
			continue;
		}
		$r = $prep->execute(array($id, $item, $count, $condition, $dlc));
if (!$r) {
$arr = $prep->errorInfo();
echo $arr[2];
}

	}
}


function createTable($tb)
{
	global $db;
	$db->exec("CREATE TABLE $tb (baseID integer, item integer, count integer, condition float, dlc integer)");
}


$db = new PDO('sqlite:fallout3.sqlite');
if(!$db)
{
    die("Errore Sqlite");
}

import("data3/f3_main_npc_items.txt",0);
import("data3/f3_tp_npc_items.txt",1);
import("data3/f3_oa_npc_items.txt",2);
import("data3/f3_bsn_npc_items.txt",3);
import("data3/f3_pl_npc_items.txt",4);
import("data3/f3_mz_npc_items.txt",5);

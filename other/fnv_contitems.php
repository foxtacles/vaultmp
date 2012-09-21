<?php

function import($file,$dlc)
{
	global $db;
	$f=file_get_contents($file);
	if($f===FALSE)
		die("Error loading $file");
	$c=explode("\n",$f);
	createTable("contitems");
	foreach($c as $t)
	{
		list($id,$item,$count,$type,$condition)=explode("|",$t);
		
		$id=hexdec($id);
		$item=hexdec($item);

if ($id == 0)
continue;

                $prep = $db->prepare("insert into contitems (baseID,item,count,condition,dlc) values (?, ?, ?, ?, ?)");

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


$db = new PDO('sqlite:newvegas.sqlite');
if(!$db)
{
    die("Errore Sqlite");
}

import("data/fnv_main_containerobjects.txt",0);
import("data/fnv_dm_containerobjects.txt",1);
import("data/fnv_hh_containerobjects.txt",2);
import("data/fnv_ow_containerobjects.txt",3);
import("data/fnv_lr_containerobjects.txt",4);
import("data/fnv_GunRunnersArsenal_containerobjects.txt",5);

import("data/fnv_ClassicPack_containerobjects.txt",6);
import("data/fnv_CaravanPack_containerobjects.txt",7);
import("data/fnv_TribalPack_containerobjects.txt",8);
import("data/fnv_MercenaryPack_containerobjects.txt",9);

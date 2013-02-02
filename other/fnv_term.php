<?php

function import($file,$dlc)
{
	global $db;
	$f=file_get_contents($file);
	if($f===FALSE)
		die("Error loading $file");
	$c=explode("\n",$f);
	createTable("terminals");
	foreach($c as $t)
	{
		list($t,$id,$lock,$note)=explode("|",$t);
		
		$id=hexdec($id);
		$note=hexdec($note);

if ($id == 0)
continue;

                $prep = $db->prepare("insert into terminals (baseID,lock,note,dlc) values (?, ?, ?, ?)");

                if ($prep === FALSE) {
			echo "Fail: " . $id;
			continue;
		}
		$r = $prep->execute(array($id, $lock, $note, $dlc));
if (!$r) {
$arr = $prep->errorInfo();
echo $arr[2];
}

	}
}


function createTable($tb)
{
	global $db;
	$db->exec("CREATE TABLE $tb (baseID integer, lock integer, note integer, dlc integer)");
}


$db = new PDO('sqlite:newvegas.sqlite');
if(!$db)
{
    die("Errore Sqlite");
}


import("data/fnv_main_TERM.txt",0);
import("data/fnv_dm_TERM.txt",1);
import("data/fnv_hh_TERM.txt",2);
import("data/fnv_ow_TERM.txt",3);
import("data/fnv_lr_TERM.txt",4);
import("data/fnv_GunRunnersArsenal_TERM.txt",5);

import("data/fnv_ClassicPack_TERM.txt",6);
import("data/fnv_CaravanPack_TERM.txt",7);
import("data/fnv_TribalPack_TERM.txt",8);
import("data/fnv_MercenaryPack_TERM.txt",9);

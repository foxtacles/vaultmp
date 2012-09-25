<?php

function import($file,$dlc)
{
	global $db;
	$f=file_get_contents($file);
	if($f===FALSE)
		die("Error loading $file");
	$c=explode("\n",$f);
	createTable("npcs");
	foreach($c as $t)
	{
		list($id,$female,$race,$essential,$deathitem,$template,$flags)=explode("|",$t);
		
		$id=hexdec($id);
		$race=hexdec($race);
		$deathitem=hexdec($deathitem);
                $template=hexdec($template);
                $flags=hexdec($flags);

if ($id == 0)
continue;

                $prep = $db->prepare("insert into npcs (baseID,essential,female,race,template,flags,deathitem,dlc) values (?, ?, ?, ?, ?, ?, ?, ?)");

                if ($prep === FALSE) {
			echo "Fail: " . $id;
			continue;
		}
		$r = $prep->execute(array($id, $essential,$female,$race,$template,$flags,$deathitem,$dlc));
if (!$r) {
$arr = $prep->errorInfo();
echo $arr[2];
}

	}
}


function createTable($tb)
{
	global $db;
	$db->exec("CREATE TABLE $tb (baseID integer, essential integer, female integer, race integer, template integer, flags integer, deathitem integer, dlc integer)");
}

$db = new PDO('sqlite:newvegas.sqlite');
if(!$db)
{
    die("Errore Sqlite");
}

import("data/fnv_main_npc.txt",0);
import("data/fnv_dm_npc.txt",1);
import("data/fnv_hh_npc.txt",2);
import("data/fnv_ow_npc.txt",3);
import("data/fnv_lr_npc.txt",4);
import("data/fnv_GunRunnersArsenal_npc.txt",5);

import("data/fnv_ClassicPack_npc.txt",6);
import("data/fnv_CaravanPack_npc.txt",7);
import("data/fnv_TribalPack_npc.txt",8);
import("data/fnv_MercenaryPack_npc.txt",9);

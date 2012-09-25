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


$db = new PDO('sqlite:fallout3.sqlite');
if(!$db)
{
    die("Errore Sqlite");
}

import("data3/f3_main_npc.txt",0);
import("data3/f3_tp_npc.txt",1);
import("data3/f3_oa_npc.txt",2);
import("data3/f3_bsn_npc.txt",3);
import("data3/f3_pl_npc.txt",4);
import("data3/f3_mz_npc.txt",5);

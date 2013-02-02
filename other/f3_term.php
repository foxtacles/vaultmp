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


$db = new PDO('sqlite:fallout3.sqlite');
if(!$db)
{
    die("Errore Sqlite");
}

import("data3/f3_main_TERM.txt",0);
import("data3/f3_tp_TERM.txt",1);
import("data3/f3_oa_TERM.txt",2);
import("data3/f3_bsn_TERM.txt",3);
import("data3/f3_pl_TERM.txt",4);
import("data3/f3_mz_TERM.txt",5);

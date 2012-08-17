<?php

function import($file,$dlc)
{
	global $db;
	$f=file_get_contents($file);
	if($f===FALSE)
		die("Error loading $file");
	$c=explode("\n",$f);
	createTable("weapons");
	createItems("items");
	foreach($c as $t)
	{
		list($id,$value,$hp,$weight,$dmg,$reload,$rate,$automatic,$slot,$ammo)=explode("|",$t);
		
		$id=hexdec($id);
		$slot=hexdec($slot);
		$automatic=hexdec($automatic);
 		$ammo=hexdec($ammo);
if ($id == 0)
continue;

                $prep = $db->prepare("insert into weapons (baseID,damage,reload,rate,automatic,ammo,dlc) values (?, ?, ?, ?, ?, ?, ?)");

                if ($prep === FALSE) {
			echo "Fail: " . $id;
			continue;
		}
		$r = $prep->execute(array($id, $dmg, $reload, $rate, $automatic, $ammo, $dlc));
if (!$r) {
$arr = $prep->errorInfo();
echo $arr[2];
}

                $prep = $db->prepare("insert into items (baseID,value,health, weight, slot, dlc) values (?, ?, ?, ?, ?, ?)");

                if ($prep === FALSE) {
			echo "Fail: " . $id;
			continue;
		}
		$r = $prep->execute(array($id, $value, $hp, $weight, $slot, $dlc));
if (!$r) {
$arr = $prep->errorInfo();
echo $arr[2];
}

	}
}


function createTable($tb)
{
	global $db;
	$db->exec("CREATE TABLE $tb (baseID integer,damage float,reload float, rate float, automatic integer, ammo integer, dlc integer)");
}

function createItems($tb)
{
	global $db;
	$db->exec("CREATE TABLE $tb (baseID integer,value integer, health integer, weight float, slot integer, dlc integer)");
}

$db = new PDO('sqlite:fallout3.sqlite');
if(!$db)
{
    die("Errore Sqlite");
}

import("data3/f3_main_weapons.txt",0);
import("data3/f3_tp_weapons.txt",1);
import("data3/f3_oa_weapons.txt",2);
import("data3/f3_bsn_weapons.txt",3);
import("data3/f3_pl_weapons.txt",4);
import("data3/f3_mz_weapons.txt",5);

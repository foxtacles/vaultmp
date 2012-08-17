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

$db = new PDO('sqlite:newvegas.sqlite');
if(!$db)
{
    die("Errore Sqlite");
}

import("data/fnv_main_weapons.txt",0);
import("data/fnv_dm_weapons.txt",1);
import("data/fnv_hh_weapons.txt",2);
import("data/fnv_ow_weapons.txt",3);
import("data/fnv_lr_weapons.txt",4);
import("data/fnv_GunRunnersArsenal_weapons.txt",5);

import("data/fnv_ClassicPack_weapons.txt",6);
import("data/fnv_CaravanPack_weapons.txt",7);
import("data/fnv_TribalPack_weapons.txt",8);
import("data/fnv_MercenaryPack_weapons.txt",9);

